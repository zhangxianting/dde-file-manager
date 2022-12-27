/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "usersharepasswordsettingdialog.h"

#include "dfm-base/utils/windowutils.h"

#include <dfm-framework/event/event.h>

#include <QDebug>
#include <QProcess>
#include <QDBusReply>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWindow>
#include <QLabel>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DWIDGET_USE_NAMESPACE

UserSharePasswordSettingDialog::UserSharePasswordSettingDialog(QWidget *parent)
    : DDialog(parent)
{
    setTitle(tr("Enter a password to protect shared folders"));
    setIcon(QIcon::fromTheme("dialog-password-publicshare"));
    initializeUi();
}

void UserSharePasswordSettingDialog::initializeUi()
{
    QStringList buttonTexts;
    buttonTexts.append(tr("Cancel", "button"));
    buttonTexts.append(tr("Confirm", "button"));
    addButton(buttonTexts[0], false);
    addButton(buttonTexts[1], false, DDialog::ButtonRecommend);
    setDefaultButton(1);
    passwordEdit = new DPasswordEdit(this);
    passwordEdit->setFocus();
    addContent(passwordEdit);
    setContentsMargins(0, 0, 0, 0);
    getButton(1)->setEnabled(false);
    QLabel *notice = new QLabel(tr("Set a password on the shared folder for non-anonymous access"), this);
    QPalette pe;
    pe.setColor(QPalette::WindowText, QColor("red"));
    notice->setPalette(pe);
    insertContent(1, notice);

    connect(passwordEdit, &DPasswordEdit::textChanged, this, [this] {
        getButton(1)->setEnabled(!passwordEdit->text().isEmpty());
    });

    if (WindowUtils::isWayLand()) {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
        this->setFixedSize(QSize(390, 210));
    }
}

void UserSharePasswordSettingDialog::onButtonClicked(const int &index)
{
    if (index == 1) {
        // set usershare password
        QString password = passwordEdit->text();
        if (password.isEmpty()) {
            close();
            return;
        }

        QString userName = dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_CurrentUserName").toString();
        dpfSlotChannel->push("dfmplugin_dirshare", "slot_Share_SetSmbPasswd", userName, password);
    }
    close();
}
