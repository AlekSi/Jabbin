/*
 * yaonline.cpp - communication with running instance of Online
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

#include "yaonline.h"

#include <QDomDocument>
#include <QDomElement>
#include <QtCrypto>
#include <QTimer>
#include <QFileInfo>
#include <QDirIterator>
#include <QDir>
#include <QFile>

#include <windows.h>
#include <tlhelp32.h>

#include "dummystream.h"
#include "psiaccount.h"
#include "psicon.h"
#include "psicontactlist.h"
#include "psicontactlist.h"
#include "psievent.h"
#include "xmpp_yalastmail.h"
#include "yaipc.h"
#include "yapopupnotification.h"
#include "yapsiserver.h"
#include "ycuapiwrapper.h"
#include "profiles.h"
#include "proxy.h"
#include "psioptions.h"
#include "common.h"
#include "globaleventqueue.h"
#include "avatars.h"
#include "psicontact.h"
#include "psicon.h"
#include "tabmanager.h"
#include "tabdlg.h"
#include "tabbablewidget.h"
#include "yachatdlg.h"
#include "psilogger.h"
#include "textutil.h"
#include "yavisualutil.h"
#include "yacommon.h"

YaOnline* YaOnlineHelper::instance()
{
	Q_ASSERT(YaOnline::instance_);
	return YaOnline::instance_;
}

YaOnline* YaOnline::instance_ = 0;

YaOnline::YaOnline(YaPsiServer* parent)
	: QObject(parent)
	, server_(parent)
	, controller_(0)
	, ycuApi_(0)
	, lastStatus_(XMPP::Status::Online)
{
	// Q_ASSERT(!instance_);
	instance_ = this;

	LOG_TRACE;
	ycuApi_ = new YcuApiWrapper(this);
	LOG_TRACE;
	connect(parent, SIGNAL(onlineObjectChanged()), SLOT(onlineObjectChanged()));
	connect(parent, SIGNAL(doShowRoster(bool)), SIGNAL(doShowRoster(bool)));
	connect(parent, SIGNAL(disconnectRereadSettingsAndReconnect()), SLOT(disconnectRereadSettingsAndReconnect()));
	connect(parent, SIGNAL(doSetOfflineMode()), SLOT(doSetOfflineMode()));
	connect(parent, SIGNAL(doSetDND(bool)), SLOT(doSetDND(bool)));
	connect(parent, SIGNAL(doSoundsChanged()), SLOT(doSoundsChanged()));
	connect(parent, SIGNAL(doPingServer()), SLOT(doPingServer()));
	connect(parent, SIGNAL(doToasterClicked(const QString&)), SLOT(doToasterClicked(const QString&)));
	connect(parent, SIGNAL(doToasterScreenLocked(const QString&)), SLOT(doToasterScreenLocked(const QString&)));
	connect(parent, SIGNAL(doToasterIgnored(const QString&)), SLOT(doToasterIgnored(const QString&)));
	connect(parent, SIGNAL(doToasterSkipped(const QString&)), SLOT(doToasterSkipped(const QString&)));
	connect(parent, SIGNAL(doShowIgnoredToasters()), SLOT(doShowIgnoredToasters()));
	connect(parent, SIGNAL(doScreenUnlocked()), SLOT(doScreenUnlocked()));
	connect(parent, SIGNAL(doPlaySound(const QString&)), SLOT(doPlaySound(const QString&)));
	connect(parent, SIGNAL(clearMoods()), SIGNAL(clearMoods()));
	connect(parent, SIGNAL(doAuthAccept(const QString&)), SLOT(doAuthAccept(const QString&)));
	connect(parent, SIGNAL(doAuthDecline(const QString&)), SLOT(doAuthDecline(const QString&)));
	connect(parent, SIGNAL(doOnlineConnected()), SLOT(doOnlineConnected()));
	connect(parent, SIGNAL(showPreferences()), SIGNAL(showYapsiPreferences()));

	updateActiveProfile();

	YaIPC::connect(this, SLOT(ipcMessage(const QString&)));
	checkForAliveOnlineProcess();
}

YaOnline::~YaOnline()
{
}

void YaOnline::setController(PsiCon* controller)
{
	controller_ = controller;

	if (controller_) {
		controller_->setYaOnline(this);
		connect(controller_->contactList(), SIGNAL(accountCountChanged()), SLOT(accountCountChanged()));
		disconnect(GlobalEventQueue::instance(), SIGNAL(queueChanged()), this, SLOT(eventQueueChanged()));
		connect(GlobalEventQueue::instance(),    SIGNAL(queueChanged()), this, SLOT(eventQueueChanged()), Qt::QueuedConnection);
	}
}

void YaOnline::onlineObjectChanged()
{
	// doesn't seem to work as "init()" for some weird reason
	// server_->dynamicCall("init(const QString&)", QString());
}

void YaOnline::accountCountChanged()
{
	if (!controller_)
		return;

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		disconnect(account, SIGNAL(lastMailNotify(const XMPP::Message&)), this, SLOT(lastMailNotify(const XMPP::Message&)));
		connect(account,    SIGNAL(lastMailNotify(const XMPP::Message&)), this, SLOT(lastMailNotify(const XMPP::Message&)));
	}
}

void YaOnline::lastMailNotify(const XMPP::Message& msg)
{
	Q_ASSERT(!msg.lastMailNotify().isNull());

	Q_UNUSED(msg);
	// DummyStream stream;
	// XMPP::Stanza s = msg.toStanza(&stream);
	// server_->dynamicCall("lastMailNotify(const QString&)", s.toString());
}

struct YaOnlineEventData {
	int id;
	PsiAccount* account;
	QString jid;

	YaOnlineEventData()
		: id(-1)
		, account(0)
	{}

	bool isNull() const
	{
		return (id == -1) && (!account || jid.isEmpty());
	}
};

static YaOnlineEventData yaOnlineIdToEventData(PsiCon* controller, const QString& id)
{
	YaOnlineEventData result;

	QStringList data = id.split(":");
	Q_ASSERT(data.size() == 2);
	Q_ASSERT(data[0] == "jabbinchat");
	QDomDocument doc;
	if (!doc.setContent(QCA::Base64().decodeString(data[1])))
		return result;

	QDomElement root = doc.documentElement();
	if (root.tagName() != "notify")
		return result;

	result.id = root.attribute("id").toInt();
	result.account = controller->contactList()->getAccount(root.attribute("account"));
	result.jid = root.attribute("jid");
	return result;
}

void YaOnline::doToasterClicked(const QString& id)
{
	if (!controller_)
		return;

	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull()) {
		if (GlobalEventQueue::instance()->ids().contains(eventData.id)) {
			bool deleteEvent = false;
			PsiEvent* event = GlobalEventQueue::instance()->peek(eventData.id);
			if (event->type() == PsiEvent::Auth) {
				AuthEvent* authEvent = static_cast<AuthEvent*>(event);
				if (authEvent->authType() == "subscribe") {
					notify(eventData.id, event);
					return;
				}

				deleteEvent = true;
			}
			else if (event->type() == PsiEvent::Mood) {
				deleteEvent = true;
			}

			if (deleteEvent) {
				event->account()->eventQueue()->dequeue(event);
				delete event;
			}
		}

		YaPopupNotification::openNotify(-1, eventData.account, eventData.jid,
		                                UserAction);
	}

	eventQueueChanged();
}

void YaOnline::doToasterScreenLocked(const QString& id)
{
	if (!controller_)
		return;

	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull()) {
		if (GlobalEventQueue::instance()->ids().contains(eventData.id)) {
			PsiEvent* event = GlobalEventQueue::instance()->peek(eventData.id);
			event->setShownInOnline(false);
			emit event->account()->eventQueue()->queueChanged();
		}
	}

	eventQueueChanged();
}

bool YaOnline::doToasterIgnored(PsiAccount* account, const XMPP::Jid& jid)
{
	bool found = false;
	foreach(TabDlg* tabDlg, controller_->tabManager()->tabSets()) {
		TabbableWidget* tab = tabDlg->getTabPointer(account, jid.full());
		YaChatDlg* chat = tab ? dynamic_cast<YaChatDlg*>(tab) : 0;
		if (chat) {
			found = true;
			if (!tabDlg->isVisible()) {
				tabDlg->showWithoutActivation();
			}

			if (!chat->isActiveWindow()) {
				tabDlg->selectTab(chat);
			}
			chat->addPendingMessage();
		}
	}

	return found;
}

void YaOnline::doToasterIgnored(const QString& id)
{
	if (!controller_)
		return;

	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull()) {
		if (GlobalEventQueue::instance()->ids().contains(eventData.id)) {
			PsiEvent* event = GlobalEventQueue::instance()->peek(eventData.id);

			if (event->type() == PsiEvent::Message) {
				Q_ASSERT(event->account());
				bool found = doToasterIgnored(event->account(), event->from());

				if (!found && eventData.account && !controller_->tabManager()->tabSets().isEmpty()) {
					eventData.account->actionOpenSavedChat(event->from().full());
					doToasterIgnored(event->account(), event->from());
				}
			}
			else if (event->type() == PsiEvent::Auth) {
				// doToasterScreenLocked(id);
			}
			else if (event->type() == PsiEvent::Mood) {
				// nothing
			}
			else {
				Q_ASSERT(false);
			}
		}
	}

	eventQueueChanged();
}

void YaOnline::doToasterSkipped(const QString& id)
{
	if (!controller_)
		return;

	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull()) {
		if (GlobalEventQueue::instance()->ids().contains(eventData.id)) {
			PsiEvent* event = GlobalEventQueue::instance()->peek(eventData.id);
			event->account()->eventQueue()->dequeue(event);
			delete event;
		}
	}

	eventQueueChanged();
}

void YaOnline::doShowIgnoredToasters()
{
	if (!controller_)
		return;

	static bool showingIgnoredToasters = false;
	if (showingIgnoredToasters)
		return;

	showingIgnoredToasters = true;

	foreach(int id, GlobalEventQueue::instance()->ids()) {
		// when chat to contact is opened it could delete some
		// more unread events related to that contact
		if (!GlobalEventQueue::instance()->ids().contains(id))
			continue;

		PsiEvent* event = GlobalEventQueue::instance()->peek(id);
		Q_ASSERT(event);
		if (!event)
			continue;

		if (event->type() == PsiEvent::Message) {
			if (event->account()) {
				// we need to open all the chats without activating them in the process
				event->account()->actionOpenSavedChat(event->from().full());
				doToasterIgnored(event->account(), event->from());
			}
		}
		else if (event->type() == PsiEvent::Auth) {
			// if (!event->shownInOnline()) {
			// 	notify(id, event);
			// }
		}
		else if (event->type() == PsiEvent::Mood) {
			// nothing
		}
		else {
			Q_ASSERT(false);
		}
	}

	foreach(TabDlg* tabDlg, controller_->tabManager()->tabSets()) {
		bringToFront(tabDlg);
	}

	showingIgnoredToasters = false;
}

static QString yaOnlineNotifyMid(int id, PsiAccount* account, const XMPP::Jid& jid)
{
	QDomDocument doc;
	QDomElement root = doc.createElement("notify");
	doc.appendChild(root);
	root.setAttribute("id",      QString::number(id));
	root.setAttribute("account", account->id());
	root.setAttribute("jid",     jid.full());

	return QString("yachat:%1")
	       .arg(QCA::Base64().encodeString(doc.toString()));
}

static QString yaOnlineNotifyMid(int id, PsiEvent* event)
{
	return yaOnlineNotifyMid(id, event->account(), event->from());
}

void YaOnline::notifyAllUnshownEvents()
{
	if (!controller_)
		return;

	foreach(int id, GlobalEventQueue::instance()->ids()) {
		if (!GlobalEventQueue::instance()->ids().contains(id))
			continue;

		PsiEvent* event = GlobalEventQueue::instance()->peek(id);
		if (event->shownInOnline()) {
			doToasterIgnored(yaOnlineNotifyMid(id, event));
			continue;
		}

		notify(id, event);
	}

	eventQueueChanged();
}

void YaOnline::doScreenUnlocked()
{
	if (!controller_)
		return;

	notifyAllUnshownEvents();
}

QString YaOnline::avatarPath(PsiAccount* account, const XMPP::Jid& jid)
{
	QString fileName = account->avatarFactory()->getCachedAvatarFileName(jid);
	if (fileName.isEmpty() || !QFile::exists(fileName)) {
		PsiContact* contact = account->findContact(jid);
		XMPP::VCard::Gender gender = contact ? contact->gender() : XMPP::VCard::UnknownGender;
		fileName = Ya::VisualUtil::noAvatarPixmapFileName(gender);
	}

	QFileInfo fi(fileName);
	QString scaledAvatar = QString("%1_toaster.png").arg(fi.baseName());
	QDir avatarsDir(account->avatarFactory()->getCacheDir());
	QString scaledAvatarPath = avatarsDir.absoluteFilePath(scaledAvatar);
	if (!QFile::exists(scaledAvatarPath)) {
		QImage image(fileName);
		static const int toasterAvatarSize = 50;
		image = image.scaled(QSize(toasterAvatarSize, toasterAvatarSize),
		                     Qt::KeepAspectRatio, Qt::SmoothTransformation);
		image.save(scaledAvatarPath);
	}

	return scaledAvatarPath;
}

QString YaOnline::contactName(PsiAccount* account, const XMPP::Jid& jid)
{
	PsiContact* contact = 0;
	if (account)
		contact = account->findContact(jid.bare());

	QString result = contact ? contact->name() : jid.bare();
	result = Ya::contactName(result, jid.bare());
	return result;
}

QString YaOnline::contactGender(PsiAccount* account, const XMPP::Jid& jid)
{
	PsiContact* contact = account->findContact(jid);
	XMPP::VCard::Gender gender = contact ? contact->gender() : XMPP::VCard::UnknownGender;
	if (gender == XMPP::VCard::Male)
		return "m";
	else if (gender == XMPP::VCard::Female)
		return "f";
	return "u";
}

void YaOnline::showToaster(const QString& type, PsiAccount* account, const XMPP::Jid& jid, const QString& _message, const QDateTime& timeStamp, const QString& callbackId)
{
	QString message = Ya::limitText(_message, 300);
	message = TextUtil::plain2richSimple(message);
	bool processMessageNotifierText = true;

	bool showToaster = false;
	if (type == "chat") {
		showToaster = PsiOptions::instance()->getOption("options.ya.popups.message.enable").toBool();
	}
	else if (type == "mood") {
		showToaster = PsiOptions::instance()->getOption("options.ya.popups.moods.enable").toBool();
		processMessageNotifierText = false;
	}
	else {
		Q_ASSERT(false);
	}

	QString emoticonified = message;
	if (processMessageNotifierText) {
		emoticonified = Ya::messageNotifierText(message);
	}
	emoticonified = TextUtil::emoticonifyForYaOnline(emoticonified);

	server_->dynamicCall("showToaster(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, bool)",
	                     emoticonified, contactName(account, jid), avatarPath(account, jid), timeStamp, callbackId, type, message, showToaster);
}

void YaOnline::notify(int id, PsiEvent* event)
{
	if (!controller_)
		return;

	event->setShownInOnline(true);
	emit event->account()->eventQueue()->queueChanged();

	if (event->type() == PsiEvent::Mood) {
		MoodEvent* moodEvent = static_cast<MoodEvent*>(event);

		showToaster("mood",
		            event->account(), event->from(),
		            moodEvent->mood().trimmed(),
		            moodEvent->timeStamp(),
		            yaOnlineNotifyMid(id, event));
		return;
	}

	if (event->type() == PsiEvent::Auth) {
		AuthEvent* authEvent = static_cast<AuthEvent*>(event);
		server_->dynamicCall("authRequest(const QString&, const QString&, const QString&, const QString&, const QString&)",
		                     yaOnlineNotifyMid(id, event),
		                     contactName(event->account(), event->from()),
		                     avatarPath(event->account(), event->from()),
		                     authEvent->authType(),
		                     contactGender(event->account(), event->from()));
		return;
	}

	// only message events are supported for now
	if (event->type() != PsiEvent::Message) {
		return;
	}

	XMPP::Message m;
	m.setFrom("lastmail.jabbin.com/jabbin");
	YaLastMail lastMail;

	lastMail.subject   = event->description();
	lastMail.timeStamp = event->timeStamp();
	lastMail.mid       = yaOnlineNotifyMid(id, event);
	m.setLastMailNotify(lastMail);

	showToaster("chat",
	            event->account(), event->from(),
	            lastMail.subject,
	            lastMail.timeStamp,
	            lastMail.mid);
}

void YaOnline::openUrl(const QString& url, bool isYandex)
{
	server_->dynamicCall("openUrl(const QString&, bool)",
	                     url, isYandex);
}

void YaOnline::clearedMessageHistory()
{
	server_->dynamicCall("eraseHistory()");
}

void YaOnline::ipcMessage(const QString& message)
{
	static bool uninstalling = false;
	if (message == "quit:uninstalling") {
		if (uninstalling) {
			return;
		}
		uninstalling = true;
		PsiLogger::instance()->log("YaOnline::ipcMessage(): uninstalling");
		server_->dynamicCall("uninstall(const QString&)", QString());
	}
	else if (message == "quit:installing") {
		PsiLogger::instance()->log("YaOnline::ipcMessage(): installing");
		server_->shutdown();
	}
}

static bool processIsRunning(const QString& processName)
{
	bool processFound = false;

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32 pe;
	memset(&pe, 0, sizeof(PROCESSENTRY32));
	pe.dwSize = sizeof(PROCESSENTRY32);

	bool result = false;
	result = Process32First(snapshot, &pe);
	while (result) {
		QString exeFile = QString::fromWCharArray(pe.szExeFile);
		if (processName.toLower() == exeFile.toLower()) {
			processFound = true;
			break;
		}
		result = Process32Next(snapshot, &pe);
	}
	CloseHandle(snapshot);
	return processFound;
}

bool YaOnline::onlineIsRunning()
{
	return processIsRunning("online.exe");
}

void YaOnline::checkForAliveOnlineProcess()
{
	if (!onlineIsRunning()) {
		PsiLogger::instance()->log("YaOnline::checkForAliveOnlineProcess(): shutting down");
		server_->shutdown();
	}
	else {
		QTimer::singleShot(500, this, SLOT(checkForAliveOnlineProcess()));
	}
}

PsiAccount* YaOnline::onlineAccount() const
{
	if (!controller_)
		return 0;

	PsiAccount* result = controller_->contactList()->onlineAccount();
	Q_ASSERT(result);
	Q_ASSERT(!result->userAccount().saveable);
	return result;
}

void YaOnline::updateOnlineAccount()
{
	if (!onlineAccount())
		return;

	UserAccount acc = onlineAccount()->userAccount();
	XMPP::Jid jid("foo@bar");
	jid.setDomain("jabbin.com");
	jid.setNode(ycuApi_->getUsername());
	if (!jid.node().isEmpty())
		acc.jid = jid.full();
	else
		acc.jid = QString();
	acc.pass = ycuApi_->getPassword();
	acc.opt_enabled = !acc.jid.isEmpty() && !acc.pass.isEmpty();
	onlineAccount()->setUserAccount(acc);
}

void YaOnline::updateProxySettings()
{
	if (!controller_)
		return;

	ProxyItemList proxyItems;
	ProxyItem proxy;
	proxy.id = 0;
	proxy.name = "Proxy";
	proxy.type = "http";
	proxy.settings.host = ycuApi_->getProxyHost();
	proxy.settings.port = ycuApi_->getProxyPort();
	proxy.settings.useAuth = false;
	proxy.settings.user = "";
	proxy.settings.pass = "";
	if (!proxy.settings.host.isEmpty() && proxy.settings.port > 0) {
		proxyItems << proxy;
	}

	controller_->proxy()->setItemList(proxyItems);

	foreach(PsiAccount* account, controller_->contactList()->accounts()) {
		Q_ASSERT(!account->isAvailable());
		UserAccount acc = account->userAccount();
		acc.proxy_index = proxyItems.count();
		account->setUserAccount(acc);
	}
}

void YaOnline::updateMiscSettings()
{
	// useSound = ycuApi_->getSoundsEnabled();
}

void YaOnline::rereadSettings()
{
	ycuApi_->rereadData();

	updateOnlineAccount();
	updateProxySettings();
	updateMiscSettings();

	// TODO
}

void YaOnline::disconnectRereadSettingsAndReconnect()
{
	doSetOfflineMode();
	rereadSettings();
	emit forceStatus(lastStatus_);
}

void YaOnline::doSetOfflineMode()
{
	if (!controller_)
		return;

	XMPP::Status::Type status = controller_->lastLoggedInStatusType();
	if (status != XMPP::Status::Offline)
		lastStatus_ = status;
	emit forceStatus(XMPP::Status::Offline);
}

void YaOnline::doSetDND(bool isDND)
{
	lastStatus_ = isDND ? XMPP::Status::DND : XMPP::Status::Online;
	if (!controller_)
		return;

	emit forceStatus(lastStatus_);
}

/**
 * Ensures that all enabled accounts are in active state, if they don't have
 * auto-connects explicitly disabled (in order to avoid resource conflicts)
 */
