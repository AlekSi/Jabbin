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
#include "generic/notifications.h"
#include "joimnotifications.h"

#include <phonon/objectdescription.h>
#include <phonon/backendcapabilities.h>

#include <QHeaderView>
#include <QFileDialog>

#ifdef Q_WS_X11
#include <unistd.h>
#endif

#define DEFAULT_CALL_SERVER_JID "jabberout.com"
#define DEFAULT_CALL_SERVER_RESOURCE "phone"

static const QString autoStartRegistryKey = "CurrentVersion/Run/joimchat.exe";

OptionsDialog::Private::Private(OptionsDialog * parent)
    : q(parent), controller(NULL),
      iconNotification(":/customwidgets/data/options/notifications.png"),
      iconSound(":/customwidgets/data/options/sounds.png"),
      iconJoim(":/customwidgets/data/options/general.png"),
      tableNotificationsCurrentRow(-1)
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
    tabs->setTabBarThickness(240);

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
    groupNofiticationOptions->setVisible(false);

    for (int row = 0; row < tableNotifications->rowCount(); row++) {
        for (int col = 1; col < tableNotifications->columnCount(); col++) {
            QTableWidgetItem * item = new QTableWidgetItem();
             // switch (col) {
             //     case 1:
             //         item->setIcon(iconNotification);
             //         break;
             //     case 2:
             //         item->setIcon(iconSound);
             //         break;
             //     case 3:
             //         item->setIcon(iconJoim);
             //         break;
             // }
            tableNotifications->setItem(row, col, item);
        }
    }

    tableNotifications->item(0, 0)->setData(Qt::UserRole, N_INCOMING_CALL);
    tableNotifications->item(1, 0)->setData(Qt::UserRole, N_CHAT_REQUEST);
    tableNotifications->item(2, 0)->setData(Qt::UserRole, N_STATUS_REQUEST);
    tableNotifications->item(3, 0)->setData(Qt::UserRole, N_CONTACT_ONLINE);
    tableNotifications->item(4, 0)->setData(Qt::UserRole, N_INCOMING_FILE);
    tableNotifications->item(5, 0)->setData(Qt::UserRole, N_INCOMING_VOICEMAIL);

    connect(tableNotifications, SIGNAL(currentCellChanged(int,int,int,int)),
            this, SLOT(tableNotificationsCurrentCellChanged(int,int,int,int)));
    connect(buttonNotificationSoundBrowse, SIGNAL(clicked()),
            this, SLOT(buttonNotificationSoundBrowseClicked()));
    connect(buttonSampleTooltip, SIGNAL(clicked()),
            this, SLOT(buttonSampleTooltipClicked()));
    connect(checkNotificationTooltip, SIGNAL(clicked()),
            this, SLOT(tableNotificationsUpdate()));
    connect(checkNotificationSound, SIGNAL(clicked()),
            this, SLOT(tableNotificationsUpdate()));
    connect(checkPopupJoim, SIGNAL(clicked()),
            this, SLOT(tableNotificationsUpdate()));
    connect(buttonServiceProvidersAdd, SIGNAL(clicked()),
            this, SLOT(buttonServiceProvidersAddClicked()));
    connect(buttonServiceProvidersRemove, SIGNAL(clicked()),
            this, SLOT(buttonServiceProvidersRemoveClicked()));

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

void OptionsDialog::Private::tableNotificationsCurrentCellChanged(int currentRow,
            int currentColumn, int previousRow, int previousColumn)
{
    if (currentRow == previousRow) {
        return;
    }

    tableNotificationsCurrentRow = currentRow;

    if (tableNotificationsCurrentRow == -1) {
        return;
    }

    qDebug() << tableNotifications->item(currentRow, 0)->data(Qt::UserRole);

    checkNotificationTooltip->setChecked(
            tableNotifications->item(currentRow, 1)->data(Qt::UserRole).toBool());
    checkPopupJoim->setChecked(
            tableNotifications->item(currentRow, 3)->data(Qt::UserRole).toBool());

    QString svalue = tableNotifications->item(currentRow, 2)->data(Qt::UserRole).toString();
    checkNotificationSound->setChecked(!svalue.isEmpty());
    labelNotificationSound->setText(svalue);
    labelNotificationSound->setVisible(!svalue.isEmpty());
    buttonNotificationSoundBrowse->setVisible(!svalue.isEmpty());

    groupNofiticationOptions->setVisible(true);
}

void OptionsDialog::Private::tableNotificationsUpdate()
{
    if (tableNotificationsCurrentRow == -1) {
        return;
    }

    tableNotifications->item(tableNotificationsCurrentRow, 1)
        ->setData(Qt::UserRole, checkNotificationTooltip->isChecked());
    tableNotifications->item(tableNotificationsCurrentRow, 1)->setIcon(
            checkNotificationTooltip->isChecked() ? iconNotification : QIcon());
    tableNotifications->item(tableNotificationsCurrentRow, 3)
        ->setData(Qt::UserRole, checkPopupJoim->isChecked());
    tableNotifications->item(tableNotificationsCurrentRow, 3)->setIcon(
            checkPopupJoim->isChecked() ? iconJoim : QIcon());

    QString svalue = QString();
    if (checkNotificationSound->isChecked()) {
        svalue = labelNotificationSound->text();
    }
    qDebug() << svalue << "###########";
    tableNotifications->item(tableNotificationsCurrentRow, 2)
        ->setData(Qt::UserRole, svalue);
    tableNotifications->item(tableNotificationsCurrentRow, 2)->setIcon(
            (!svalue.isEmpty()) ? iconSound : QIcon());

}

