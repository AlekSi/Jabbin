/*
 * yachatdlg.cpp - custom chat dialog
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

#include <QPainter>
#include <QIcon>
#include <QTextDocument> // for Qt::escape()
#include <QMouseEvent>

#include "yachatdlg.h"
#include "yatrayicon.h"

#include "iconselect.h"
#include "textutil.h"
#include "iconselect.h"
#include "yacommon.h"
#include "yacontactlabel.h"
#include "yaselflabel.h"
#include "psicon.h"
#include "psicontact.h"
#include "psicon.h"
#include "psiaccount.h"
#include "userlist.h"
#include "yachattooltip.h"
#include "iconset.h"
#include "yachatviewmodel.h"
#include "globaleventqueue.h"
#include "xmpp_message.h"
#include "yavisualutil.h"
#include "yaprivacymanager.h"
#include "applicationinfo.h"
#include "tabdlg.h"
#include "yapushbutton.h"
#include "psioptions.h"
#include "psiiconset.h"
#include "contacttooltip.h"

static const QString emoticonsEnabledOptionPath = "options.ya.emoticons-enabled";
static const QString enableTypographyOptionPath = "options.ya.typography.enable";

class YaChatDlgShared : public QObject
{
	Q_OBJECT
public:
	QAction* typographyAction() const { return typographyAction_; }
	QAction* emoticonsAction() const { return emoticonsAction_; }

	static YaChatDlgShared* instance()
	{
		if (!instance_) {
			instance_ = new YaChatDlgShared();
		}
		return instance_;
	}

private slots:
	void typographyActionTriggered(bool enabled)
	{
		PsiOptions::instance()->setOption(enableTypographyOptionPath, enabled);
	}

	void emoticonsActionTriggered(bool enabled)
	{
		option.useEmoticons = enabled;
		PsiOptions::instance()->setOption(emoticonsEnabledOptionPath, enabled);
	}

	void optionChanged(const QString& option)
	{
		if (option == emoticonsEnabledOptionPath) {
			emoticonsAction_->setChecked(PsiOptions::instance()->getOption(emoticonsEnabledOptionPath).toBool());
		}
		else if (option == enableTypographyOptionPath) {
			typographyAction_->setChecked(PsiOptions::instance()->getOption(enableTypographyOptionPath).toBool());
		}
	}

private:
	YaChatDlgShared()
		: QObject(QCoreApplication::instance())
	{
		typographyAction_ = new QAction(tr("Typographica"), this);
		typographyAction_->setCheckable(true);
		connect(typographyAction_, SIGNAL(triggered(bool)), SLOT(typographyActionTriggered(bool)));

		emoticonsAction_ = new QAction(tr("Enable emoticons"), this);
		emoticonsAction_->setCheckable(true);
		connect(emoticonsAction_, SIGNAL(triggered(bool)), SLOT(emoticonsActionTriggered(bool)));

		connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
		optionChanged(emoticonsEnabledOptionPath);
		optionChanged(enableTypographyOptionPath);
	}

	static YaChatDlgShared* instance_;
	QPointer<QAction> typographyAction_;
	QPointer<QAction> emoticonsAction_;
};

YaChatDlgShared* YaChatDlgShared::instance_ = 0;

//----------------------------------------------------------------------------
// YaContactToolTipArea
//----------------------------------------------------------------------------

class YaContactToolTipArea : public YaPushButton
{
	Q_OBJECT
public:
	YaContactToolTipArea(QWidget* parent)
		: YaPushButton(parent)
	{
		// setFixedHeight(30 + 5);
		// setCursor(Qt::PointingHandCursor);
		setText(tr("Info"));
	}

	// reimplemented
	//QSize minimumSizeHint() const
	//{
	//	QSize sh = YaPushButton::minimumSizeHint();
	//	sh.setWidth(75);
	//	return sh;
	//}

protected slots:
	// reimplemented
	void updateButtonStyle()
	{
#ifndef YAPSI_NO_STYLESHEETS
		QString styleSheet = QString(
		"QPushButton {"
		"	font-size: 12px;"
		"	color: black;"
		"	border-image: url(:/images/pushbutton/silver_profile/pushbutton.png) 12px 0px 12px 10px;"
		"	border-width: 0px 7px 0px 15px;"
		"}"
		""
		"QPushButton:disabled {"
		"	color: black;"
		"}"
		""
		"QPushButton:focus {"
		"	border-image: url(:/images/pushbutton/silver_profile/pushbutton_focus.png) 12px 0px 12px 10px;"
		"}"
		""
		"QPushButton:hover {"
		"	border-image: url(:/images/pushbutton/silver_profile/pushbutton_hover.png) 12px 0px 12px 10px;"
		"}"
		""
		"QPushButton:pressed {"
		"	border-image: url(:/images/pushbutton/silver_profile/pushbutton_pressed.png) 12px 0px 12px 10px;"
		"}"
		);
		// setStyleSheet(styleSheet);
#endif
	}

protected:
	// reimplemented
	// virtual QStyleOptionButton getStyleOption() const
	// {
	// 	QStyleOptionButton opt = YaPushButton::getStyleOption();
	// 	opt.palette.setColor(QPalette::ButtonText, Qt::black);
	// 	return opt;
	// }

	// reimplemented
	//virtual int iconPopOut() const
	//{
	//	return 0;
	//}

	// reimplemented
	//virtual void adjustRect(const QStyleOptionButton* btn, QRect* rect) const
	//{
	//	Q_UNUSED(btn);
	//	rect->translate(0, 1);
	//	rect->adjust(4, 0, 0, 0);
	//	YaPushButton::adjustRect(btn, rect);
	//}
};

//----------------------------------------------------------------------------
// YaChatDlg
//----------------------------------------------------------------------------

YaChatDlg::YaChatDlg(const Jid& jid, PsiAccount* acc, TabManager* tabManager)
	: ChatDlg(jid, acc, tabManager)
	, selfProfile_(YaProfile::create(acc))
	, contactProfile_(YaProfile::create(acc, jid))
	, model_(0)
	, showAuthButton_(true)
{
	model_ = new YaChatViewModel();
	connect(this, SIGNAL(invalidateTabInfo()), SLOT(updateComposingMessage()));

	connect(account(), SIGNAL(updatedActivity()), SLOT(updateModelNotices()));
	connect(account(), SIGNAL(enabledChanged()), SLOT(updateModelNotices()));
	QTimer::singleShot(0, this, SLOT(updateModelNotices()));
}

YaChatDlg::~YaChatDlg()
{
	delete model_;
}

void YaChatDlg::initUi()
{
	// setFrameStyle(QFrame::StyledPanel);
	ui_.setupUi(this);

	YaChatContactStatus* contactStatus = new YaChatContactStatus(ui_.contactStatus->parentWidget());
	replaceWidget(ui_.contactStatus, contactStatus);
	ui_.contactStatus = contactStatus;

	// connect(ui_.mle, SIGNAL(textEditCreated(QTextEdit*)), SLOT(chatEditCreated()));
	chatEditCreated();

	//QPushButton *contactToolTipAreaOld = ui_.contactToolTipArea;
	//ui_.contactToolTipArea = new YaContactToolTipArea(contactToolTipAreaOld->parentWidget());
	//static_cast<YaContactToolTipArea*>(ui_.contactToolTipArea)->init();
	//replaceWidget(contactToolTipAreaOld, ui_.contactToolTipArea);
	// ui_.contactToolTipArea->installEventFilter(this);

	connect(ui_.contactToolTipArea, SIGNAL(clicked()), SLOT(showContactProfile()));
	connect(ui_.buttonCall, SIGNAL(clicked()), SLOT(callContact()));

	// TODO: connect this button to start a call
	// ui_.contactToolTipArea->setText(tr("Info"));
	// ui_.buttonCall->setText(tr("Call"));
	#ifndef YAPSI_NO_STYLESHEETS
		QString styleSheet = QString(
		"QPushButton, QToolButton {"
		"	font-size: 12px;"
		"	color: black;"
		"	border-image: url(:/images/pushbutton/silver_profile/pushbutton.png) 12px 0px 12px 10px;"
		"	border-width: 0px 7px 0px 15px;"
		"}"
		""
		"QPushButton:disabled, QToolButton:disabled {"
		"	color: black;"
		"}"
		""
		"QPushButton:focus, QToolButton:focus {"
		"	border-image: url(:/images/pushbutton/silver_profile/pushbutton_focus.png) 12px 0px 12px 10px;"
		"}"
		""
		"QPushButton:hover, QToolButton:hover {"
		"	border-image: url(:/images/pushbutton/silver_profile/pushbutton_hover.png) 12px 0px 12px 10px;"
		"}"
		""
		"QPushButton:pressed, QToolButton:pressed {"
		"	border-image: url(:/images/pushbutton/silver_profile/pushbutton_pressed.png) 12px 0px 12px 10px;"
		"}"
		);
		// ui_.buttonCall->setStyleSheet(styleSheet);
		// ui_.contactToolTipArea->setStyleSheet(styleSheet);
		// ui_.buttonEmoticons->setStyleSheet(styleSheet);
	#endif

	// connect(ui_.sendButton, SIGNAL(clicked()), SLOT(doSend()));
	// connect(ui_.historyButton, SIGNAL(clicked()), SLOT(doHistory()));
	// connect(ui_.addToFriendsButton, SIGNAL(clicked()), SLOT(switchFriends()));
	// connect(ui_.buzzButton, SIGNAL(clicked()), SLOT(sendBuzz()));
	// connect(ui_.profileButton, SIGNAL(clicked()), SLOT(doInfo()));
	// connect(account()->psi()->iconSelectPopup(), SIGNAL(textSelected(QString)), SLOT(addEmoticon(QString)));
	// ui_.smileysButton->setMenu(account()->psi()->iconSelectPopup());
	//
	// ui_.selfUserpic->setMode(YaSelfAvatarLabel::OpenProfile);
	//
	// ui_.fontButton->hide();
	// ui_.backgroundButton->hide();
	//
	// ui_.selfUserpic->setContactList(account()->psi()->contactList());
	// ui_.selfName->setContactList(account()->psi()->contactList());
	ui_.contactUserpic->setProfile(contactProfile_);
	ui_.contactName->setProfile(contactProfile_);
	ui_.chatView->setModel(model_);
	QTimer::singleShot(0, this, SLOT(updateContactName()));

	ui_.contactName->setMinimumSize(100, 30);

	// connect(ui_.separator, SIGNAL(textSelected(QString)), SLOT(addEmoticon(QString)));
	connect(ui_.buttonEmoticons, SIGNAL(textSelected(QString)), SLOT(addEmoticon(QString)));
	// connect(ui_.separator, SIGNAL(addContact()), SLOT(addContact()));
	// connect(ui_.separator, SIGNAL(authContact()), SLOT(authContact()));

	{
		if (PsiIconset::instance()->yaEmoticonSelectorIconset()) {
			// ui_.separator->setIconset(*PsiIconset::instance()->yaEmoticonSelectorIconset());
			ui_.buttonEmoticons->setIconset(* PsiIconset::instance()->yaEmoticonSelectorIconset());
		}
	}

	foreach(QPushButton* button, findChildren<QPushButton*>())
		if (button->objectName().startsWith("dummyButton"))
			button->setVisible(false);
	// ui_.settingsButton->setVisible(false);

	// workaround for Qt bug
	// http://www.trolltech.com/developer/task-tracker/index_html?id=150562&method=entry
	// Since we're not using tables all that much, the problem doesn't occur anymore even on Qt 4.3.0
	// chatView()->setWordWrapMode(QTextOption::WrapAnywhere);

	ui_.sendButton->setAction(actionSend());

	resize(sizeHint());
	doClear();
}

void YaChatDlg::restoreLastMessages(unsigned int lastMessagesCount)
{
	foreach(Ya::SpooledMessage spooledMessage, Ya::lastMessages(selfProfile_->account(), contactProfile_->jid(), lastMessagesCount)) {
		bool found = false;
		foreach(int id, GlobalEventQueue::instance()->ids()) {
			PsiEvent* event = GlobalEventQueue::instance()->peek(id);
			if (event->type() == PsiEvent::Message && !spooledMessage.isMood) {
				MessageEvent* me = static_cast<MessageEvent*>(event);
				if (!me->timeStamp().secsTo(spooledMessage.timeStamp) &&
				    (me->message().body() == spooledMessage.message.body()))
				{
					found = true;
					break;
				}
			}
			else if (event->type() == PsiEvent::Mood && spooledMessage.isMood) {
				MoodEvent* me = static_cast<MoodEvent*>(event);
				if (!me->timeStamp().secsTo(spooledMessage.timeStamp) &&
				    (me->mood() == spooledMessage.mood))
				{
					found = true;
					break;
				}
			}
		}

		if (found)
			continue;

#ifndef YAPSI
		if (isEmoteMessage(spooledMessage.message))
			appendEmoteMessage(Spooled_History, spooledMessage.timeStamp, spooledMessage.originLocal, messageText(spooledMessage.message));
		else
			appendNormalMessage(Spooled_History, spooledMessage.timeStamp, spooledMessage.originLocal, messageText(spooledMessage.message));
#else
		if (spooledMessage.isMood) {
			addMoodChange(Spooled_History, spooledMessage.mood, spooledMessage.timeStamp);
			continue;
		}

		if (isEmoteMessage(spooledMessage.message))
			appendEmoteMessage(Spooled_History, spooledMessage.timeStamp, spooledMessage.originLocal, spooledMessage.message.spamFlag(), messageText(spooledMessage.message));
		else
			appendNormalMessage(Spooled_History, spooledMessage.timeStamp, spooledMessage.originLocal, spooledMessage.message.spamFlag(), messageText(spooledMessage.message));
#endif
	}
}

void YaChatDlg::doHistory()
{
	Ya::showHistory(selfProfile_->account(), contactProfile_->jid());
}

void YaChatDlg::doSend()
{
	if (!couldSendMessages()) {
		return;
	}

	YaPrivacyManager* privacyManager = dynamic_cast<YaPrivacyManager*>(account()->privacyManager());
	if (privacyManager && privacyManager->isContactBlocked(jid())) {
		privacyManager->setContactBlocked(jid(), false);
	}

	ChatDlg::doSend();
}

void YaChatDlg::showEvent(QShowEvent* e)
{
	optionsUpdate();
	ChatDlg::showEvent(e);
}

void YaChatDlg::capsChanged()
{
}

bool YaChatDlg::isEncryptionEnabled() const
{
	return false;
}

void YaChatDlg::contactUpdated(UserListItem* u, int status, const QString& statusString)
{
	Q_UNUSED(u);
	Q_UNUSED(status);
	PsiContact* contact = account()->findContact(jid().bare());

	XMPP::Status::Type statusType = XMPP::Status::Offline;
	if (status != -1)
		statusType = static_cast<XMPP::Status::Type>(status);

	ui_.contactStatus->realWidget()->setStatus(contact ? contact->status().type() : statusType, statusString);
	ui_.contactUserpic->setStatus(statusType);

	if (lastStatus_.type() != statusType) {
		model_->setStatusTypeChangedNotice(statusType);
	}

	lastStatus_ = XMPP::Status(statusType, statusString);

	updateModelNotices();
	ui_.buttonCall->setVisible(contact->isCallable());
	// ui_.addToFriendsButton->setEnabled(!Ya::isInFriends(u));
}

void YaChatDlg::addMoodChange(SpooledType spooled, const QString& mood, const QDateTime& timeStamp)
{
	model_->addMoodChange(static_cast<YaChatViewModel::SpooledType>(spooled), mood, timeStamp);
}

void YaChatDlg::updateModelNotices()
{
	PsiContact* contact = account()->findContact(jid().bare());

	model_->setUserIsBlockedNoticeVisible(contact && contact->isBlocked());
	model_->setUserIsOfflineNoticeVisible(ui_.contactStatus->realWidget()->status() == XMPP::Status::Offline);
	model_->setUserNotInListNoticeVisible(!contact || !contact->inList());
	model_->setNotAuthorizedToSeeUserStatusNoticeVisible(contact && !contact->authorizesToSeeStatus());
	model_->setAccountIsOfflineNoticeVisible(!account()->isAvailable());
	model_->setAccountIsDisabledNoticeVisible(!account()->enabled());

	// ui_.separator->setShowAddButton(contact && contact->isBlocked() && account()->isAvailable());
	// ui_.separator->setShowAuthButton(!ui_.separator->showAddButton() && contact && !contact->authorizesToSeeStatus() && showAuthButton_ && account()->isAvailable());
}

void YaChatDlg::updateAvatar()
{
	// TODO
}

void YaChatDlg::optionsUpdate()
{
	ChatDlg::optionsUpdate();
}

QString YaChatDlg::colorString(bool local, ChatDlg::SpooledType spooled) const
{
	return Ya::colorString(local, spooled);
}

void YaChatDlg::appendSysMsg(const QString &str)
{
	// TODO: add a new type to the model
#ifndef YAPSI
	appendNormalMessage(Spooled_None, QDateTime::currentDateTime(), false, str);
#else
	appendNormalMessage(Spooled_None, QDateTime::currentDateTime(), false, 0, str);
#endif
}

void YaChatDlg::appendEmoteMessage(ChatDlg::SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString txt)
{
	model_->addEmoteMessage(static_cast<YaChatViewModel::SpooledType>(spooled), time, local, spamFlag, txt);
}

void YaChatDlg::appendNormalMessage(ChatDlg::SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString txt)
{
	model_->addMessage(static_cast<YaChatViewModel::SpooledType>(spooled), time, local, spamFlag, txt);
}

void YaChatDlg::appendMessageFields(const Message& m)
{
	QString txt;
	if (!m.subject().isEmpty()) {
		txt += QString("<b>") + tr("Subject:") + "</b> " + QString("%1").arg(Qt::escape(m.subject()));
	}
	if (!m.urlList().isEmpty()) {
		UrlList urls = m.urlList();
		txt += QString("<i>") + tr("-- Attached URL(s) --") + "</i>";
		for (QList<Url>::ConstIterator it = urls.begin(); it != urls.end(); ++it) {
			const Url &u = *it;
			txt += QString("<b>") + tr("URL:") + "</b> " + QString("%1").arg(TextUtil::linkify(Qt::escape(u.url())));
			txt += QString("<b>") + tr("Desc:") + "</b> " + QString("%1").arg(u.desc());
		}
	}

	if (txt.isEmpty()) {
		return;
	}

	ChatDlg::SpooledType spooledType = m.spooled() ?
	        Spooled_OfflineStorage :
	        Spooled_None;
#ifndef YAPSI
	appendNormalMessage(spooledType, m.timeStamp(), false, txt);
#else
	appendNormalMessage(spooledType, m.timeStamp(), false, m.spamFlag(), txt);
#endif
}

ChatViewClass* YaChatDlg::chatView() const
{
	return ui_.chatView;
}

ChatEdit* YaChatDlg::chatEdit() const
{
	return ui_.mle;
}

void YaChatDlg::switchFriends()
{
	// FIXME: Disable addToFriendsButton when current account is offline

	if (!account()->isAvailable())
		return;

	UserListItem* u = account()->find(jid().bare());
	if (Ya::isInFriends(u)) {
		foreach(QString group, u->groups()) {
			if (Ya::isFriendsGroup(group)) {
				account()->actionGroupRemove(jid().bare(), group);
			}
		}
	} else {
		PsiContact* contact = account()->findContact(jid().bare());
		if (contact) {
			contact->setGroups(QStringList() << Ya::defaultFriendsGroup());
		} else {
			QStringList groups;
			groups << Ya::defaultFriendsGroup();
			account()->dj_add(jid().bare(), "", groups, true);
		}
	}
}

void YaChatDlg::sendBuzz()
{
	if (!account()->isAvailable())
		return;
	Message m(jid());
	m.setType("chat");
	m.setBody(Ya::buzzMessage());
	m.setTimeStamp(QDateTime::currentDateTime());
	aSend(m);
}

void YaChatDlg::showContactProfile()
{
	PsiContact* contact = account()->findContact(jid().bare());
	// FIXME: won't work after contact was deleted
	if (contact) {
		QRect rect = ui_.contactToolTipArea->geometry();
		QRect itemRect = QRect(ui_.chatTopFrame->mapToGlobal(rect.topLeft()),
			ui_.chatTopFrame->mapToGlobal(rect.bottomRight()));
		ContactTooltip::instance()->showContact(contact, itemRect);
	//	YaChatToolTip::instance()->showText(itemRect, contact, ui_.contactToolTipArea, 0);

	}
}

bool YaChatDlg::eventFilter(QObject* obj, QEvent* e)
{
	// if (obj == ui_.contactToolTipArea &&
	//     (e->type() == QEvent::ToolTip || e->type() == QEvent::MouseButtonRelease)) {
	// 	showContactProfile();
	// }

	return ChatDlg::eventFilter(obj, e);
}

void YaChatDlg::doClear()
{
	model_->doClear();
	model_->addDummyHeader();
}

void YaChatDlg::nicksChanged()
{
	ui_.chatView->nicksChanged(whoNick(true), whoNick(false));
}

void YaChatDlg::updateComposingMessage()
{
	bool enable = state() == TabbableWidget::StateComposing;
	if (enable != model_->composingEventVisible())
		model_->setComposingEventVisible(enable);
}

/**
 * Makes sure widget don't nastily overlap when trying to resize the dialog
 * to the smallest possible size
 */
