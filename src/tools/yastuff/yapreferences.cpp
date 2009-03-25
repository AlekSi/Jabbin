/*
 * yapreferences.cpp - preferences window
 * Copyright (C) 2008  Yandex LLC (Michail Pishchagin)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "yapreferences.h"

#include <QSettings>
#include <QCoreApplication>
#include <QFontDialog>
#include <QPainter>
#include <QKeyEvent>
#include <QProgressDialog>

#include "psicon.h"
#include "psiaccount.h"
#include "common.h"
#include "psioptions.h"
#include "shortcutmanager.h"
#include "psicontactlist.h"
#include "eventdb.h"
#include "psicontact.h"
#include "psitooltip.h"
#include "yavisualutil.h"
#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif
#include "yaremoveconfirmationmessagebox.h"
#include "yamanageaccounts.h"
#include "yawindowbackground.h"
#include "chatdlg.h"

static const QString autoStartRegistryKey = "CurrentVersion/Run/jabbinchat.exe";

YaPreferences::YaPreferences()
	: YaWindow(0)
	, controller_(0)
	, accountsPage_(0)
{
	ui_.setupUi(this);
	ui_.preferencesPage->installEventFilter(this);

#if defined(Q_WS_WIN) && !defined(YAPSI_ACTIVEX_SERVER)
	ui_.startAutomatically->show();
#else
	ui_.startAutomatically->hide();
#endif

#ifdef Q_WS_MAC
	ui_.ctrlEnterSendsChatMessages->setText(ui_.ctrlEnterSendsChatMessages->text().replace("Ctrl+Enter", QString::fromUtf8("⌘↩")));
#endif

	ui_.clearMessageHistory->setAlignment(Qt::AlignLeft);
	ui_.clearMessageHistory->setEncloseInBrackets(true);

	connect(ui_.okButton, SIGNAL(clicked()), SLOT(accept()));
	connect(ui_.clearMessageHistory, SIGNAL(clicked()), SLOT(clearMessageHistory()));
	// ui_.clearMessageHistory->setButtonStyle(YaPushButton::ButtonStyle_Destructive);

	if (PsiOptions::instance()->getOption("options.ui.account.single").toBool()) {
		ui_.manageAccountsButton->hide();
	}

	background_ = YaWindowBackground(YaWindowBackground::Roster);

	accountsPage_ = new YaManageAccounts(this);
	ui_.stackedWidget->addWidget(accountsPage_);

	foreach(YaPreferencesTabButton* btn, findChildren<YaPreferencesTabButton*>()) {
		btn->setCheckable(true);
		btn->setButtonStyle(YaPushButton::ButtonStyle_Normal);
		btn->updateGeometry();
	}
	openPreferences();
	connect(ui_.preferencesButton, SIGNAL(clicked()), SLOT(openPreferences()));
	connect(ui_.manageAccountsButton, SIGNAL(clicked()), SLOT(openAccounts()));

	{
		QMap<QString, QString> m;
		m["carbon.png"] = tr("Carbon");
		m["orange.png"] = tr("Orange");
		m["pinky.png"] = tr("Pinky");
		m["seawater.png"] = tr("Sea water");
		m["silver.png"] = tr("Silver");
		m["sky.png"] = tr("Sky");
		m["office.png"] = tr("Office");

		ui_.chatBackgroundComboBox->addItem(tr("Office"), "office.png");

		foreach(QString b, YaWindowBackground::funnyBackgrounds()) {
			Q_ASSERT(m.contains(b));
			ui_.chatBackgroundComboBox->addItem(m[b], b);
		}

		ui_.chatBackgroundComboBox->addItem(tr("Random"), "random");
	}

	{
		ui_.contactListAvatarsComboBox->addItem(tr("Big"), 2);
		ui_.contactListAvatarsComboBox->addItem(tr("Small"), 3);
		// ui_.contactListAvatarsComboBox->addItem(tr("Disable"), 0);
		ui_.contactListAvatarsComboBox->addItem(tr("Auto-sized"), 1);
	}

	foreach(QFrame* frame, ui_.preferencesFrame->findChildren<QFrame*>()) {
		if (!frame->objectName().startsWith("expandingFillerFrame"))
			continue;
		QGridLayout* grid = dynamic_cast<QGridLayout*>(ui_.preferencesFrame->layout());
		Q_ASSERT(grid);
		if (!grid)
			break;

		int row, column, rowSpan, columnSpan;
		grid->getItemPosition(grid->indexOf(frame), &row, &column, &rowSpan, &columnSpan);
		if (rowSpan > 1) {
			Q_ASSERT(columnSpan == 1);
			Q_ASSERT(column > 0);
			grid->setColumnStretch(column, 100);
		}
		else {
			Q_ASSERT(columnSpan > 1);
			Q_ASSERT(rowSpan == 1);
			Q_ASSERT(row > 0);
			grid->setRowStretch(row, 100);
		}
	}

	YaPushButton::initAllButtons(this);
#ifdef Q_WS_MAC
	resize(620, 346);
#else
	resize(601, 346);
#endif
	setProperty("show-offscreen", true);
}

YaPreferences::~YaPreferences()
{
	if (controller_) {
		controller_->dialogUnregister(this);
	}
}

void YaPreferences::openPreferences()
{
	setCurrentPage(Page_Preferences);
}

void YaPreferences::openAccounts()
{
	setCurrentPage(Page_Accounts);
}

void YaPreferences::setCurrentPage(YaPreferences::Page page)
{
	bool updatesEnabled = this->updatesEnabled();
	setUpdatesEnabled(false);
	bool pp = page == Page_Preferences;
	ui_.preferencesButton->setChecked(pp);
	ui_.preferencesButton->setEnabled(!pp);
	ui_.manageAccountsButton->setChecked(!pp);
	ui_.manageAccountsButton->setEnabled(pp);

	ui_.stackedWidget->setCurrentWidget(pp ? ui_.preferencesPage : accountsPage_);
	if (!pp) {
		accountsPage_->selectFirstAccount();
	}
	setUpdatesEnabled(updatesEnabled);
}

// TODO: also make sure option saving is snappy
struct Connection {
	Connection(QObject* obj, const char* signal, const char* slot)
		: obj_(obj)
		, signal_(signal)
		, slot_(slot)
	{}

	QObject* obj_;
	const char* signal_;
	const char* slot_;
};

void YaPreferences::setChangedConnectionsEnabled(bool changedConnectionsEnabled)
{
	QList<Connection> connections;
	connections << Connection(ui_.fontCombo, SIGNAL(currentFontChanged(const QFont &)), SLOT(changeChatFontFamily(const QFont &)));
	connections << Connection(ui_.fontSizeCombo, SIGNAL(activated(const QString &)), SLOT(changeChatFontSize(const QString &)));
	connections << Connection(ui_.chatBackgroundComboBox, SIGNAL(currentIndexChanged(int)), SLOT(smthSet()));
	connections << Connection(ui_.contactListAvatarsComboBox, SIGNAL(currentIndexChanged(int)), SLOT(smthSet()));
	foreach(QCheckBox* checkBox, findChildren<QCheckBox*>()) {
		connections << Connection(checkBox, SIGNAL(stateChanged(int)), SLOT(smthSet()));
	}

	foreach(Connection c, connections) {
		if (changedConnectionsEnabled)
			connect(c.obj_, c.signal_, this, c.slot_);
		else
			disconnect(c.obj_, c.signal_, this, c.slot_);
	}
}

void YaPreferences::setController(PsiCon* controller)
{
	controller_ = controller;
	Q_ASSERT(controller_);
	controller_->dialogRegister(this);
	accountsPage_->setController(controller_);

#ifdef YAPSI_ACTIVEX_SERVER
	connect(ui_.yaOnlinePreferences, SIGNAL(clicked()), controller_->yaOnline(), SLOT(showPreferences()), Qt::QueuedConnection);
#else
	ui_.yaOnlinePreferences->hide();
#endif
}

void YaPreferences::activate()
{
	restore();
}

void YaPreferences::restore()
{
	setChangedConnectionsEnabled(false);
	ui_.playSounds->setChecked(useSound);

#if defined(Q_WS_WIN) && !defined(YAPSI_ACTIVEX_SERVER)
	// TODO: needs testing
	QSettings autoStartSettings(QSettings::NativeFormat, QSettings::UserScope, "Microsoft", "Windows");
	ui_.startAutomatically->setChecked(autoStartSettings.contains(autoStartRegistryKey));
#else
	ui_.startAutomatically->hide();
#endif

	ui_.useOfficeBackground->setChecked(PsiOptions::instance()->getOption("options.ya.office-background").toBool());
	// ui_.chatBackgroundComboBox->setEnabled(!ui_.useOfficeBackground->isChecked());

	ui_.ctrlEnterSendsChatMessages->setChecked(ShortcutManager::instance()->shortcuts("chat.send").contains(QKeySequence(Qt::CTRL + Qt::Key_Return)));
	ui_.enableEmoticons->setChecked(option.useEmoticons);
	ui_.showContactListGroups->setChecked(PsiOptions::instance()->getOption("options.ya.main-window.contact-list.show-groups").toBool());
	ui_.showMoodChangePopups->setChecked(PsiOptions::instance()->getOption("options.ya.popups.moods.enable").toBool());
	ui_.showMessageNotifications->setChecked(PsiOptions::instance()->getOption("options.ya.popups.message.enable").toBool());
	ui_.showMessageText->setChecked(PsiOptions::instance()->getOption("options.ya.popups.message.show-text").toBool());
	ui_.showMessageText->setEnabled(ui_.showMessageNotifications->isChecked());

	bool logEnabled = false;
	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		if (account->userAccount().opt_log) {
			logEnabled = true;
			break;
		}
	}
	ui_.logMessageHistory->setChecked(logEnabled);

	chatFont_.fromString(option.font[2]);
	ui_.fontCombo->setCurrentFont(chatFont_);
	int fontIndex = ui_.fontSizeCombo->findText(QString::number(chatFont_.pointSize()));
	if (fontIndex == -1) {
		ui_.fontSizeCombo->addItem(QString::number(chatFont_.pointSize()));
		fontIndex = ui_.fontSizeCombo->findText(QString::number(chatFont_.pointSize()));
		Q_ASSERT(fontIndex != -1);
	}
	ui_.fontSizeCombo->setCurrentIndex(fontIndex);

	ui_.chatBackgroundComboBox->setCurrentIndex(
	    ui_.chatBackgroundComboBox->findData(
	        PsiOptions::instance()->getOption("options.ya.chat-background").toString()
	    ));

	int avatarStyle = PsiOptions::instance()->getOption("options.ya.main-window.contact-list.avatar-style").toInt();
	ui_.enableContactListAvatars->setChecked(avatarStyle != 0);
	ui_.contactListAvatarsComboBox->setEnabled(ui_.enableContactListAvatars->isChecked());
	if (avatarStyle != 0) {
		ui_.contactListAvatarsComboBox->setCurrentIndex(
		    ui_.contactListAvatarsComboBox->findData(
		        avatarStyle
		    ));
	}

	setChangedConnectionsEnabled(true);
}

void YaPreferences::save()
{
	useSound = ui_.playSounds->isChecked();

#if defined(Q_WS_WIN) && !defined(YAPSI_ACTIVEX_SERVER)
	QSettings autoStartSettings(QSettings::NativeFormat, QSettings::UserScope, "Microsoft", "Windows");
	if (!ui_.startAutomatically->isChecked()) {
		autoStartSettings.remove(autoStartRegistryKey);
	}
	else {
		autoStartSettings.setValue(autoStartRegistryKey,
		                           QCoreApplication::applicationFilePath());
	}
#endif

	PsiOptions::instance()->setOption("options.ya.chat-background",
	                                  ui_.chatBackgroundComboBox->itemData(
	                                      ui_.chatBackgroundComboBox->currentIndex()
	                                  ).toString());
	PsiOptions::instance()->setOption("options.ya.office-background", ui_.useOfficeBackground->isChecked());

	// Soft return.
	// Only update this if the value actually changed, or else custom presets
	// might go lost.
	bool soft = ShortcutManager::instance()->shortcuts("chat.send").contains(QKeySequence(Qt::Key_Return));
	if (soft != !ui_.ctrlEnterSendsChatMessages->isChecked()) {
		QVariantList vl;
		if (!ui_.ctrlEnterSendsChatMessages->isChecked()) {
			vl << qVariantFromValue(QKeySequence(Qt::Key_Enter)) << qVariantFromValue(QKeySequence(Qt::Key_Return));
		}
		else  {
			vl << qVariantFromValue(QKeySequence(Qt::Key_Enter+Qt::CTRL)) << qVariantFromValue(QKeySequence(Qt::CTRL+Qt::Key_Return));
		}
		PsiOptions::instance()->setOption("options.shortcuts.chat.send",vl);
	}

	option.useEmoticons = ui_.enableEmoticons->isChecked();
	PsiOptions::instance()->setOption("options.ya.emoticons-enabled", option.useEmoticons);

	PsiOptions::instance()->setOption("options.ya.main-window.contact-list.show-groups", ui_.showContactListGroups->isChecked());

	PsiOptions::instance()->setOption("options.ya.popups.moods.enable", ui_.showMoodChangePopups->isChecked());

	PsiOptions::instance()->setOption("options.ya.popups.message.enable", ui_.showMessageNotifications->isChecked());

	PsiOptions::instance()->setOption("options.ya.popups.message.show-text", ui_.showMessageText->isChecked());
	ui_.showMessageText->setEnabled(ui_.showMessageNotifications->isChecked());

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		UserAccount ua = account->userAccount();
		ua.opt_log = ui_.logMessageHistory->isChecked();
		account->setUserAccount(ua);
	}

	option.font[2] = chatFont_.toString();

	int avatarStyle = 0;
	ui_.contactListAvatarsComboBox->setEnabled(ui_.enableContactListAvatars->isChecked());
	if (ui_.enableContactListAvatars->isChecked()) {
		avatarStyle = ui_.contactListAvatarsComboBox->itemData(
		                  ui_.contactListAvatarsComboBox->currentIndex()
		              ).toInt();
	}
	PsiOptions::instance()->setOption("options.ya.main-window.contact-list.avatar-style",
	                                  avatarStyle);

	controller_->slotApplyOptions(option);
}

void YaPreferences::clearMessageHistory()
{
	YaRemoveConfirmationMessageBox msgBox(
	    tr("Removing message logs"),
	    tr("Do you really want to remove <b>all</b> message logs?"),
	    this);
	msgBox.setDestructiveActionName(tr("Remove All"));
	if (msgBox.confirmRemoval()) {
		confirmationDelete();
	}
}

void YaPreferences::changeChatFontFamily(const QFont & font)
{
	chatFont_.setFamily(font.family());
	save();
}

void YaPreferences::changeChatFontSize(const QString & value)
{
	bool ok;
	int size = value.toInt(&ok, 10);
	if (!ok) {
		ui_.fontSizeCombo->setEditText(QString::number(chatFont_.pointSize()));
	} else {
		chatFont_.setPointSize(size);
		save();
	}
}

void YaPreferences::accept()
{
	close();
}

void YaPreferences::smthSet()
{
	save();

	if (sender() == ui_.useOfficeBackground) {
		ui_.preferencesFrame->repaint();
		ui_.preferencesPage->repaint();
		// ui_.chatBackgroundComboBox->setEnabled(!ui_.useOfficeBackground->isChecked());
	}
}

bool YaPreferences::eventFilter(QObject* obj, QEvent* e)
{
	if (e->type() == QEvent::Paint) {
		if (obj == ui_.preferencesFrame) {
			QWidget* w = static_cast<QWidget*>(obj);
			QPainter p(w);
			p.fillRect(w->rect(), Qt::white);
			return true;
		}
		else if (obj == ui_.preferencesPage) {
			QPainter p(ui_.preferencesPage);
			QRect g = ui_.stackedWidget->geometry();
			QRect r = rect().adjusted(-g.left(), -g.top(), 0, 0);
			background_.paint(&p, r, isActiveWindow());
			return true;
		}
		else if (obj == this) {
			QPainter p(this);
			background_.paint(&p, rect(), isActiveWindow());
			return true;
		}
	}

	return YaWindow::eventFilter(obj, e);
}

void YaPreferences::confirmationDelete()
{
	int numLogs = 0;
	// int processedLogs = 0;
	foreach(PsiAccount* account, controller_->contactList()->accounts())
	numLogs += account->contactList().count();

	// QProgressDialog progress(tr("Removing logs..."), tr("Abort"), 0, numLogs, this);
	// progress.setWindowModality(Qt::WindowModal);
	// progress.show();

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		EDBHandle dbHandle(account->edb());
		foreach(PsiContact* contact, account->contactList()) {
			// progress.setValue(processedLogs++);
			qApp->processEvents();

			// if (progress.wasCanceled())
			// 	break;

			dbHandle.erase(contact->jid());
		}

		while (dbHandle.busy())
			QCoreApplication::instance()->processEvents();

		foreach(ChatDlg* chat, account->findAllDialogs<ChatDlg*>()) {
			chat->doClear();
		}

		// if (progress.wasCanceled())
		// 	break;
	}

#ifdef YAPSI_ACTIVEX_SERVER
	controller_->yaOnline()->clearedMessageHistory();
#endif
}

void YaPreferences::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Escape) {
		close();
		return;
	}
#ifdef Q_WS_MAC
	else if (event->key() == Qt::Key_W && event->modifiers() & Qt::ControlModifier) {
		close();
		return;
	}
#endif
	QWidget::keyPressEvent(event);
}
