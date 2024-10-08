// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "recenteventreceiver.h"
#include "utils/recentmanager.h"

#include <dfm-base/dfm_event_defines.h>

#include <dfm-framework/dpf.h>

#include <QDir>
#include <QTimer>

Q_DECLARE_METATYPE(QDir::Filters);
Q_DECLARE_METATYPE(QString *)
Q_DECLARE_METATYPE(bool *)

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;

RecentEventReceiver *RecentEventReceiver::instance()
{
    static RecentEventReceiver ins;
    return &ins;
}

void RecentEventReceiver::initConnect()
{
    dpfHookSequence->follow("dfmplugin_propertydialog", "hook_PropertyDialog_Disable", RecentEventReceiver::instance(), &RecentEventReceiver::handlePropertydialogDisable);
    dpfSignalDispatcher->subscribe(GlobalEventType::kChangeCurrentUrl, RecentEventReceiver::instance(), &RecentEventReceiver::handleWindowUrlChanged);
}

void RecentEventReceiver::handleWindowUrlChanged(quint64 winId, const QUrl &url)
{
    if (url.scheme() == RecentHelper::scheme()) {
        QTimer::singleShot(0, this, [=] {
            QDir::Filters f = QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden;
            dpfSlotChannel->push("dfmplugin_workspace", "slot_View_SetFilter", winId, f);
        });
    }
}

void RecentEventReceiver::handleUpdateRecent(const QList<QUrl> &urls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)
    if (ok && !urls.isEmpty()) {
        RecentManager::instance()->updateRecent();
    }
}

void RecentEventReceiver::handleFileRenameResult(quint64 winId, const QMap<QUrl, QUrl> &renamedUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(winId)
    Q_UNUSED(errMsg)

    if (!ok || renamedUrls.isEmpty())
        return;
    RecentManager::instance()->updateRecent();
}

void RecentEventReceiver::handleFileCutResult(const QList<QUrl> &srcUrls, const QList<QUrl> &destUrls, bool ok, const QString &errMsg)
{
    Q_UNUSED(errMsg)

    if (!ok || destUrls.isEmpty())
        return;
    RecentManager::instance()->updateRecent();
}

bool RecentEventReceiver::handlePropertydialogDisable(const QUrl &url)
{
    if (url != RecentHelper::rootUrl())
        return false;

    return true;
}

RecentEventReceiver::RecentEventReceiver(QObject *parent)
    : QObject(parent)
{
}