void YaOnline::doOnlineConnected()
{
	if (!controller_)
		return;

	foreach(PsiAccount* account, controller_->contactList()->enabledAccounts()) {
		if (!account->isAvailable() && !account->disableAutoConnect()) {
			account->setStatus(makeStatus(lastStatus() == XMPP::Status::DND ? XMPP::Status::DND : XMPP::Status::Online,
			                              controller_->currentStatusMessage()), false);
		}
	}
}

void YaOnline::doSoundsChanged()
{
	ycuApi_->rereadData();
	updateMiscSettings();
}

void YaOnline::doQuit()
{
	server_->dynamicCall("chatExit(const QString&)", QString());
}

void YaOnline::showSidebar()
{
	server_->dynamicCall("showSidebar(const QString&)", QString());
}

void YaOnline::showPreferences()
{
	server_->dynamicCall("showProperties(const QString&)", QString());
}

void YaOnline::setErrorMessage(const QString& error)
{
	server_->dynamicCall("setErrorMessage(const QString&)", error);
}

void YaOnline::clearErrorMessage()
{
	server_->dynamicCall("clearErrorMessage(const QString&)", QString());
}

void YaOnline::doPingServer()
{
	if (!controller_)
		return;

	foreach(PsiAccount* account, controller_->contactList()->enabledAccounts()) {
		account->pingServer();
	}
}