QSize YaChatDlg::minimumSizeHint() const
{
	QSize sh = ChatDlg::minimumSizeHint();
	sh.setWidth(qMax(200, sh.width()));
	return sh;
}

void YaChatDlg::aboutToShow()
{
	ChatDlg::aboutToShow();
	if (window()->isVisible())
		ui_.chatSplitter->layout()->activate();
}

void YaChatDlg::receivedPendingMessage()
{
#ifndef YAPSI_ACTIVEX_SERVER
	ChatDlg::receivedPendingMessage();
#endif
}

void YaChatDlg::addPendingMessage()
{
	ChatDlg::receivedPendingMessage();
}

void YaChatDlg::resizeEvent(QResizeEvent* e)
{
	ChatDlg::resizeEvent(e);
	updateContactName();
}

void YaChatDlg::updateContactName()
{
	if (getManagingTabDlg()) {
		ui_.contactName->updateEffectiveWidth(getManagingTabDlg()->windowExtra());
	}
}

void YaChatDlg::chatEditCreated()
{
	ChatDlg::chatEditCreated();

	// ui_.separator->setChatWidgets(chatEdit(), chatView());
	chatEdit()->setTypographyAction(YaChatDlgShared::instance()->typographyAction());
	chatEdit()->setEmoticonsAction(YaChatDlgShared::instance()->emoticonsAction());
}

void YaChatDlg::setLooks()
{
	ChatDlg::setLooks();

	// ui_.separator->updateChatEditHeight();
}

void YaChatDlg::callContact()
{
	PsiContact* contact = account()->findContact(jid().bare());
	if (contact && contact->isCallable()) {
		contact->openCall();
	}
}

void YaChatDlg::addContact()
{
	// TODO: need some sort of central dispatcher for this kind of stuff
	emit YaRosterToolTip::instance()->addContact(jid());
}

void YaChatDlg::authContact()
{
	PsiContact* contact = account()->findContact(jid().bare());
	if (contact) {
		contact->rerequestAuthorizationFrom();
	}

	showAuthButton_ = false;
	updateModelNotices();
}

void YaChatDlg::closed()
{
	showAuthButton_ = true;
	updateModelNotices();

	ChatDlg::closed();
}

void YaChatDlg::activated()
{
	ChatDlg::activated();
	updateContactName();
}

#include "yachatdlg.moc"
