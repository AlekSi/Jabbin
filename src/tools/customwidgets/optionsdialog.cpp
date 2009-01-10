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

#include <phonon/objectdescription.h>
#include <phonon/backendcapabilities.h>

#include <QHeaderView>

#ifdef Q_WS_X11
#include <unistd.h>
#endif

static const QString autoStartRegistryKey = "CurrentVersion/Run/joimchat.exe";

OptionsDialog::Private::Private(OptionsDialog * parent)
    : q(parent), controller(NULL),
      iconNotification(":/customwidgets/data/options/notifications.png"),
      iconSound(":/customwidgets/data/options/sounds.png"),
      iconJoim(":/customwidgets/data/options/general.png")
{
    setupUi(parent);

    // Hiding unimplemented options
    tabs->removeTab(tabs->indexOf(tabAudio));
    tabs->removeTab(tabs->indexOf(tabSounds));
    labelLanguage->setVisible(false);
    comboLanguage->setVisible(false);
    checkUseOfficeBackground->setVisible(false);
    comboChatBackground->setVisible(false);
    labelChatBackground->setVisible(false);
    labelStatusAway->setVisible(false);
    editStatusAway->setVisible(false);

    // Init
    tabs->setViewType(CustomWidgets::AdvancedTabBar::ListView);
    tabs->setTabBarThickness(64);

    connect(buttonBox, SIGNAL(clicked(QAbstractButton *)),
                this, SLOT(buttonClicked(QAbstractButton *)));

    // Avatars combo
    comboContactListAvatars->addItem(tr("Auto-sized"), 1);
    comboContactListAvatars->addItem(tr("Big"), 2);
    comboContactListAvatars->addItem(tr("Small"), 3);

    // Chat bg combo
    QMap<QString, QString> m;
    m["carbon.png"]   = tr("Carbon");
    m["orange.png"]   = tr("Orange");
    m["pinky.png"]    = tr("Pinky");
    m["seawater.png"] = tr("Sea water");
    m["silver.png"]   = tr("Silver");
    m["sky.png"]      = tr("Sky");
    m["office.png"]   = tr("Office");

    comboChatBackground->addItem(tr("Office"), "office.png");

    foreach(QString b, YaWindowBackground::funnyBackgrounds()) {
        Q_ASSERT(m.contains(b));
        comboChatBackground->addItem(m[b], b);
    }

    comboChatBackground->addItem(tr("Random"), "random");

    // Alerts and Messages page
    tableNotifications->setColumnWidth(0, 250);
    tableNotifications->setColumnWidth(1, 32);
    tableNotifications->setColumnWidth(2, 32);
    tableNotifications->setColumnWidth(3, 32);
    tableNotifications->verticalHeader()->hide();
    tableNotifications->horizontalHeader()->hide();
    groupNofiticationOptions->setEnabled(false);

    for (int row = 0; row < tableNotifications->rowCount(); row++) {
        for (int col = 1; col < tableNotifications->columnCount(); col++) {
            QTableWidgetItem * item = new QTableWidgetItem();
            switch (col) {
                case 1:
                    item->setIcon(iconNotification);
                    break;
                case 2:
                    item->setIcon(iconSound);
                    break;
                case 3:
                    item->setIcon(iconJoim);
                    break;
            }
            tableNotifications->setItem(row, col, item);
        }
    }




    // audio devices
    foreach (Phonon::AudioOutputDevice dev, Phonon::BackendCapabilities::availableAudioOutputDevices()) {
        comboSpeakerDevice->addItem(dev.description() + " (" + dev.name() + ")");
    }

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
    : StyledWindow(parent), d(new Private(this))
{
    setUseCustomDecorations(true);
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
    // General Settings page
    bool value = getOption("options.general.contact.doubleclick.call", Bool);
    d->radioContactDoubleClickCall->setChecked(value);
    d->radioContactDoubleClickChat->setChecked(!value);

    d->editStatusAway->setValue(getOption("options.general.status.auto.away", Int));
    d->editStatusDND->setValue(getOption("options.general.status.auto.dnd", Int));
    d->editStatusOffline->setValue(getOption("options.general.status.auto.offline", Int));
    d->checkRestoreActiveStatus->setChecked(getOption("options.general.status.auto.restore", Bool));

#if defined(Q_WS_WIN)
    // TODO: needs testing
    QSettings autoStartSettings(QSettings::NativeFormat, QSettings::UserScope, "Microsoft", "Windows");
    d->checkAutostart->setChecked(autoStartSettings.contains(autoStartRegistryKey));
#elif defined(Q_WS_X11)
    QString home = QDir::homePath();
    // gnome (and possibly others)
    bool autorunGnome = QFile::exists(home + "/.config/autostart/joim");
    // kde
    bool autorunKDE = QFile::exists(home + "/.kde/Autostart/joim") ||
                      QFile::exists(home + "/.kde4/Autostart/joim");
    d->checkAutostart->setChecked(autorunKDE && autorunGnome);
    if (autorunGnome != autorunKDE) {
        d->checkAutostart->setCheckState(Qt::PartiallyChecked);
    }
#else
    ui_.startAutomatically->hide();
#endif

    d->comboLanguage->setCurrentText(getOption("options.general.language", String));

    // Appearance page
    d->checkUseOfficeBackground->setChecked(getOption("options.ya.office-background", Bool));
    d->checkShowContactListGroups->setChecked(getOption("options.ya.main-window.contact-list.show-groups", Bool));

    int avatarStyle = getOption("options.ya.main-window.contact-list.avatar-style", Int);
    d->checkEnableContactListAvatars->setChecked(avatarStyle);
    d->comboContactListAvatars->setEnabled(avatarStyle);
    if (avatarStyle != 0) {
        d->comboContactListAvatars->setCurrentIndex(
            d->comboContactListAvatars->findData(avatarStyle));
    }

    d->checkCtrlEnterSendsChatMessages->setChecked(
            ShortcutManager::instance()->shortcuts("chat.send").contains(
                QKeySequence(Qt::CTRL + Qt::Key_Return)));

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

    QFont font;
    font.fromString(option.font[2]);
    d->comboFont->setCurrentFont(font);
    d->editFontSize->setValue(font.pointSize());

    // Audio settings page
    // Nothing at the moment, we don't use Phonon for talk
    // (we can not yet, no input support)

    // Alerts and Messages page
    // value = getOption("options.notifications.joim-popup", Bool);
    // d->radioJoimCallAlert->setChecked(value);
    // d->radioSystemCallAlert->setChecked(!value);

    // d->checkPopupOnContactRequest->setChecked(getOption("options.notifications.friend-request", Bool));

    // d->checkNotificationContactOnline->setChecked(getOption("options.notifications.contact-online", Bool));
    // d->checkNotificationChatRequest->setChecked(getOption("options.notifications.chat-request", Bool));
    // d->checkNotificationReceivingFile->setChecked(getOption("options.notifications.receiving-file", Bool));
    // d->checkNotificationVoicemail->setChecked(getOption("options.notifications.voicemail", Bool));


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
    setOption("options.general.status.auto.restore", d->checkRestoreActiveStatus->isChecked());

    bool autostart = d->checkAutostart->isChecked();
#if defined(Q_WS_WIN)
    // TODO: needs testing
    QSettings autoStartSettings(QSettings::NativeFormat, QSettings::UserScope, "Microsoft", "Windows");
    if (autostart) {
        autoStartSettings.remove(autoStartRegistryKey);
    } else {
        autoStartSettings.setValue(autoStartRegistryKey,
            QCoreApplication::applicationFilePath());
    }
#elif defined(Q_WS_X11)
    QString home = QDir::homePath();
    if (autostart) {
        symlink(QCoreApplication::applicationFilePath(), home + "/.config/autostart/joim");
        symlink(QCoreApplication::applicationFilePath(), home + "/.kde/Autostart/joim");
        symlink(QCoreApplication::applicationFilePath(), home + "/.kde4/Autostart/joim");
    } else {
        unlink(home + "/.config/autostart/joim");
        unlink(home + "/.kde/Autostart/joim");
        unlink(home + "/.kde4/Autostart/joim");
    }
#else
    // nothing
#endif

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

    QFont font = d->comboFont->currentFont();
    font.setPointSize(d->editFontSize->value());
    option.font[2] = d->comboFont->currentFont().toString();

    // Alerts and Messages page
    // setOption("options.notifications.joim-popup", d->radioJoimCallAlert->isChecked());

    // setOption("options.notifications.friend-request", d->checkPopupOnContactRequest->isChecked());
    // setOption("options.notifications.contact-online", d->checkNotificationContactOnline->isChecked());
    // setOption("options.notifications.chat-request", d->checkNotificationChatRequest->isChecked());
    // setOption("options.notifications.receiving-file", d->checkNotificationReceivingFile->isChecked());
    // setOption("options.notifications.voicemail", d->checkNotificationVoicemail->isChecked());


    //
    if (d->controller)
        d->controller->slotApplyOptions(option);
#undef setOption
}

void OptionsDialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    load();

    d->tabs->setCurrentIndex(0);
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