void YaOnline::setDND(bool isDND)
{
	server_->dynamicCall("setDND(bool)", isDND);
}

// void YaOnline::doPlaySound(const QString& type)
// {
// 	if (!onlineAccount())
// 		return;
//
// 	if (type == "message") {
// 		onlineAccount()->playSound(eChat1);
// 	}
// }

static const int maximumEventCount = 99;
static QStringList skipEvents(QList<int>& goodIds, int type)
{
	QStringList skipIds;

	if (goodIds.count() > maximumEventCount) {
		QMutableListIterator<int> it(goodIds);
		while (it.hasNext()) {
			int id = it.next();
			PsiEvent* event = GlobalEventQueue::instance()->peek(id);
			Q_ASSERT(event);
			if (event->type() == type) {
				skipIds << yaOnlineNotifyMid(id, event);
				it.remove();
			}

			if (goodIds.count() <= maximumEventCount)
				break;
		}
	}

	return skipIds;
}

void YaOnline::eventQueueChanged()
{
	static bool inEventQueueChanged = false;

	if (!controller_)
		return;

	if (inEventQueueChanged)
		return;
	inEventQueueChanged = true;

	QList<int> goodIds;

	foreach(int id, GlobalEventQueue::instance()->ids()) {
		PsiEvent* event = GlobalEventQueue::instance()->peek(id);
		Q_ASSERT(event);
		if (event->type() == PsiEvent::Message ||
		    event->type() == PsiEvent::Auth ||
		    event->type() == PsiEvent::Mood)
		{
			goodIds += id;
		}
	}

	{
		QStringList skipIds;
		skipIds += skipEvents(goodIds, PsiEvent::Mood);
		skipIds += skipEvents(goodIds, PsiEvent::Auth);

		if (goodIds.count() > maximumEventCount) {
			bool onlyMessageEvents = true;
			bool lastEventIsMessage = false;
			foreach(int id, goodIds) {
				PsiEvent* event = GlobalEventQueue::instance()->peek(id);
				if (event->type() != PsiEvent::Message) {
					onlyMessageEvents = false;
					break;
				}
			}

			if (!goodIds.isEmpty()) {
				PsiEvent* event = GlobalEventQueue::instance()->peek(goodIds.last());
				lastEventIsMessage = event->type() == PsiEvent::Message;
			}

			if (onlyMessageEvents && lastEventIsMessage) {
				skipIds += skipEvents(goodIds, PsiEvent::Message);
			}
		}

		foreach(QString id, skipIds) {
			doToasterSkipped(id);
		}
	}

	QStringList mids;
	static QString separator = " ";

	int eventCount = 0;
	foreach(int id, goodIds) {
		PsiEvent* event = GlobalEventQueue::instance()->peek(id);

		Q_ASSERT(event);
		eventCount++;

		QString mid = yaOnlineNotifyMid(id, event);
		Q_ASSERT(!mid.contains(separator));

		Q_ASSERT(eventCount <= maximumEventCount);
		mids += mid;
	}

	server_->dynamicCall("setIgnoredToasters(const QString&)", mids.join(separator));

	inEventQueueChanged = false;
}

