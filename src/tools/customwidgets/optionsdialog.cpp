/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser/Library General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser/Library General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser/Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "optionsdialog.h"
#include "optionsdialog_p.h"
#include "ui_optionsdialog_base.h"
#include "psioptions.h"
#include "shortcutmanager.h"
#include "psiaccount.h"
#include "psicontactlist.h"
#include "common.h"

OptionsDialog::Private::Private(OptionsDialog * parent)
    : q(parent), controller(NULL)
{
    setupUi(parent);
    tabs->setViewType(AdvancedTabBar::ListView);
    tabs->setTabBarThickness(200);

    connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
                this, SLOT(buttonClicked(QAbstractButton *)));
}

void OptionsDialog::Private::buttonClicked(QAbstractButton * object)
{
    QDialogButtonBox::StandardButton button = buttonBox->standardButton(object);
    if (button == QDialogButtonBox::Ok || button == QDialogButtonBox::Apply) {
        q->save();
    }
    if (button == QDialogButtonBox::Ok || button == QDialogButtonBox::Cancel) {
        q->hide();
    }

}

OptionsDialog::OptionsDialog(QWidget * parent)
    : YaWindow(parent), d(new Private(this))
{
}

OptionsDialog::~OptionsDialog()
{
    delete d;
}

void OptionsDialog::cancel()
{
}

void OptionsDialog::apply()
{
}

void OptionsDialog::ok()
{
}

void OptionsDialog::load()
{
#define getOption(A, B) PsiOptions::instance()->getOption(A).to##B ()
    // General page
    bool value = getOption("options.general.contact.doubleclick.call", Bool);
    d->radioContactDoubleClickCall->setChecked(value);
    d->radioContactDoubleClickChat->setChecked(!value);

    d->editStatusAway->setValue(getOption("options.general.status.auto.away", Int));
    d->editStatusDND->setValue(getOption("options.general.status.auto.dnd", Int));
    d->editStatusOffline->setValue(getOption("options.general.status.auto.offline", Int));

    d->checkAutostart->setChecked(getOption("options.general.autostart", Bool));
    d->comboLanguage->setCurrentText(getOption("options.general.language", String));

    // Appearance page
    d->checkUseOfficeBackground->setChecked(getOption("options.ya.office-background", Bool));
    d->checkShowContactListGroups->setChecked(getOption("options.ya.main-window.contact-list.show-groups", Bool));
    int avatarStyle = getOption("options.ya.main-window.contact-list.avatar-style", Int);
    d->checkEnableContactListAvatars->setChecked(avatarStyle != 0);

    d->checkCtrlEnterSendsChatMessages->setChecked(ShortcutManager::instance()->shortcuts("chat.send").contains(QKeySequence(Qt::CTRL + Qt::Key_Return)));
    d->comboChatBackground->setCurrentIndex(
            d->comboChatBackground->findData(getOption("options.ya.chat-background", String)
            ));
    d->checkEnableEmoticons->setChecked(option.useEmoticons);

    bool logEnabled = false;
    if (d->controller)
    foreach (PsiAccount * account, d->controller->contactList()->accounts()) {
        if (account->userAccount().opt_log) {
            logEnabled = true;
            break;
        }
    }
    d->checkLogMessageHistory->setChecked(logEnabled);

    // TODO: Font settings...



#undef getOption
}

void OptionsDialog::save()
{
#define setOption(A, B) PsiOptions::instance()->setOption(A, B)
    // General page
    setOption("options.general.contact.doubleclick.call", d->radioContactDoubleClickCall->isChecked());

    setOption("options.general.status.auto.away", d->editStatusAway->value());
    setOption("options.general.status.auto.dnd", d->editStatusDND->value());
    setOption("options.general.status.auto.offline", d->editStatusOffline->value());

    setOption("options.general.autostart", d->checkAutostart->isChecked());
    setOption("options.general.language", d->comboLanguage->currentText());

    // Appearance page
    setOption("options.ya.office-background", d->checkUseOfficeBackground->isChecked());
    setOption("options.ya.main-window.contact-list.show-groups", d->checkShowContactListGroups->isChecked());
    setOption("options.ya.main-window.contact-list.avatar-style", d->checkEnableContactListAvatars->isChecked() ? "1" : "0");

    bool soft = ShortcutManager::instance()->shortcuts("chat.send").contains(QKeySequence(Qt::Key_Return));
    if (soft != !d->checkCtrlEnterSendsChatMessages->isChecked()) {
        QVariantList vl;
        if (!d->checkCtrlEnterSendsChatMessages->isChecked()) {
            vl << qVariantFromValue(QKeySequence(Qt::Key_Enter)) << qVariantFromValue(QKeySequence(Qt::Key_Return));
        }
        else {
            vl << qVariantFromValue(QKeySequence(Qt::Key_Enter+Qt::CTRL)) << qVariantFromValue(QKeySequence(Qt::CTRL+Qt::Key_Return));
        }
        setOption("options.shortcuts.chat.send", vl);
    }

    // TODO: Font settings...
    if (d->controller)
        d->controller->slotApplyOptions(option);
#undef setOption
}

void OptionsDialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    load();
}

void OptionsDialog::openPreferences()
{
    d->tabs->setCurrentWidget(d->tabGeneralSettings);
    show();
}

void OptionsDialog::openAccounts()
{
    d->tabs->setCurrentWidget(d->tabAccounts);
    show();
}

void OptionsDialog::setController(PsiCon * controller)
{
    d->controller = controller;
}
