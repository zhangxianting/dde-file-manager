/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liqianga@uniontech.com>
 *
 * Maintainer: liuzhangjian<liqianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "searchmenuscene.h"
#include "searchmenuscene_p.h"
#include "utils/searchhelper.h"

#include "plugins/common/core/dfmplugin-menu/menuscene/action_defines.h"
#include "plugins/common/core/dfmplugin-menu/menu_eventinterface_helper.h"

#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/dfm_menu_defines.h"

#include <DDesktopServices>

#include <QProcess>
#include <QMenu>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_search;
DFMBASE_USE_NAMESPACE

static constexpr char kWorkspaceMenuSceneName[] = "WorkspaceMenu";
static constexpr char kSortAndDisplayMenuSceneName[] = "SortAndDisplayMenu";
static constexpr char kExtendMenuSceneName[] = "ExtendMenu";

AbstractMenuScene *SearchMenuCreator::create()
{
    return new SearchMenuScene();
}

SearchMenuScenePrivate::SearchMenuScenePrivate(SearchMenuScene *qq)
    : AbstractMenuScenePrivate(qq),
      q(qq)
{
}

void SearchMenuScenePrivate::updateMenu(QMenu *menu)
{
    auto actions = menu->actions();
    if (isEmptyArea) {
        QAction *selAllAct = nullptr;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            const auto &p = act->property(ActionPropertyKey::kActionID);
            if (p == dfmplugin_menu::ActionID::kSelectAll) {
                selAllAct = act;
                break;
            }
        }

        if (selAllAct) {
            actions.removeOne(selAllAct);
            actions.append(selAllAct);
            menu->addActions(actions);
            menu->insertSeparator(selAllAct);
        }
    } else {
        QAction *openLocalAct = nullptr;
        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            const auto &p = act->property(ActionPropertyKey::kActionID);
            if (p == SearchActionId::kOpenFileLocation) {
                openLocalAct = act;
                break;
            }
        }

        // insert 'OpenFileLocation' action
        if (openLocalAct) {
            actions.removeOne(openLocalAct);
            actions.insert(1, openLocalAct);
            menu->addActions(actions);
        }
    }
}

bool SearchMenuScenePrivate::openFileLocation(const QString &path)
{
    // why? because 'DDesktopServices::showFileItem(realUrl(event->url()))' will call session bus 'org.freedesktop.FileManager1'
    // but cannot find session bus when user is root!
    if (SysInfoUtils::isRootUser()) {
        QStringList urls { path };
        return QProcess::startDetached("dde-file-manager", QStringList() << "--show-item" << urls << "--raw");
    }

    return DDesktopServices::showFileItem(path);
}

void SearchMenuScenePrivate::disableSubScene(AbstractMenuScene *scene, const QString &sceneName)
{
    for (const auto s : scene->subscene()) {
        if (sceneName == s->name()) {
            scene->removeSubscene(s);
            delete s;
            return;
        } else {
            disableSubScene(s, sceneName);
        }
    }
}

SearchMenuScene::SearchMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new SearchMenuScenePrivate(this))
{
    d->predicateName[SearchActionId::kOpenFileLocation] = tr("Open file location");
    d->predicateName[dfmplugin_menu::ActionID::kSelectAll] = tr("Select all");
}

SearchMenuScene::~SearchMenuScene()
{
}

QString SearchMenuScene::name() const
{
    return SearchMenuCreator::name();
}

bool SearchMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    if (!d->currentDir.isValid())
        return false;

    QVariantHash tmpParams = params;
    QList<AbstractMenuScene *> currentScene;
    if (d->isEmptyArea) {
        if (auto sortAndDisplayScene = dfmplugin_menu_util::menuSceneCreateScene(kSortAndDisplayMenuSceneName))
            currentScene.append(sortAndDisplayScene);
    } else {
        const auto &targetUrl = SearchHelper::searchTargetUrl(d->currentDir);
        if (targetUrl.scheme() == Global::Scheme::kTrash || targetUrl.scheme() == Global::Scheme::kRecent) {
            auto parentSceneName = dpfSlotChannel->push("dfmplugin_workspace", "slot_FindMenuScene", targetUrl.scheme()).toString();
            if (auto scene = dfmplugin_menu_util::menuSceneCreateScene(parentSceneName))
                currentScene.append(scene);

            tmpParams[MenuParamKey::kCurrentDir] = targetUrl;
        } else {
            if (auto workspaceScene = dfmplugin_menu_util::menuSceneCreateScene(kWorkspaceMenuSceneName))
                currentScene.append(workspaceScene);
        }
    }

    // the scene added by binding must be initializeed after 'defalut scene'.
    currentScene.append(subScene);
    setSubscene(currentScene);

    // 初始化所有子场景
    bool ret = AbstractMenuScene::initialize(params);
    d->disableSubScene(this, kExtendMenuSceneName);
    return ret;
}

AbstractMenuScene *SearchMenuScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<SearchMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool SearchMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    // 创建子场景菜单
    AbstractMenuScene::create(parent);

    if (d->isEmptyArea) {
        QAction *tempAction = parent->addAction(d->predicateName.value(dfmplugin_menu::ActionID::kSelectAll));
        d->predicateAction[dfmplugin_menu::ActionID::kSelectAll] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(dfmplugin_menu::ActionID::kSelectAll));
    } else {
        auto actionList = parent->actions();
        auto iter = std::find_if(actionList.begin(), actionList.end(), [](const QAction *action) {
            const auto &p = action->property(ActionPropertyKey::kActionID);
            return (p == SearchActionId::kOpenFileLocation);
        });

        if (iter == actionList.end()) {
            QAction *tempAction = parent->addAction(d->predicateName.value(SearchActionId::kOpenFileLocation));
            d->predicateAction[SearchActionId::kOpenFileLocation] = tempAction;
            tempAction->setProperty(ActionPropertyKey::kActionID, QString(SearchActionId::kOpenFileLocation));
        }
    }

    return true;
}

void SearchMenuScene::updateState(QMenu *parent)
{
    AbstractMenuScene::updateState(parent);
    d->updateMenu(parent);
}

bool SearchMenuScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.contains(actionId)) {
        // open file location
        if (actionId == SearchActionId::kOpenFileLocation) {
            for (const auto &file : d->selectFiles) {
                auto info = InfoFactory::create<AbstractFileInfo>(file);
                d->openFileLocation(info->pathInfo(PathInfo::kAbsoluteFilePath));
            }

            return true;
        }

        // select all
        if (actionId == dfmplugin_menu::ActionID::kSelectAll) {
            dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SelectAll", d->windowId);
            return true;
        }
    }

    return AbstractMenuScene::triggered(action);
}