XMPP::Status::Type YaOnline::lastStatus() const
{
	return lastStatus_;
}

static QString processFromDirName(const QString& path, const QString& fromPath)
{
	QString result = path;
	result.replace(fromPath, QString());
	if (result.startsWith("/"))
		result.remove(0, 1);
	return result;
}

bool copyDir(const QString& _fromPath, const QString& toPath)
{

	QFileInfo fromDirInfo(_fromPath);
	QString fromPath = fromDirInfo.absoluteFilePath();
	if (fromDirInfo.isSymLink()) {
		fromPath = fromDirInfo.symLinkTarget();
	}

	QDir toDir(toPath);

	QDirIterator it(fromPath, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
	while (it.hasNext()) {
		it.next();

		QString path = it.fileInfo().absolutePath();
		if (!path.startsWith(fromPath)) {
			continue;
		}

		path = processFromDirName(path, fromPath);
		QString fileName = processFromDirName(it.filePath(), fromPath);

		if (path.isEmpty())
			toDir.mkpath(".");
		else
			toDir.mkpath(path);

		QString newFileName = toDir.filePath(fileName);
		if (it.fileInfo().isFile() && !QFile::exists(newFileName)) {
			QFile::copy(it.filePath(), newFileName);
		}
	}

	return true;
}

void YaOnline::updateActiveProfile()
{
	Q_ASSERT(ycuApi_);
	ycuApi_->rereadData();

	activeProfile_ = ycuApi_->getUsername();
	if (!activeProfile_.isEmpty()) {
		activeProfile_ = "ya_" + activeProfile_;

		if (!QDir(pathToProfile(activeProfile_)).exists() && QDir(pathToProfile("default")).exists()) {
			copyDir(pathToProfile("default"), pathToProfile(activeProfile_));
		}
	}
	else {
		activeProfile_ = "default";
	}
}

QString YaOnline::activeProfile() const
{
	if (activeProfile_.isEmpty()) {
		((YaOnline*)this)->updateActiveProfile();
	}

	return activeProfile_;
}

void YaOnline::doAuthAccept(const QString& id)
{
	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull() && eventData.account) {
		eventData.account->dj_auth(eventData.jid);
	}
}

void YaOnline::doAuthDecline(const QString& id)
{
	YaOnlineEventData eventData = yaOnlineIdToEventData(controller_, id);
	if (!eventData.isNull() && eventData.account) {
		eventData.account->dj_deny(eventData.jid);
	}
}