void OptionsDialog::Private::buttonServiceProvidersAddClicked()
{
    listServiceProviders->addItem("");
    QListWidgetItem * item =
            listServiceProviders->item(listServiceProviders->count() - 1);
    item->setFlags( item->flags() | Qt::ItemIsEditable );

    listServiceProviders->editItem(item);
}

void OptionsDialog::Private::buttonServiceProvidersRemoveClicked()
{
    qDebug() << "buttonServiceProvidersRemoveClicked()";
    foreach (QListWidgetItem * item, listServiceProviders->selectedItems()) {
        qDebug() << "deleting " << item->text();
        delete item;
    }
}

void OptionsDialog::Private::buttonNotificationSoundBrowseClicked()
{
    QString file = QFileDialog::getOpenFileName(
            q, tr("Choose a sound file"));
    qDebug() << "Chosen file " << file;
    labelNotificationSound->setText(file);
    tableNotificationsUpdate();
}

void OptionsDialog::Private::buttonSampleTooltipClicked()
{
    QMap < QString, QString > actions;

    CustomWidgets::Notifications * notifications = CustomWidgets::Notifications::instance();
    notifications->showNotification("Simple Notification",
            "This is a simple notification tooltip with a specified timeout",
            iconJoim.pixmap(64, 64), actions, 2);

    actions["chat"] = "Close";
    notifications->showNotification("Notification with action",
            "This is a simple notification tooltip with a longer timeout, and an action button",
            iconJoim.pixmap(64, 64), actions, 4);
    notifications->showNotification("Permanent notification",
            "This is a notification tooltip without a timer, so you need to click 'Close'",
            iconJoim.pixmap(64, 64), actions);
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
    for (int row = 0; row < d->tableNotifications->rowCount(); row++) {
        QString id = d->tableNotifications->item(row, 0)->data(Qt::UserRole).toString();

        bool value;
        QString svalue;

        // tooltip
        value = getOption(id + ".tooltip", Bool);
        d->tableNotifications->item(row, 1)->setData(Qt::UserRole, value);
        d->tableNotifications->item(row, 1)->setIcon(value ? d->iconNotification : QIcon());

        // sound
        svalue = getOption(id + ".sound", String);
        d->tableNotifications->item(row, 2)->setData(Qt::UserRole, svalue);
        d->tableNotifications->item(row, 2)->setIcon(svalue.isEmpty() ? QIcon() : d->iconSound);

        // popup joim
        value = getOption(id + ".popupjoim", Bool);
        d->tableNotifications->item(row, 3)->setData(Qt::UserRole, value);
        d->tableNotifications->item(row, 3)->setIcon(value ? d->iconJoim : QIcon());

    }

    // Accounts page
    //define getOption(A, B) PsiOptions::instance()->getOption(A).to##B ()
    QString svalue = getOption("call.server.jid", String);
    if (svalue.isEmpty()) {
        svalue = DEFAULT_CALL_SERVER_JID;
    }
    d->editPhoneServicesServer->setText(svalue);

    svalue = getOption("call.server.resource", String);
    if (svalue.isEmpty()) {
        svalue = DEFAULT_CALL_SERVER_RESOURCE;
    }
    d->editPhoneServicesName->setText(svalue);

    // Lists
    QVariantList lvalue = getOption("service.providers.list", List);
    d->listServiceProviders->clear();

    int i = 0;
    foreach (QVariant service, lvalue) {
        d->listServiceProviders->addItem(service.toString());
        QListWidgetItem * item = d->listServiceProviders->item(i);
        item->setFlags( item->flags() | Qt::ItemIsEditable );
        i++;
    }

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
    for (int row = 0; row < d->tableNotifications->rowCount(); row++) {
        QString id = d->tableNotifications->item(row, 0)->data(Qt::UserRole).toString();

        bool value;
        QString svalue;

        // tooltip
        value = d->tableNotifications->item(row, 1)->data(Qt::UserRole).toBool();
        setOption(id + ".tooltip", value);

        // sound
        svalue = d->tableNotifications->item(row, 2)->data(Qt::UserRole).toString();
        setOption(id + ".sound", svalue);

        // popup joim
        value = d->tableNotifications->item(row, 3)->data(Qt::UserRole).toBool();
        setOption(id + ".popupjoim", value);

    }

    // Accounts page
    QString svalue = d->editPhoneServicesServer->text();
    if (svalue.isEmpty()) {
        svalue = DEFAULT_CALL_SERVER_JID;
    }
    setOption("call.server.jid", svalue);

    svalue = d->editPhoneServicesName->text();
    if (svalue.isEmpty()) {
        svalue = DEFAULT_CALL_SERVER_RESOURCE;
    }
    setOption("call.server.resource", svalue);

    QVariantList lvalue;
    for (int i = 0; i < d->listServiceProviders->count(); i++) {
        lvalue << d->listServiceProviders->item(i)->text();
    }
    setOption("service.providers.list", QVariant(lvalue));
    qDebug() << PsiOptions::instance()->getOption("service.providers.list");

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
