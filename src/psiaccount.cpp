/*
 * psiaccount.cpp - handles a Psi account
 * Copyright (C) 2001-2005  Justin Karneges
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * You can also redistribute and/or modify this program under the
 * terms of the Psi License, specified in the accompanied COPYING
 * file, as published by the Psi Project; either dated January 1st,
 * 2005, or (at your option) any later version.
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


#include <QFileDialog>
#include <qinputdialog.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qpointer.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qobject.h>
#include <qmap.h>
#include <qca.h>
#include <qfileinfo.h>
#include <QPixmap>
#include <QFrame>
#include <QList>
#include <QHostInfo>
#include <Qt3Support/Q3PtrListIterator>

#include "calldialog.h"
#include "psiaccount.h"
#include "psiiconset.h"
#include "psicon.h"
#include "profiles.h"
#include "xmpp_tasks.h"
#include "xmpp_xmlcommon.h"
#include "pongserver.h"
#include "s5b.h"
#ifdef FILETRANSFER
#include "filetransfer.h"
#endif
#include "pgpkeydlg.h"
#include "psioptions.h"
#include "textutil.h"
#include "httpauthmanager.h"
#include "pgputil.h"
#include "applicationinfo.h"
#include "pgptransaction.h"
#include "accountmanagedlg.h"
#include "changepwdlg.h"
#include "xmlconsole.h"
#include "userlist.h"
#include "psievent.h"
#include "jidutil.h"
#include "eventdlg.h"
#ifdef YAPSI
#include "yaprivacymanager.h"
#else
#include "psiprivacymanager.h"
#endif
#include "rosteritemexchangetask.h"
#include "chatdlg.h"
#include "contactview.h"
#include "mood.h"
#include "tune.h"
#ifdef USE_PEP
#include "tunecontroller.h"
#endif
#ifdef GROUPCHAT
#include "groupchatdlg.h"
#endif
#include "statusdlg.h"
#include "infodlg.h"
#include "adduserdlg.h"
#include "historydlg.h"
#include "capsmanager.h"
#include "registrationdlg.h"
#include "searchdlg.h"
#include "discodlg.h"
#include "eventdb.h"
#include "accountmodifydlg.h"
#include "passphrasedlg.h"
#include "voicecaller.h"
// ivan: disabling old CallDlg: #include "voicecalldlg.h"
// ivan: disabling old CallDlg: #include "calldlg.h"
#ifdef HAVE_JINGLE
#include "jinglevoicecaller.h"
#endif
#ifdef GOOGLE_FT
#include "googleftmanager.h"
#endif
#include "pepmanager.h"
#include "serverinfomanager.h"
#ifdef WHITEBOARDING
#include "wbmanager.h"
#endif
#include "bookmarkmanager.h"
#include "vcardfactory.h"
//#include "qssl.h"
#include "sslcertdlg.h"
#include "mooddlg.h"
#include "qwextend.h"
#include "geolocation.h"
#include "physicallocation.h"
#include "psipopup.h"
#include "pgputil.h"
#include "translationmanager.h"
#include "irisprotocol/iris_discoinfoquerier.h"
#include "iconwidget.h"
#ifdef FILETRANSFER
#include "filetransdlg.h"
#endif
#include "systeminfo.h"
#include "avatars.h"
#include "ahcommanddlg.h"
#include "mucjoindlg.h"
#include "ahcservermanager.h"
#include "rc.h"
#include "tabdlg.h"
#include "certutil.h"
#include "proxy.h"
#include "psicontactlist.h"
#include "psicontact.h"
#include "psiselfcontact.h"
#include "alertable.h"
#ifdef YAPSI
#include "yacommon.h"
#include "globaleventqueue.h"
#include "yapopupnotification.h"
#include "xmpp_yalastmail.h"
#include "yachatdlg.h"
#include "yatoastercentral.h"
#include "yarostertooltip.h"
#endif
#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif
#include "tabmanager.h"

#ifdef PSI_PLUGINS
#include "pluginmanager.h"
#endif

#include <QtCrypto>

#if defined(Q_WS_MAC) && defined(HAVE_GROWL)
#include "psigrowlnotifier.h"
#endif

#include "bsocket.h"
/*#ifdef Q_WS_WIN
#include <windows.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif*/
#include "psilogger.h"

#include "tools/services/servicespanel.h"

using namespace XMPP;

// #define INITIAL_PRESENCE_AFTER_VCARD

struct GCContact
{
	Jid jid;
	Status status;
};

//----------------------------------------------------------------------------
// BlockTransportPopup -- blocks popups on transport status changes
//----------------------------------------------------------------------------

class BlockTransportPopupList;

class BlockTransportPopup : public QObject
{
	Q_OBJECT
public:
	BlockTransportPopup(QObject *, const Jid &);

	Jid jid() const;

private slots:
	void timeout();

private:
	Jid j;
	int userCounter;
	friend class BlockTransportPopupList;
};

BlockTransportPopup::BlockTransportPopup(QObject *parent, const Jid &_j)
: QObject(parent)
{
	j = _j;
	userCounter = 0;

	// Hack for ICQ SMS
	if ( j.host().left(3) == "icq" ) {
		new BlockTransportPopup(parent, "sms." + j.host()); // sms.icq.host.com
		new BlockTransportPopup(parent, "sms"  + j.host().right(j.host().length() - 3)); // sms.host.com
	}

	QTimer::singleShot(15000, this, SLOT(timeout()));
}

void BlockTransportPopup::timeout()
{
	if ( userCounter > 1 ) {
		QTimer::singleShot(15000, this, SLOT(timeout()));
		userCounter = 0;
	}
	else
		deleteLater();
}

Jid BlockTransportPopup::jid() const
{
	return j;
}

//----------------------------------------------------------------------------
// BlockTransportPopupList
//----------------------------------------------------------------------------

class BlockTransportPopupList : public QObject
{
	Q_OBJECT
public:
	BlockTransportPopupList();

	bool find(const Jid &, bool online = false);
};

BlockTransportPopupList::BlockTransportPopupList()
: QObject()
{
}

bool BlockTransportPopupList::find(const Jid &j, bool online)
{
	if ( j.user().isEmpty() ) // always show popups for transports
		return false;

	QList<BlockTransportPopup *> list = findChildren<BlockTransportPopup *>();
	foreach(BlockTransportPopup* btp, list) {
		if ( j.host() == btp->jid().host() ) {
			if ( online )
				btp->userCounter++;
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------
// PsiAccount
//----------------------------------------------------------------------------

class PsiAccount::Private : public Alertable
{
	Q_OBJECT
public:
	Private(PsiAccount *parent)
		: Alertable(parent)
		, selfContact(0)
		, contactList(0)
		, psi(0)
		, account(parent)
		, options(0)
		, client(0)
		, eventQueue(0)
		, xmlConsole(0)
		, blockTransportPopupList(0)
		, privacyManager(0)
		, capsManager(0)
		, rosterItemExchangeTask(0)
		, ahcManager(0)
		, rcSetStatusServer(0)
		, rcSetOptionsServer(0)
		, rcForwardServer(0)
		, avatarFactory(0)
		, voiceCaller(0)
		, tabManager(0)
#ifdef GOOGLE_FT
		, googleFTManager(0)
#endif
#ifdef WHITEBOARDING
		, wbManager(0)
#endif
		, serverInfoManager(0)
		, pepManager(0)
		, bookmarkManager(0)
		, httpAuthManager(0)
		, conn(0)
		, stream(0)
		, tls(0)
		, tlsHandler(0)
		, xmlRingbuf(1000)
		, xmlRingbufWrite(0)
		, doPopups_(true)
		, pongServer_(0)
		, pingServerTimer_(0)
		, lastNumberOfSecondsIdle_(0)
	{
		oldStatus_.setType(Status::Offline);
		pingServerTimer_ = new QTimer(this);
		pingServerTimer_->setSingleShot(true);
		pingServerTimer_->setInterval(5000);
		connect(pingServerTimer_, SIGNAL(timeout()), SLOT(pingServerTimerTimeout()));
	}

	PsiContactList* contactList;
	PsiSelfContact* selfContact;
	PsiCon *psi;
	PsiAccount *account;
	PsiOptions *options;
	Client *client;
	UserAccount acc, accnext;
	Jid jid, nextJid;
	Status loginStatus;
	bool loginWithPriority;
	EventQueue *eventQueue;
	XmlConsole *xmlConsole;
	UserList userList;
	UserListItem self;
	int lastIdle;
	bool nickFromVCard;
	QCA::PGPKey cur_pgpSecretKey;
	QList<Message*> messageQueue;
	BlockTransportPopupList *blockTransportPopupList;
	int userCounter;
	PsiPrivacyManager* privacyManager;
	CapsManager* capsManager;
	RosterItemExchangeTask* rosterItemExchangeTask;
	bool pepAvailable;
	QString currentConnectionError;
	int currentConnectionErrorCondition;

	// Tune
	Tune lastTune;

	// Ad-hoc commands
	AHCServerManager* ahcManager;
	RCSetStatusServer* rcSetStatusServer;
	RCSetOptionsServer* rcSetOptionsServer;
	RCForwardServer* rcForwardServer;

	// Avatars
	AvatarFactory* avatarFactory;
	QString photoHash;

	// Voice Call
	VoiceCaller* voiceCaller;

	TabManager *tabManager;

#ifdef GOOGLE_FT
	// Google file transfer manager
	GoogleFTManager* googleFTManager;
#endif

#ifdef WHITEBOARDING
	// Whiteboard
	WbManager* wbManager;
#endif

	// PubSub
	ServerInfoManager* serverInfoManager;
	PEPManager* pepManager;

	// Bookmarks
	BookmarkManager* bookmarkManager;

	// HttpAuth
	HttpAuthManager* httpAuthManager;

	QList<GCContact*> gcbank;
	QStringList groupchats;

	QPointer<AdvancedConnector> conn;
	QPointer<ClientStream> stream;
	QPointer<QCA::TLS> tls;
	QPointer<QCATLSHandler> tlsHandler;
	bool usingSSL;

	QVector<xmlRingElem> xmlRingbuf;
	int xmlRingbufWrite;

	QHostAddress localAddress;

	QList<PsiContact*> contacts;

private:
	bool doPopups_;

public:
	bool noPopup(ActivationType activationType) const
	{
		if (activationType == FromXml || !doPopups_)
			return true;

		if (lastManualStatus_.isAvailable()) {
			if (lastManualStatus_.type() == XMPP::Status::DND)
				return true;
			if ((lastManualStatus_.type() == XMPP::Status::Away || lastManualStatus_.type() == XMPP::Status::XA) && option.noAwayPopup)
				return true;
		}

		return false;
	}

private slots:
	void removeContact(PsiContact* contact)
	{
		Q_ASSERT(contacts.contains(contact));
		emit account->removedContact(contact);
		contacts.removeAll(contact);
	}

	/**
	 * TODO: make it work with multiple groups per contact.
	 * And with metacontacts too.
	 */
	PsiContact* addContact(const UserListItem& u)
	{
		Q_ASSERT(!findContact(u.jid()));
		// PsiContactGroup* parent = groupsForUserListItem(u).first();
		PsiContact* contact = new PsiContact(u, account);
		contacts.append(contact);
		connect(contact, SIGNAL(destroyed(PsiContact*)), SLOT(removeContact(PsiContact*)));
#ifdef YAPSI
		connect(contact, SIGNAL(moodChanged(const QString&)), account, SLOT(moodChanged(const QString&)));
#endif
		emit account->addedContact(contact);
		return contact;
	}

public:
	PsiContact* findContact(const Jid& jid) const
	{
		foreach(PsiContact* contact, contacts)
			if (contact->find(jid))
				return contact;

		return 0;
	}

	PsiContact* findContactOrSelf(const Jid& jid) const
	{
		PsiContact* contact = findContact(jid);
		if (!contact) {
			if (selfContact->find(jid)) {
				contact = selfContact;
			}
		}
		return contact;
	}

	QStringList groupList() const
	{
		QStringList groupList;

		foreach(PsiContact* contact, contacts)
			foreach(QString group, contact->userListItem().groups())
				if (!groupList.contains(group))
					groupList.append(group);

		groupList.sort();
		return groupList;
	}

	QString pathToProfileEvents() const
	{
		return pathToProfile(activeProfile) + "/events-" + JIDUtil::encode(acc.id).toLower() + ".xml";
	}

	// FIXME: Rename updateEntry -> updateContact
	void updateEntry(const UserListItem& u)
	{
		if (u.isSelf()) {
			selfContact->update(u);
		}
		else {
			PsiContact* contact = findContact(u.jid());
			if (!contact) {
				contact = addContact(u);
			}
			else {
				contact->update(u);
			}
		}
	}

	// FIXME: Rename removeEntry -> removeContact
	void removeEntry(const Jid& jid)
	{
		PsiContact* contact = findContact(jid);
		Q_ASSERT(contact);
		delete contact;
		emit account->removeContact(jid);
	}

	void setState(int state)
	{
		const PsiIcon* alert = 0;
		if (state == -1)
			alert = IconsetFactory::iconPtr("psi/connect");
		Alertable::setAlert(alert);
	}

	void setAlert(const Jid& jid, const PsiIcon* icon)
	{
		PsiContact* contact = findContactOrSelf(jid);
		if (contact)
			contact->setAlert(icon);
	}

	void clearAlert(const Jid& jid)
	{
		PsiContact* contact = findContactOrSelf(jid);
		if (contact)
			contact->setAlert(0);
	}

	void animateNick(const Jid& jid)
	{
		// TODO
		Q_UNUSED(jid);
	}

private:
	void alertFrameUpdated()
	{
		// account->emitDataUpdated();
	}

public slots:
	void queueChanged()
	{
		eventQueue->toFile(pathToProfileEvents());
	}

	void loadQueue()
	{
		bool soundEnabled = useSound;
		useSound  = false; // disable the sound and popups
		doPopups_ = false;

		QFileInfo fi( pathToProfileEvents() );
		if ( fi.exists() )
			eventQueue->fromFile(pathToProfileEvents());

		useSound = soundEnabled;
		doPopups_ = true;
	}

	void setEnabled( bool e )
	{
		acc.opt_enabled = e;
		account->cpUpdate(self);
		// account->updateParent();

		eventQueue->setEnabled(e);

		// signals
		account->enabledChanged();
		account->updatedAccount();
	}

	void client_xmlIncoming(const QString &s)
	{
		xmlRingbuf[xmlRingbufWrite].type = RingXmlIn;
		xmlRingbuf[xmlRingbufWrite].xml = s;
		xmlRingbuf[xmlRingbufWrite].time = QDateTime::currentDateTime();
		xmlRingbufWrite = (xmlRingbufWrite + 1) % xmlRingbuf.count();
	}
	void client_xmlOutgoing(const QString &s)
	{
		xmlRingbuf[xmlRingbufWrite].type = RingXmlOut;
		xmlRingbuf[xmlRingbufWrite].xml = s;
		xmlRingbuf[xmlRingbufWrite].time = QDateTime::currentDateTime();
		xmlRingbufWrite = (xmlRingbufWrite + 1) % xmlRingbuf.count();
	}

	void vcardChanged(const Jid &j)
	{
		// our own vcard?
		if(j.compare(jid, false)) {
			const VCard *vcard = VCardFactory::instance()->vcard(j);
			if(vcard) {
				vcardPhotoUpdate(vcard->photo());
			}
		}
	}

	void vcardPhotoUpdate(const QByteArray &photoData)
	{
		QString newHash;
		if(!photoData.isEmpty()) {
			newHash = QCA::Hash("sha1").hashToString(photoData);
		}
		if(newHash != photoHash) {
			photoHash = newHash;
			account->setStatusDirect(loginStatus);
		}
	}

private:
	struct item_dialog2
	{
		QWidget *widget;
		Jid jid;
	};

	QList<item_dialog2*> dialogList;

	bool compareJids(const Jid& j1, const Jid& j2, bool compareResource) const
	{
		return j1.compare(j2, compareResource);
	}

public:
	// implementation for QList<PsiAccount::xmlRingElem> PsiAccount::dumpRingbuf()
	QList< xmlRingElem > dumpRingbuf()
	{
		xmlRingElem el;
		QList< xmlRingElem > ret;
		for(int i=0; i < xmlRingbuf.count(); i++) {
			el = xmlRingbuf[(xmlRingbufWrite + 1 + i) % xmlRingbuf.count()];
			if (!el.xml.isEmpty()) {
				ret += el;
			}
		}
		return ret;
	}
	QWidget* findDialog(const QMetaObject& mo, const Jid& jid, bool compareResource) const
	{
		foreach(item_dialog2* i, dialogList) {
			if (mo.cast(i->widget) && compareJids(i->jid, jid, compareResource))
				return i->widget;
		}
		return 0;
	}

	void findDialogs(const QMetaObject& mo, const Jid& jid, bool compareResource, QList<void*>* list) const
	{
		foreach(item_dialog2* i, dialogList) {
			if (mo.cast(i->widget) && compareJids(i->jid, jid, compareResource))
				list->append(i->widget);
		}
	}

	void findDialogs(const QMetaObject& mo, QList<void*>* list) const
	{
		foreach(item_dialog2* i, dialogList) {
			if (mo.cast(i->widget))
				list->append(i->widget);
		}
	}

	void dialogRegister(QWidget* w, const Jid& jid)
	{
		connect(w, SIGNAL(destroyed(QObject*)), SLOT(forceDialogUnregister(QObject*)));
		item_dialog2 *i = new item_dialog2;
		i->widget = w;
		i->jid = jid;
		dialogList.append(i);
	}

	void dialogUnregister(QWidget* w)
	{
		foreach(item_dialog2 *i, dialogList) {
			if (i->widget == w) {
				dialogList.removeAll(i);
				delete i;
				return;
			}
		}
	}

	void deleteDialogList()
	{
		while (!dialogList.isEmpty()) {
			item_dialog2* i = dialogList.takeFirst();

			delete i->widget;
			delete i;
		}
	}

private slots:
	void forceDialogUnregister(QObject* obj)
	{
		dialogUnregister(static_cast<QWidget*>(obj));
	}

public slots:
	void doModify()
	{
		AccountModifyDlg *w = account->findDialog<AccountModifyDlg*>();
		if(w)
			bringToFront(w);
		else {
			w = new AccountModifyDlg(account, 0);
			w->show();
		}
	}

public:
	enum AutoAway {
		AutoAway_None = 0,
		AutoAway_Away,
		AutoAway_XA,
		AutoAway_DND,
		AutoAway_Offline
	};

	void resetAutoAway()
	{
		if (oldStatus_.type() == Status::Offline) {
		    return;
		}
		account->setStatusDirect(oldStatus_);
		oldStatus_.setType(Status::Offline);
	}

	void setAutoAway(AutoAway autoAway)
	{
		if (!account->isAvailable())
			return;

		if (oldStatus_.type() == Status::Offline) {
			oldStatus_ = account->status();
		}

		Status status = autoAwayStatus(autoAway);
#ifdef YAPSI
		status.setStatus(loginStatus.status());
#endif
		if (status.type() != loginStatus.type() ||
		    status.status() != loginStatus.status())
		{
			account->setStatusDirect(status);
		}
	}

	void setManualStatus(Status status)
	{
		lastManualStatus_ = status;
	}

	const Status& lastManualStatus() const
	{
		return lastManualStatus_;
	}

private:
	Status lastManualStatus_;

	XMPP::Status autoAwayStatus(AutoAway autoAway)
	{
		// if (!lastManualStatus_.isAway()) {
			switch (autoAway) {
			case AutoAway_Away:
				return Status(XMPP::Status::Away, option.asMessage, acc.priority);
			case AutoAway_DND:
				return Status(XMPP::Status::DND, option.asMessage, acc.priority);
			case AutoAway_XA:
				return Status(XMPP::Status::XA, option.asMessage, acc.priority);
			case AutoAway_Offline:
				return Status(Status::Offline, loginStatus.status(), acc.priority);
			default:
				;
			}
		//}
		return lastManualStatus_;
	}

#ifdef YAPSI
public:
	QString mood;
	bool askingForVCard;
	QHash<QString, QDateTime> lastTimeMoodChanged_;
	bool disableAutoConnect;
#endif

public slots:
	void pingServer()
	{
		if (!account->isAvailable())
			return;

		PingTask* pingTask = new PingTask(client->rootTask());
		connect(pingTask, SIGNAL(finished()), SLOT(pingFinished()));
		pingTask->ping(account->jid().domain());
		pingTask->go(true);

		pingServerTimer_->start();
	}

	void pingServerTimerTimeout()
	{
		account->logout();
		account->disconnect();
		account->setStatus(lastManualStatus_, false);
	}

	void serverPong()
	{
		lastPongTime_ = QDateTime::currentDateTime();
		pingServerTimer_->stop();
	}

	void pingFinished()
	{
		PingTask* pingTask = static_cast<PingTask*>(sender());
		if (pingTask->success()) {
			serverPong();
		}
	}

public:
	PongServer* pongServer_;
	QTimer* pingServerTimer_;
	QDateTime lastPongTime_;
	int lastNumberOfSecondsIdle_;
	Status oldStatus_;
};

PsiAccount::PsiAccount(const UserAccount &acc, PsiContactList *parent, CapsRegistry* capsRegistry, TabManager *tabManager)
	: QObject(parent)
{
	d = new Private( this );
	d->contactList = parent;
	d->tabManager = tabManager;
	d->psi = parent->psi();
	d->options = PsiOptions::instance();
	d->client = 0;
	d->userCounter = 0;
	d->avatarFactory = 0;
	d->voiceCaller = 0;
	d->blockTransportPopupList = new BlockTransportPopupList();

	v_isActive = false;
	isDisconnecting = false;
	notifyOnlineOk = false;
	doReconnect = false;
	presenceSent = false;
#ifdef YAPSI
	d->askingForVCard = false;
	d->disableAutoConnect = false;
#endif

	d->loginStatus = Status(Status::Offline);
	d->loginWithPriority = false;
	d->lastIdle = 0;
	d->setManualStatus(Status(Status::Offline, "", 0));

	d->eventQueue = new EventQueue(this);
	connect(d->eventQueue, SIGNAL(queueChanged()), SIGNAL(queueChanged()));
	connect(d->eventQueue, SIGNAL(queueChanged()), d, SLOT(queueChanged()));
	connect(d->eventQueue, SIGNAL(eventFromXml(PsiEvent *)), SLOT(eventFromXml(PsiEvent *)));
	d->userList.setAutoDelete(true);
	d->self = UserListItem(true);
	d->self.setSubscription(Subscription::Both);
	d->nickFromVCard = false;

	// we need to copy groupState, because later initialization will depend on that
	d->acc.groupState = acc.groupState;

	// stream
	d->conn = 0;
	d->tls = 0;
	d->tlsHandler = 0;
	d->stream = 0;
	d->usingSSL = false;

	// create Jabber::Client
	d->client = new Client;
	d->client->setOSName(SystemInfo::instance()->os());
	d->client->setTimeZone(SystemInfo::instance()->timezoneString(), SystemInfo::instance()->timezoneOffset());
	d->client->setClientName(ApplicationInfo::name());
	d->client->setClientVersion(ApplicationInfo::version());
	d->client->setCapsNode(ApplicationInfo::capsNode());
	d->client->setCapsVersion(ApplicationInfo::capsVersion());

	DiscoItem::Identity identity;
	identity.category = "client";
	identity.type = "pc";
	identity.name = ApplicationInfo::name();
	d->client->setIdentity(identity);

	QStringList features;
	features << "http://jabber.org/protocol/commands";
	features << "http://jabber.org/protocol/rosterx";
	features << "http://jabber.org/protocol/muc";
	features << "jabber:x:data";
	d->client->setFeatures(Features(features));

#ifdef FILETRANSFER
	d->client->setFileTransferEnabled(true);
#else
	d->client->setFileTransferEnabled(false);
#endif

	setSendChatState(option.messageEvents);

	//connect(d->client, SIGNAL(connected()), SLOT(client_connected()));
	//connect(d->client, SIGNAL(handshaken()), SLOT(client_handshaken()));
	//connect(d->client, SIGNAL(error(const StreamError &)), SLOT(client_error(const StreamError &)));
	//connect(d->client, SIGNAL(sslCertReady(const QSSLCert &)), SLOT(client_sslCertReady(const QSSLCert &)));
	//connect(d->client, SIGNAL(closeFinished()), SLOT(client_closeFinished()));
	//connect(d->client, SIGNAL(authFinished(bool, int, const QString &)), SLOT(client_authFinished(bool, int, const QString &)));
	connect(d->client, SIGNAL(rosterRequestFinished(bool, int, const QString &)), SLOT(client_rosterRequestFinished(bool, int, const QString &)));
	connect(d->client, SIGNAL(rosterItemAdded(const RosterItem &)), SLOT(client_rosterItemAdded(const RosterItem &)));
	connect(d->client, SIGNAL(rosterItemAdded(const RosterItem &)), SLOT(client_rosterItemUpdated(const RosterItem &)));
	connect(d->client, SIGNAL(rosterItemUpdated(const RosterItem &)), SLOT(client_rosterItemUpdated(const RosterItem &)));
	connect(d->client, SIGNAL(rosterItemRemoved(const RosterItem &)), SLOT(client_rosterItemRemoved(const RosterItem &)));
	connect(d->client, SIGNAL(resourceAvailable(const Jid &, const Resource &)), SLOT(client_resourceAvailable(const Jid &, const Resource &)));
	connect(d->client, SIGNAL(resourceUnavailable(const Jid &, const Resource &)), SLOT(client_resourceUnavailable(const Jid &, const Resource &)));
	connect(d->client, SIGNAL(presenceError(const Jid &, int, const QString &)), SLOT(client_presenceError(const Jid &, int, const QString &)));
	connect(d->client, SIGNAL(messageReceived(const Message &)), SLOT(client_messageReceived(const Message &)));
	connect(d->client, SIGNAL(subscription(const Jid &, const QString &, const QString&)), SLOT(client_subscription(const Jid &, const QString &, const QString&)));
	connect(d->client, SIGNAL(debugText(const QString &)), SLOT(client_debugText(const QString &)));
	connect(d->client, SIGNAL(groupChatJoined(const Jid &)), SLOT(client_groupChatJoined(const Jid &)));
	connect(d->client, SIGNAL(groupChatLeft(const Jid &)), SLOT(client_groupChatLeft(const Jid &)));
	connect(d->client, SIGNAL(groupChatPresence(const Jid &, const Status &)), SLOT(client_groupChatPresence(const Jid &, const Status &)));
	connect(d->client, SIGNAL(groupChatError(const Jid &, int, const QString &)), SLOT(client_groupChatError(const Jid &, int, const QString &)));
#ifdef FILETRANSFER
	connect(d->client->fileTransferManager(), SIGNAL(incomingReady()), SLOT(client_incomingFileTransfer()));
#endif
	connect(d->client, SIGNAL(beginImportRoster()), SIGNAL(beginBulkContactUpdate()));
	connect(d->client, SIGNAL(endImportRoster()), SIGNAL(endBulkContactUpdate()));
	connect(d->client, SIGNAL(xmlIncoming(const QString &)), d, SLOT(client_xmlIncoming(const QString &)));
	connect(d->client, SIGNAL(xmlOutgoing(const QString &)), d, SLOT(client_xmlOutgoing(const QString &)));

	// Privacy manager
#ifdef YAPSI
	d->privacyManager = new YaPrivacyManager(this);
	connect(d->privacyManager, SIGNAL(simulateContactOffline(const XMPP::Jid&)), SLOT(simulateContactOffline(const XMPP::Jid&)));
#else
	d->privacyManager = new PsiPrivacyManager(d->client->rootTask());
#endif

	// Caps manager
	d->capsManager = new CapsManager(d->client->jid(), capsRegistry, new IrisProtocol::DiscoInfoQuerier(d->client));
	d->capsManager->setEnabled(option.useCaps);

	// Roster item exchange task
	d->rosterItemExchangeTask = new RosterItemExchangeTask(d->client->rootTask());
	connect(d->rosterItemExchangeTask,SIGNAL(rosterItemExchange(const Jid&, const RosterExchangeItems&)),SLOT(actionRecvRosterExchange(const Jid&,const RosterExchangeItems&)));

	// Initialize server info stuff
	d->serverInfoManager = new ServerInfoManager(d->client);
	connect(d->serverInfoManager,SIGNAL(featuresChanged()),SLOT(serverFeaturesChanged()));

	// XMPP Ping
	d->pongServer_ = new PongServer(d->client->rootTask());
	connect(d->pongServer_, SIGNAL(serverPing()), d, SLOT(serverPong()));

	// Initialize PubSub stuff
	d->pepManager = new PEPManager(d->client, d->serverInfoManager);
	connect(d->pepManager,SIGNAL(itemPublished(const Jid&, const QString&, const PubSubItem&)),SLOT(itemPublished(const Jid&, const QString&, const PubSubItem&)));
	connect(d->pepManager,SIGNAL(itemRetracted(const Jid&, const QString&, const PubSubRetraction&)),SLOT(itemRetracted(const Jid&, const QString&, const PubSubRetraction&)));
	d->pepAvailable = false;

#ifdef WHITEBOARDING
	 // Initialize Whiteboard manager
	d->wbManager = new WbManager(d->client, this);
#endif

	// Avatars
	d->avatarFactory = new AvatarFactory(this);
	d->self.setAvatarFactory(avatarFactory());

	connect(VCardFactory::instance(), SIGNAL(vcardChanged(const Jid&)), d, SLOT(vcardChanged(const Jid&)));

	// Bookmarks
	d->bookmarkManager = new BookmarkManager(d->client);

#ifdef USE_PEP
	// Tune Controller
	connect(d->psi->tuneController(), SIGNAL(stopped()), SLOT(tuneStopped()));
	connect(d->psi->tuneController(), SIGNAL(playing(const Tune&)),SLOT(tunePlaying(const Tune&)));
#endif

	// HttpAuth
	d->httpAuthManager = new HttpAuthManager(d->client->rootTask());
	connect(d->httpAuthManager, SIGNAL(confirmationRequest(const PsiHttpAuthRequest &)), SLOT(incomingHttpAuthRequest(const PsiHttpAuthRequest &)));

	// Initialize Adhoc Commands server
	d->ahcManager = new AHCServerManager(this);
	d->rcSetStatusServer = 0;
	d->rcSetOptionsServer = 0;
	d->rcForwardServer = 0;
	setRCEnabled(option.useRC);

	// Plugins
#ifdef PSI_PLUGINS
	PluginManager::instance()->addAccount(this, d->client);
#endif

	// HTML
	if(PsiOptions::instance()->getOption("options.html.chat.render").toBool())
		d->client->addExtension("html",Features("http://jabber.org/protocol/xhtml-im"));

	d->selfContact = new PsiSelfContact(d->self, this);

	// restore cached roster
	for(Roster::ConstIterator it = acc.roster.begin(); it != acc.roster.end(); ++it)
		client_rosterItemUpdated(*it);

	// restore pgp key bindings
	for(VarList::ConstIterator kit = acc.keybind.begin(); kit != acc.keybind.end(); ++kit) {
		const VarListItem &i = *kit;
		UserListItem *u = find(Jid(i.key()));
		if(u) {
			u->setPublicKeyID(i.data());
			cpUpdate(*u);
		}
	}

	setUserAccount(acc);

	connect(d->psi, SIGNAL(emitOptionsUpdate()), SLOT(optionsUpdate()));
	//connect(d->psi, SIGNAL(pgpToggled(bool)), SLOT(pgpToggled(bool)));
	connect(&PGPUtil::instance(), SIGNAL(pgpKeysUpdated()), SLOT(pgpKeysUpdated()));

	d->psi->setToggles(d->acc.tog_offline, d->acc.tog_away, d->acc.tog_agents, d->acc.tog_hidden,d->acc.tog_self);

	d->setEnabled(d->acc.opt_enabled);

	// Listen to the capabilities manager
	connect(capsManager(),SIGNAL(capsChanged(const Jid&)),SLOT(capsChanged(const Jid&)));

	//printf("PsiAccount: [%s] loaded\n", name().latin1());
	d->xmlConsole = new XmlConsole(this);
	if(option.xmlConsoleOnLogin && d->acc.opt_enabled) {
		this->showXmlConsole();
		d->xmlConsole->enable();
	}

	// Voice Calling
#ifdef HAVE_JINGLE
	d->voiceCaller = new JingleVoiceCaller(this);
#endif
	if (d->voiceCaller) {
		d->client->addExtension("voice-v1", Features(QString("http://www.google.com/xmpp/protocol/voice/v1")));
		connect(d->voiceCaller,SIGNAL(incoming(const Jid&)),SLOT(incomingVoiceCall(const Jid &)));
	}

#ifdef GOOGLE_FT
	d->googleFTManager = new GoogleFTManager(client());
	d->client->addExtension("share-v1", Features(QString("http://www.google.com/xmpp/protocol/share/v1")));
	connect(d->googleFTManager,SIGNAL(incomingFileTransfer(GoogleFileTransfer*)),SLOT(incomingGoogleFileTransfer(GoogleFileTransfer*)));
#endif

	// Extended presence
	if (d->options->getOption("options.extended-presence.notify").toBool()) {
		QStringList pepNodes;
		pepNodes += "http://jabber.org/protocol/mood+notify";
		pepNodes += "http://jabber.org/protocol/tune+notify";
		pepNodes += "http://jabber.org/protocol/physloc+notify";
		pepNodes += "http://jabber.org/protocol/geoloc+notify";
		pepNodes += "http://www.xmpp.org/extensions/xep-0084.html#ns-metadata+notify";
		d->client->addExtension("ep-notify",Features(pepNodes));
	}

	// load event queue from disk
	QTimer::singleShot(0, d, SLOT(loadQueue()));

	d->contactList->link(this);

	ServicesPanel::instance()->init(this);
}

PsiAccount::~PsiAccount()
{
#ifdef __GNUC__
#warning "Uncomment these"
#endif
#ifdef PSI_PLUGINS
	// PluginManager::instance()->removeClient(this);
#endif
	// nuke all related dialogs
	deleteAllDialogs();

	logout(true);
	QString str = name();

	while (!d->messageQueue.isEmpty())
		delete d->messageQueue.takeFirst();

#ifdef FILETRANSFER
	d->psi->ftdlg()->killTransfers(this);
#endif

	if (d->voiceCaller)
		delete d->voiceCaller;

	delete d->ahcManager;
	delete d->privacyManager;
	delete d->capsManager;
	delete d->pepManager;
	delete d->serverInfoManager;
#ifdef WHITEBOARDING
	delete d->wbManager;
#endif
	delete d->bookmarkManager;
	delete d->client;
	delete d->httpAuthManager;
	cleanupStream();
	delete d->eventQueue;
	delete d->avatarFactory;

	delete d->blockTransportPopupList;

	d->contactList->unlink(this);
	delete d;

	//printf("PsiAccount: [%s] unloaded\n", str.latin1());
}

void PsiAccount::cleanupStream()
{
	delete d->stream;
	d->stream = 0;

	delete d->tls;
	d->tls = 0;
	d->tlsHandler = 0;

	delete d->conn;
	d->conn = 0;

	d->usingSSL = false;

	d->localAddress = QHostAddress();
}

bool PsiAccount::enabled() const
{
	return d->acc.opt_enabled;
}

void PsiAccount::setEnabled(bool e)
{
	if ( d->acc.opt_enabled == e )
		return;

	if (!e) {
#ifndef YAPSI
		if (eventQueue()->count()) {
			QMessageBox::information(0, tr("Error"), tr("Unable to disable the account, as it has pending events."));
			return;
		}
		if (isActive()) {
			if (QMessageBox::information(0, tr("Disable Account"), tr("The account is currently active.\nDo you want to log out ?"),QMessageBox::Yes,QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) == QMessageBox::Yes) {
#endif
				logout();
#ifndef YAPSI
			}
			else {
				return;
			}
		}
#endif
	}

	d->setEnabled( e );
}

bool PsiAccount::isActive() const
{
	return v_isActive;
}

bool PsiAccount::isConnected() const
{
	return (d->stream && d->stream->isAuthenticated());
}

/**
 * This function returns true when the account is connected to the server,
 * authenticated and is not currently trying to disconnect.
 */
bool PsiAccount::isAvailable() const
{
	return isConnected() && isActive() && loggedIn() && !isDisconnecting;
}

const QString & PsiAccount::name() const
{
	return d->acc.name;
}

const QString &PsiAccount::id() const
{
	return d->acc.id;
}

/**
 * TODO: FIXME: Has side-effects of updating toggle's status, clearing
 * the roster and keybindings.
 */
const UserAccount & PsiAccount::userAccount() const
{
	d->psi->getToggles(&d->acc.tog_offline, &d->acc.tog_away, &d->acc.tog_agents, &d->acc.tog_hidden, &d->acc.tog_self);

	// save the roster and pgp key bindings
	d->acc.roster.clear();
	d->acc.keybind.clear();
	UserListIt it(d->userList);
	for(UserListItem *u; (u = it.current()); ++it) {
		if(u->inList())
			d->acc.roster += *u;

		if(!u->publicKeyID().isEmpty())
			d->acc.keybind.set(u->jid().full(), u->publicKeyID());
	}

	return d->acc;
}

UserList *PsiAccount::userList() const
{
	return &d->userList;
}

Client *PsiAccount::client() const
{
	return d->client;
}

EventQueue *PsiAccount::eventQueue() const
{
	return d->eventQueue;
}

EDB *PsiAccount::edb() const
{
	return d->psi->edb();
}

PsiCon *PsiAccount::psi() const
{
	return d->psi;
}

AvatarFactory *PsiAccount::avatarFactory() const
{
	return d->avatarFactory;
}

VoiceCaller* PsiAccount::voiceCaller() const
{
	return d->voiceCaller;
}

PrivacyManager* PsiAccount::privacyManager() const
{
	return d->privacyManager;
}

CapsManager* PsiAccount::capsManager() const
{
	return d->capsManager;
}

bool PsiAccount::hasPGP() const
{
	return !d->cur_pgpSecretKey.isNull();
}

QHostAddress *PsiAccount::localAddress() const
{
	QString s = d->localAddress.toString();
	if (s.isEmpty() || s == "0.0.0.0")
		return 0;
	return &d->localAddress;
}

#ifdef YAPSI
struct HostPortOverride {
	HostPortOverride(QString _host, int _port, UserAccount::SSLFlag _ssl)
		: host(_host)
		, port(_port)
		, ssl(_ssl)
	{}
	HostPortOverride()
		: port(-1)
		, ssl(UserAccount::SSL_No)
	{}

	QString host;
	int port;
	UserAccount::SSLFlag ssl;
};
#endif

void PsiAccount::setUserAccount(const UserAccount &_acc)
{
	UserAccount acc = _acc;

#ifdef YAPSI
	// store password, auto-reconnect and stuff
	acc.opt_pass   = true;
	acc.opt_auto   = true;
	acc.opt_reconn = true;

	// TODO FIXME: disable when proper SSL certificates are present
	acc.opt_ignoreSSLWarnings = true;

	QMap<QString, HostPortOverride> hostPortOverride;
//	hostPortOverride["jabberout.com"]          = HostPortOverride("xmpp.jabberout.com",      5222, UserAccount::SSL_Yes);
//	hostPortOverride["jabber.ru"]      = HostPortOverride("jabber.ru",           5222, UserAccount::SSL_Yes);
//	hostPortOverride["yandex-team.ru"] = HostPortOverride("xmpp.yandex-team.ru", 5222, UserAccount::SSL_Yes);
	hostPortOverride["gmail.com"]      = HostPortOverride("talk.google.com",     5223, UserAccount::SSL_Legacy);

	acc.jid = Ya::yaRuAliasing(acc.jid);
	XMPP::Jid ajid(acc.jid);
	if (hostPortOverride.contains(ajid.host())) {
		HostPortOverride hpo = hostPortOverride[ajid.host()];
		acc.opt_compress = true;
		acc.opt_host = true;
		acc.host = hpo.host;
		acc.port = hpo.port;
		acc.ssl = hpo.ssl;
	}
	else {
		// protection from sticking with the wrong 'host' setting related to YaLoginPage::signIn()
		QMapIterator<QString, HostPortOverride> i(hostPortOverride);
		while (i.hasNext()) {
			i.next();
			if (acc.host == i.value().host) {
				acc.opt_host = false;
				break;
			}
		}
	}

	acc.name = acc.jid;
#endif

	bool renamed = false;
	QString oldfname;
	if(d->acc.name != acc.name) {
		renamed = true;
		oldfname = d->pathToProfileEvents();
	}

	d->acc = acc;

	// rename queue file?
	if(renamed) {
		QFileInfo oldfi(oldfname);
		QFileInfo newfi(d->pathToProfileEvents());
		if(oldfi.exists()) {
			QDir dir = oldfi.dir();
			dir.rename(oldfi.fileName(), newfi.fileName());
		}
	}

	if(d->stream) {
		if(d->acc.opt_keepAlive)
			d->stream->setNoopTime(55000);  // prevent NAT timeouts every minute
		else
			d->stream->setNoopTime(0);
	}

	// d->cp->setName(d->acc.name);

	Jid j = acc.jid;
	d->nextJid = j;
	if(!isActive()) {
		d->jid = j;
		d->self.setJid(j);
		d->updateEntry(d->self);
	}
	if(!d->nickFromVCard)
		setNick(j.user());

	QString pgpSecretKeyID = (d->acc.pgpSecretKey.isNull() ? "" : d->acc.pgpSecretKey.keyId());
	d->self.setPublicKeyID(pgpSecretKeyID);
	if(PGPUtil::instance().pgpAvailable()) {
		bool updateStatus = !PGPUtil::instance().equals(d->acc.pgpSecretKey, d->cur_pgpSecretKey) && loggedIn();
		d->cur_pgpSecretKey = d->acc.pgpSecretKey;
		pgpKeyChanged();
		if (updateStatus) {
			d->loginStatus.setXSigned("");
			setStatusDirect(d->loginStatus);
		}
	}

	cpUpdate(d->self);
	updatedAccount();
}

void PsiAccount::deleteQueueFile()
{
	QFileInfo fi(d->pathToProfileEvents());
	if(fi.exists()) {
		QDir dir = fi.dir();
		dir.remove(fi.fileName());
	}
}

const Jid & PsiAccount::jid() const
{
	return d->jid;
}

QString PsiAccount::nameWithJid() const
{
	return (name() + " (" + JIDUtil::toString(jid(),true) + ')');
}

void PsiAccount::autoLogin()
{
	// auto-login ?
	if (d->acc.opt_enabled) {
		bool autoLogin = d->acc.opt_auto;
#ifdef YAPSI
		autoLogin &= !d->acc.pass.isEmpty();
#endif
		if (autoLogin) {
			PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::autoLogin() ... setStatus").arg(LOG_THIS)
			                           .arg(name()));
#ifndef YAPSI
			setStatus(Status(Status::Online, "", d->acc.priority));
#else
			qDebug() << "PsiAccount::autoLogin()" << d->psi->lastLoggedInStatusText();
			setStatus(Status(d->psi->lastLoggedInStatusType(), d->psi->lastLoggedInStatusText(), d->acc.priority));
#endif
		}
	}
}

// logs on with the active account settings
void PsiAccount::login()
{
	PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::login(); doReconnect = %3; isActive = %4").arg(LOG_THIS)
	                           .arg(name())
	                           .arg(doReconnect)
	                           .arg(isActive()));

	if(isActive() && !doReconnect)
		return;

	if((d->acc.ssl == UserAccount::SSL_Yes || d->acc.ssl == UserAccount::SSL_Legacy) && !QCA::isSupported("tls")) {
		QMessageBox::information(0, (d->psi->contactList()->enabledAccounts().count() > 1 ? QString("%1: ").arg(name()) : "") + tr("SSL Error"), tr("Cannot login: SSL is enabled but no SSL/TLS (plugin) support is available."));
		return;
	}

	d->jid = d->nextJid;

	v_isActive = true;
	isDisconnecting = false;
	notifyOnlineOk = false;
	rosterDone = false;
	presenceSent = false;
#ifdef YAPSI
	d->askingForVCard = false;
	d->disableAutoConnect = false;
#endif

	stateChanged();

	bool useHost = false;
	QString host;
	int port = -1;
	if(d->acc.opt_host) {
		useHost = true;
		host = d->acc.host;
		if (host.isEmpty()) {
			host = d->jid.domain();
		}
		port = d->acc.port;
	}

	AdvancedConnector::Proxy p;
	if(d->acc.proxy_index > 0) {
		const ProxyItem &pi = d->psi->proxy()->getItem(d->acc.proxy_index-1);
		if(pi.type == "http") // HTTP Connect
			p.setHttpConnect(pi.settings.host, pi.settings.port);
		else if(pi.type == "socks") // SOCKS
			p.setSocks(pi.settings.host, pi.settings.port);
		else if(pi.type == "poll") { // HTTP Poll
			QUrl u = pi.settings.url;
			if(u.queryItems().isEmpty()) {
				if (useHost)
					u.addQueryItem("server",host + ':' + QString::number(port));
				else
					u.addQueryItem("server",d->jid.host());
			}
			p.setHttpPoll(pi.settings.host, pi.settings.port, u.toString());
			p.setPollInterval(2);
		}

		if(pi.settings.useAuth)
			p.setUserPass(pi.settings.user, pi.settings.pass);
	}

	// stream
	d->conn = new AdvancedConnector;
	if(d->acc.ssl != UserAccount::SSL_No && QCA::isSupported("tls")) {
		d->tls = new QCA::TLS;
		d->tls->setTrustedCertificates(CertUtil::allCertificates());
		d->tlsHandler = new QCATLSHandler(d->tls);
		d->tlsHandler->setXMPPCertCheck(true);
		connect(d->tlsHandler, SIGNAL(tlsHandshaken()), SLOT(tls_handshaken()));
	}
	d->conn->setProxy(p);
	if (useHost) {
		d->conn->setOptHostPort(host, port);
		d->conn->setOptSSL(d->acc.ssl == UserAccount::SSL_Legacy);
	}
	else if (QCA::isSupported("tls")) {
		d->conn->setOptProbe(d->acc.legacy_ssl_probe && d->acc.ssl != UserAccount::SSL_No);
	}

	d->stream = new ClientStream(d->conn, d->tlsHandler);
	d->stream->setRequireMutualAuth(d->acc.req_mutual_auth);
	d->stream->setSSFRange(d->acc.security_level,256);
	d->stream->setAllowPlain(d->acc.allow_plain);
	d->stream->setCompress(d->acc.opt_compress);
	d->stream->setLang(TranslationManager::instance()->currentXMLLanguage());
	if(d->acc.opt_keepAlive)
		d->stream->setNoopTime(55000);  // prevent NAT timeouts every minute
	else
		d->stream->setNoopTime(0);
	connect(d->stream, SIGNAL(connected()), SLOT(cs_connected()));
	connect(d->stream, SIGNAL(securityLayerActivated(int)), SLOT(cs_securityLayerActivated(int)));
	connect(d->stream, SIGNAL(needAuthParams(bool, bool, bool)), SLOT(cs_needAuthParams(bool, bool, bool)));
	connect(d->stream, SIGNAL(authenticated()), SLOT(cs_authenticated()));
	connect(d->stream, SIGNAL(connectionClosed()), SLOT(cs_connectionClosed()), Qt::QueuedConnection);
	connect(d->stream, SIGNAL(delayedCloseFinished()), SLOT(cs_delayedCloseFinished()));
	connect(d->stream, SIGNAL(warning(int)), SLOT(cs_warning(int)));
	connect(d->stream, SIGNAL(error(int)), SLOT(cs_error(int)), Qt::QueuedConnection);

	Jid j = d->jid.withResource((d->acc.opt_automatic_resource ? localHostName() : d->acc.resource ));
	d->client->connectToServer(d->stream, j);
}

// disconnect or stop reconnecting
void PsiAccount::logout(bool fast, const Status &s)
{
	PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::logout(); doReconnect = %3; isActive = %4; fast = %5; loggedIn = %6").arg(LOG_THIS)
	                           .arg(name())
	                           .arg(doReconnect)
	                           .arg(isActive())
	                           .arg(fast)
	                           .arg(loggedIn()));

	if(!isActive())
		return;

	clearCurrentConnectionError();

	// cancel reconnect
	doReconnect = false;

	if(loggedIn()) {
		// Extended Presence
		if (d->options->getOption("options.extended-presence.tune.publish").toBool() && !d->lastTune.isNull())
			publishTune(Tune());

		d->client->removeExtension("ep");
		d->client->removeExtension("pep");

		// send logout status
		d->client->setPresence(s);
	}

	isDisconnecting = true;

	if(!fast)
		simulateRosterOffline();

	v_isActive = false;
	d->loginStatus = Status(Status::Offline);
	stateChanged();

#ifdef YAPSI
	disconnect();
#else
	// Using 100msecs; See note on disconnect()
	QTimer::singleShot(100, this, SLOT(disconnect()));
#endif
}

// skz note: I had to split logout() because server seem to need some time to store status
// before stream is closed (weird, I know)
void PsiAccount::disconnect()
{
	PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::disconnect(); doReconnect = %3; isActive = %4; isDisconnecting = %5").arg(LOG_THIS)
	                           .arg(name())
	                           .arg(doReconnect)
	                           .arg(isActive())
	                           .arg(isDisconnecting));

	if (isDisconnecting) {
		// disconnect
		d->client->close();
		cleanupStream();

		disconnected();
	}
}

// TODO: search through the Psi and replacing most of loggedIn() calls with isAvailable()
// because it's safer
bool PsiAccount::loggedIn() const
{
	return (v_isActive && presenceSent);
}

void PsiAccount::tls_handshaken()
{
	if (!d->tls || d->tls->peerCertificateChain().isEmpty())
		return;
	QCA::Certificate cert = d->tls->peerCertificateChain().primary();
	int r = d->tls->peerIdentityResult();
	if (r == QCA::TLS::Valid && !d->tlsHandler->certMatchesHostname()) r = QCA::TLS::HostMismatch;
	if(r != QCA::TLS::Valid && !d->acc.opt_ignoreSSLWarnings) {
		QCA::Validity validity =  d->tls->peerCertificateValidity();
		QString str = CertUtil::resultToString(r,validity);
		QMessageBox msgBox(QMessageBox::Warning,
			(d->psi->contactList()->enabledAccounts().count() > 1 ? QString("%1: ").arg(name()) : "") + tr("Server Authentication"),
			tr("The %1 certificate failed the authenticity test.").arg(d->jid.host()) + '\n' + tr("Reason: %1.").arg(str));
		QPushButton *detailsButton = msgBox.addButton(tr("&Details..."), QMessageBox::ActionRole);
		QPushButton *continueButton = msgBox.addButton(tr("Co&ntinue"), QMessageBox::AcceptRole);
		QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);
		msgBox.setDefaultButton(detailsButton);
		msgBox.setResult(QDialog::Accepted);

		connect(this, SIGNAL(disconnected()), &msgBox, SLOT(reject()));
		connect(this, SIGNAL(reconnecting()), &msgBox, SLOT(reject()));

		while (msgBox.result() != QDialog::Rejected) {
			msgBox.exec();
			if (msgBox.clickedButton() == detailsButton) {
				msgBox.setResult(QDialog::Accepted);
				SSLCertDlg::showCert(cert, r, validity);
			}
			else if (msgBox.clickedButton() == continueButton) {
				d->tlsHandler->continueAfterHandshake();
				break;
			}
			else if (msgBox.clickedButton() == cancelButton) {
				logout();
				break;
			}
			else {	// msgBox was hidden because connection was closed
				break;
			}
		}
	}
	else
		d->tlsHandler->continueAfterHandshake();
}

void PsiAccount::showCert()
{
	if (!d->tls || !d->tls->isHandshaken()) return;
	QCA::Certificate cert = d->tls->peerCertificateChain().primary();
	int r = d->tls->peerIdentityResult();
	if (r == QCA::TLS::Valid && !d->tlsHandler->certMatchesHostname()) r = QCA::TLS::HostMismatch;
	QCA::Validity validity =  d->tls->peerCertificateValidity();
	SSLCertDlg::showCert(cert, r, validity);
}


void PsiAccount::cs_connected()
{
	// get IP address
	ByteStream *bs = d->conn ? d->conn->stream() : 0;
	if (!bs)
		return;

	if(bs->inherits("BSocket") || bs->inherits("XMPP::BSocket")) {
		d->localAddress = ((BSocket *)bs)->address();
	}
}

void PsiAccount::cs_securityLayerActivated(int layer)
{
	if ((layer == ClientStream::LayerSASL) && (d->stream->saslSSF() <= 1)) {
		 // integrity protected only
	} else {
		d->usingSSL = true;
		stateChanged();
	}
}

void PsiAccount::cs_needAuthParams(bool user, bool pass, bool realm)
{
	if(user) {
		if (d->acc.customAuth && !d->acc.authid.isEmpty())
			d->stream->setUsername(d->acc.authid);
		else
			d->stream->setUsername(d->jid.user());
    }
	else if (d->acc.customAuth && !d->acc.authid.isEmpty())
		qWarning("Custom authentication user not used");

	if(pass)
		d->stream->setPassword(d->acc.pass);

	if (realm) {
		if (d->acc.customAuth && !d->acc.realm.isEmpty()) {
			d->stream->setRealm(d->acc.realm);
		}
		else {
			d->stream->setRealm(d->jid.domain());
		}
	}
	else if (d->acc.customAuth && !d->acc.realm.isEmpty())
		qWarning("Custom authentication realm not used");

	if(d->acc.customAuth)
		d->stream->setAuthzid(d->jid.bare());
	d->stream->continueAfterParams();
}

void PsiAccount::cs_authenticated()
{
	//printf("PsiAccount: [%s] authenticated\n", name().latin1());
	d->conn->changePollInterval(10); // for http poll, slow down after login

	// Update our jid (if necessary)
	if (!d->stream->jid().isEmpty()) {
		d->jid = d->stream->jid().bare();
	}

	QString resource = (d->stream->jid().resource().isEmpty() ? ( d->acc.opt_automatic_resource ? localHostName() : d->acc.resource) : d->stream->jid().resource());

	d->client->start(d->jid.host(), d->jid.user(), d->acc.pass, resource);
	if (!d->stream->old()) {
		JT_Session *j = new JT_Session(d->client->rootTask());
		connect(j,SIGNAL(finished()),SLOT(sessionStart_finished()));
		j->go(true);
	}
	else {
		sessionStarted();
	}

#ifdef YAPSI
	if (!d->acc.opt_pass) {
		d->acc.pass = QString();
	}
#endif
}

void PsiAccount::sessionStart_finished()
{
	JT_Session *j = (JT_Session*)sender();
	if ( j->success() ) {
		sessionStarted();
	}
	else {
		cs_error(-1);
	}
}


void PsiAccount::sessionStarted()
{
	if (d->voiceCaller)
		d->voiceCaller->initialize();

	// flag roster for delete
	UserListIt it(d->userList);
	for(UserListItem *u; (u = it.current()); ++it) {
		if(u->inList())
			u->setFlagForDelete(true);
	}

	// ask for roster
	d->client->rosterRequest();
}

void PsiAccount::cs_connectionClosed()
{
	cs_error(-1);
}

void PsiAccount::cs_delayedCloseFinished()
{
	//printf("PsiAccount: [%s] connection closed\n", name().latin1());
}

void PsiAccount::cs_warning(int w)
{
	bool showNoTlsWarning = w == ClientStream::WarnNoTLS && d->acc.ssl == UserAccount::SSL_Yes;
	bool doCleanupStream = !d->stream || showNoTlsWarning;

	if (doCleanupStream) {
		d->client->close();
		cleanupStream();
		v_isActive = false;
		d->loginStatus = Status(Status::Offline);
		stateChanged();
		disconnected();
	}

	if (showNoTlsWarning) {
		QMessageBox* m = new QMessageBox(QMessageBox::Critical, (d->psi->contactList()->enabledAccounts().count() > 1 ? QString("%1: ").arg(name()) : "") + tr("Server Error"), tr("The server does not support TLS encryption."), QMessageBox::Ok, 0, Qt::WDestructiveClose);
		m->setModal(true);
		m->show();
	}
	else if (!doCleanupStream) {
		Q_ASSERT(d->stream);
		d->stream->continueAfterWarning();
	}
}

void PsiAccount::getErrorInfo(int err, AdvancedConnector *conn, Stream *stream, QCATLSHandler *tlsHandler, QString *_str, bool *_reconn, bool *_disableAutoConnect)
{
	QString str;
	bool reconn = false;
	bool disableAutoConnect = false;

	if(err == -1) {
		str = tr("Disconnected");
		reconn = true;
	}
	else if(err == XMPP::ClientStream::ErrParse) {
		str = tr("XML Parsing Error");
		reconn = true;
	}
	else if(err == XMPP::ClientStream::ErrProtocol) {
		str = tr("XMPP Protocol Error");
		reconn = true;
	}
	else if(err == XMPP::ClientStream::ErrStream) {
		int x;
		QString s;
		reconn = true;
		if (stream) // Stream can apparently be gone if you disconnect in time
			x = stream->errorCondition();
		else {
			x = XMPP::Stream::GenericStreamError;
#ifndef YAPSI
			reconn = false;
#endif
		}

		if(x == XMPP::Stream::GenericStreamError)
			s = tr("Generic stream error");
		else if(x == XMPP::ClientStream::Conflict) {
			s = tr("Conflict (remote login replacing this one)");
			reconn = false;
			disableAutoConnect = true;
		}
		else if(x == XMPP::ClientStream::ConnectionTimeout)
			s = tr("Timed out from inactivity");
		else if(x == XMPP::ClientStream::InternalServerError)
			s = tr("Internal server error");
		else if(x == XMPP::ClientStream::InvalidXml)
			s = tr("Invalid XML");
		else if(x == XMPP::ClientStream::PolicyViolation) {
			s = tr("Policy violation");
			reconn = false;
		}
		else if(x == XMPP::ClientStream::ResourceConstraint) {
			s = tr("Server out of resources");
			reconn = false;
		}
		else if(x == XMPP::ClientStream::SystemShutdown)
			s = tr("Server is shutting down");
		str = tr("XMPP Stream Error: %1").arg(s);
	}
	else if(err == XMPP::ClientStream::ErrConnection) {
		int x = conn->errorCode();
		QString s;
		reconn = true;
		if(x == XMPP::AdvancedConnector::ErrConnectionRefused)
			s = tr("Unable to connect to server");
		else if(x == XMPP::AdvancedConnector::ErrHostNotFound)
			s = tr("Host not found");
		else if(x == XMPP::AdvancedConnector::ErrProxyConnect)
			s = tr("Error connecting to proxy");
		else if(x == XMPP::AdvancedConnector::ErrProxyNeg)
			s = tr("Error during proxy negotiation");
		else if(x == XMPP::AdvancedConnector::ErrProxyAuth) {
			s = tr("Proxy authentication failed");
			reconn = false;
		}
		else if(x == XMPP::AdvancedConnector::ErrStream)
			s = tr("Socket/stream error");
		str = tr("Connection Error: %1").arg(s);
	}
	else if(err == XMPP::ClientStream::ErrNeg) {
		int x = stream->errorCondition();
		QString s;
		if(x == XMPP::ClientStream::HostGone)
			s = tr("Host no longer hosted");
		else if(x == XMPP::ClientStream::HostUnknown)
			s = tr("Host unknown");
		else if(x == XMPP::ClientStream::RemoteConnectionFailed) {
			s = tr("A required remote connection failed");
			reconn = true;
		}
		else if(x == XMPP::ClientStream::SeeOtherHost)
			s = tr("See other host: %1").arg(stream->errorText());
		else if(x == XMPP::ClientStream::UnsupportedVersion)
			s = tr("Server does not support proper XMPP version");
		str = tr("Stream Negotiation Error: %1").arg(s);
	}
	else if(err == XMPP::ClientStream::ErrTLS) {
		int x = stream->errorCondition();
		QString s;
		if(x == XMPP::ClientStream::TLSStart)
			s = tr("Server rejected STARTTLS");
		else if(x == XMPP::ClientStream::TLSFail) {
			int t = tlsHandler->tlsError();
			if(t == QCA::TLS::ErrorHandshake)
				s = tr("TLS handshake error");
			else
				s = tr("Broken security layer (TLS)");
		}
		str = s;
	}
	else if(err == XMPP::ClientStream::ErrAuth) {
		int x = stream->errorCondition();
		QString s;
// #ifdef YAPSI
// 		reconn = true;
// #endif
		if(x == XMPP::ClientStream::GenericAuthError)
			s = tr("Unable to login");
		else if(x == XMPP::ClientStream::NoMech)
			s = tr("No appropriate mechanism available for given security settings (e.g. SASL library too weak, or plaintext authentication not enabled)");
		else if(x == XMPP::ClientStream::BadProto)
			s = tr("Bad server response");
		else if(x == XMPP::ClientStream::BadServ)
			s = tr("Server failed mutual authentication");
		else if(x == XMPP::ClientStream::EncryptionRequired)
			s = tr("Encryption required for chosen SASL mechanism");
		else if(x == XMPP::ClientStream::InvalidAuthzid)
			s = tr("Invalid account information");
		else if(x == XMPP::ClientStream::InvalidMech)
			s = tr("Invalid SASL mechanism");
		else if(x == XMPP::ClientStream::InvalidRealm)
			s = tr("Invalid realm");
		else if(x == XMPP::ClientStream::MechTooWeak)
			s = tr("SASL mechanism too weak for this account");
		else if(x == XMPP::ClientStream::NotAuthorized)
			s = tr("Not authorized");
		else if(x == XMPP::ClientStream::TemporaryAuthFailure)
			s = tr("Temporary auth failure");
		str = tr("Authentication error: %1").arg(s);
	}
	else if(err == XMPP::ClientStream::ErrSecurityLayer)
		str = tr("Broken security layer (SASL)");
	else
		str = tr("None");
	//printf("str[%s], reconn=%d\n", str.latin1(), reconn);
	*_str = str;
	*_reconn = reconn;
	*_disableAutoConnect = disableAutoConnect;
}

void PsiAccount::cs_error(int err)
{
	QString str;
	bool reconn;

	if (!isActive()) return; // all cleaned up already

	getErrorInfo(err, d->conn, d->stream, d->tlsHandler, &str, &reconn, &d->disableAutoConnect);
	d->currentConnectionError = str;
	d->currentConnectionErrorCondition = -1;
	if(err == XMPP::ClientStream::ErrAuth && d && d->stream) {
		d->currentConnectionErrorCondition = d->stream->errorCondition();
	}

	d->client->close();
	cleanupStream();

#ifdef YAPSI
	// we could disable an account while 'emit connectionError()'
	QString bakError = d->currentConnectionError;
	int bakErrorCond = d->currentConnectionErrorCondition;
#endif

	emit connectionError(d->currentConnectionError);
	//printf("Error: [%s]\n", str.latin1());

#ifdef YAPSI
	d->currentConnectionError = bakError;
	d->currentConnectionErrorCondition = bakErrorCond;
#endif

	isDisconnecting = true;

	PsiLogger::instance()->log(QString("%1 PsiAccount(%6)::cs_error(%2); loggedIn = %3; d->acc.opt_reconn = %4; reconn = %5;").arg(LOG_THIS)
	                           .arg(err)
	                           .arg(loggedIn())
	                           .arg(d->acc.opt_reconn)
	                           .arg(reconn)
	                           .arg(name()));

	if ( loggedIn() ) { // FIXME: is this condition okay?
#ifndef YAPSI
		simulateRosterOffline();
#else
		QList<PsiContact*> onlineContacts;
		if (reconn) {
			foreach(PsiContact* contact, d->contacts) {
				if (contact->isOnline())
					contact->setReconnectingState(true);
			}
		}
		else {
			simulateRosterOffline();
		}

		foreach(TabDlg* tabDlg, d->tabManager->tabSets()) {
			for (uint j = 0; j < tabDlg->tabCount(); ++j) {
				ChatDlg* chatDlg = dynamic_cast<ChatDlg*>(tabDlg->getTab(j));
				if (chatDlg)
					chatDlg->invalidateTab();
			}
		}
#endif
	}

	presenceSent = false; // this stops the idle detector?? (FIXME)
#ifdef YAPSI
	d->askingForVCard = false;
#endif

	// Auto-Reconnect?
	if(d->acc.opt_reconn && reconn) {
		int delay = 5000; // reconnect in 5 seconds
#ifdef YAPSI
		delay = 60000;
#endif
		doReconnect = true;
		stateChanged();
		QTimer::singleShot(delay, this, SLOT(reconnect()));
		return;
	}

	v_isActive = false;
	d->loginStatus = Status(Status::Offline);
	stateChanged();
	disconnected();

#ifndef YAPSI
	bool printAccountName = d->psi->contactList()->enabledAccounts().count() > 1;
	QMessageBox* m = new QMessageBox(QMessageBox::Critical,
	                                 (printAccountName ? QString("%1: ").arg(name()) : "") + tr("Server Error"),
	                                 tr("There was an error communicating with the server.\nDetails: %1").arg(str),
	                                 QMessageBox::Ok, 0, Qt::WDestructiveClose);
	m->setModal(true);
	m->show();
#endif
}

void PsiAccount::clearCurrentConnectionError()
{
	d->currentConnectionError = QString();
	d->currentConnectionErrorCondition = -1;
	emit connectionError(d->currentConnectionError);
}

QString PsiAccount::currentConnectionError() const
{
	return d->currentConnectionError;
}

int PsiAccount::currentConnectionErrorCondition() const
{
	return d->currentConnectionErrorCondition;
}

void PsiAccount::client_rosterRequestFinished(bool success, int, const QString &)
{
	PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::client_rosterRequestFinished(); success = %3;").arg(LOG_THIS)
	                           .arg(name())
	                           .arg(success));

	if(success) {
		//printf("PsiAccount: [%s] roster retrieved ok.  %d entries.\n", name().latin1(), d->client->roster().count());

		// delete flagged items
		UserListIt it(d->userList);
		for(UserListItem *u; (u = it.current());) {
			if(u->flagForDelete()) {
				//QMessageBox::information(0, "blah", QString("deleting: [%1]").arg(u->jid().full()));

				d->eventQueue->clear(u->jid());
				updateReadNext(u->jid());

				d->removeEntry(u->jid());
				d->userList.removeRef(u);
			}
			else
				++it;
		}

#ifdef YAPSI
		// requestAvatarsForAllContacts();
#endif
	}
	else {
		//printf("PsiAccount: [%s] error retrieving roster: [%d, %s]\n", name().latin1(), code, str.latin1());
	}

	rosterDone = true;

	// Get stored options
	// FIXME: Should be an account-specific option
	//if (PsiOptions::instance()->getOption("options.options-storage.load").toBool())
	//	PsiOptions::instance()->load(d->client);

	setStatusDirect(d->loginStatus, d->loginWithPriority);

	emit rosterRequestFinished();
}

void PsiAccount::requestAvatarsForAllContacts()
{
	foreach(PsiContact* contact, d->contacts) {
		const VCard* vcard = VCardFactory::instance()->vcard(contact->jid());
		if (!vcard) {
			VCardFactory::instance()->getVCard(contact->jid(), client()->rootTask(), 0, 0, true);
		}
	}
}

void PsiAccount::resolveContactName()
{
	JT_VCard *j = (JT_VCard *)sender();
	if ( j->success() ) {
		QString nick = j->vcard().nickName();
		if ( !nick.isEmpty() ) {
			actionRename( j->jid(), nick );
		}
	}
}

void PsiAccount::serverFeaturesChanged()
{
	setPEPAvailable(d->serverInfoManager->hasPEP());
}

void PsiAccount::setPEPAvailable(bool b)
{
	if (d->pepAvailable == b)
		return;

	d->pepAvailable = b;

#ifdef PEP
	// Publish support
	if (b && !d->client->extensions().contains("ep")) {
		QStringList pepNodes;
		pepNodes += "http://jabber.org/protocol/mood";
		pepNodes += "http://jabber.org/protocol/tune";
		pepNodes += "http://jabber.org/protocol/physloc";
		pepNodes += "http://jabber.org/protocol/geoloc";
		pepNodes += "http://www.xmpp.org/extensions/xep-0084.html#ns-data";
		pepNodes += "http://www.xmpp.org/extensions/xep-0084.html#ns-metadata";
		d->client->addExtension("ep",Features(pepNodes));
		setStatusActual(d->loginStatus);
	}
	else if (!b && d->client->extensions().contains("ep")) {
		d->client->removeExtension("ep");
		setStatusActual(d->loginStatus);
	}

	// Publish current tune information
	if (b && d->psi->tuneController() && d->options->getOption("options.extended-presence.tune.publish").toBool()) {
		Tune current = d->psi->tuneController()->currentTune();
		if (!current.isNull())
			publishTune(current);
	}
#endif
}

void PsiAccount::getBookmarks_success(const QList<URLBookmark>&, const QList<ConferenceBookmark>& conferences)
{
	QObject::disconnect(d->bookmarkManager,SIGNAL(getBookmarks_success(const QList<URLBookmark>&, const QList<ConferenceBookmark>&)),this,SLOT(getBookmarks_success(const QList<URLBookmark>&, const QList<ConferenceBookmark>&)));

#ifdef GROUPCHAT
	if (!GCMainDlg::mucEnabled())
		return;

	foreach(ConferenceBookmark c, conferences) {
		if (!findDialog<GCMainDlg*>(Jid(c.jid().userHost())) && c.autoJoin()) {
			QString nick = c.nick();
			if (nick.isEmpty())
				nick = d->jid.node();

			MUCJoinDlg *w = new MUCJoinDlg(psi(), this);
			w->le_host->setText(c.jid().domain());
			w->le_room->setText(c.jid().node());
			w->le_nick->setText(nick);
			w->le_pass->setText(c.password());
			w->show();
			w->doJoin();
		}
	}
#endif
}

void PsiAccount::incomingHttpAuthRequest(const PsiHttpAuthRequest &req)
{
	HttpAuthEvent *e = new HttpAuthEvent(req, this);
	handleEvent(e, IncomingStanza);
}

void PsiAccount::client_rosterItemAdded(const RosterItem &r)
{
	if ( r.isPush() && r.name().isEmpty() && option.autoResolveNicksOnAdd ) {
		// automatically resolve nickname from vCard, if newly added item doesn't have any
		VCardFactory::instance()->getVCard(r.jid(), d->client->rootTask(), this, SLOT(resolveContactName()));
	}
}

void PsiAccount::client_rosterItemUpdated(const RosterItem &r)
{
	// see if the item added is already in our local list
	UserListItem *u = d->userList.find(r.jid());
	if(u) {
		u->setFlagForDelete(false);
#ifdef YAPSI
		QString yaMood = u->yaMood();
		bool yaMoodSet = u->yaMoodSet();
#endif
		u->setRosterItem(r);
#ifdef YAPSI
		if (!yaMood.isEmpty() && r.yaMood().isEmpty() && yaMoodSet) {
			u->setYaMood(yaMood, yaMoodSet);
		}
#endif
	}
	else {
		// we don't have it at all, so add it
		u = new UserListItem;
		u->setRosterItem(r);
		u->setAvatarFactory(avatarFactory());
		d->userList.append(u);
	}
	u->setInList(true);

	d->updateEntry(*u);
}

void PsiAccount::client_rosterItemRemoved(const RosterItem &r)
{
	UserListItem *u = d->userList.find(r.jid());
	if(!u)
		return;

	u->setInList(false);
	simulateContactOffline(u);

	// if the item has messages queued, then move them to 'not in list'
	if(d->eventQueue->count(r.jid()) > 0) {
		u->setInList(false);
		d->updateEntry(*u);
	}
	// else remove them for good!
	else {
		d->removeEntry(u->jid());
		d->userList.removeRef(u);
	}
}

void PsiAccount::tryVerify(UserListItem *u, UserResource *ur)
{
	if(PGPUtil::instance().pgpAvailable())
		verifyStatus(u->jid().withResource(ur->name()), ur->status());
}

void PsiAccount::incomingVoiceCall(const Jid& j)
{
//	VoiceCallDlg* vc = new VoiceCallDlg(j,voiceCaller());
//	vc->show();
//	vc->incoming();

/* ivan: disabling old CallDlg:
    CallDlg *c = ensureCallDlg(j);
    processCalls(j);
    bringToFront(c);
    c->incoming();
    */

    CallDialog::instance()->init(j, this, voiceCaller());
    CallDialog::instance()->incoming();
}

void PsiAccount::client_resourceAvailable(const Jid &j, const Resource &r)
{
	// Notification
	enum PopupType {
		PopupOnline = 0,
		PopupStatusChange = 1
	};
	PopupType popupType = PopupOnline;

	if ( j.user().isEmpty() )
		new BlockTransportPopup(d->blockTransportPopupList, j);

	bool doSound = false;
	bool doPopup = false;
	foreach(UserListItem* u, findRelevant(j)) {
		bool doAnim = false;
		bool local = false;
		if(u->isSelf() && r.name() == d->client->resource())
			local = true;

		// add/update the resource
		QString oldStatus, oldKey;
		UserResource* rp;
		UserResourceList::Iterator rit = u->userResourceList().find(j.resource());
		bool found = (rit == u->userResourceList().end()) ? false: true;
		if(!found) {
			popupType = PopupOnline;

			UserResource ur(r);
			//ur.setSecurityEnabled(true);
			if(local)
				ur.setClient(ApplicationInfo::name(),ApplicationInfo::version(),SystemInfo::instance()->os());

			u->userResourceList().append(ur);
			rp = &u->userResourceList().last();

			if(notifyOnlineOk && !local) {
				doAnim = true;
				if (!u->isHidden()) {
					doSound = true;
					doPopup = true;
				}
			}
		}
		else {
			if ( !doPopup )
				popupType = PopupStatusChange;

			oldStatus = (*rit).status().status();
			oldKey = (*rit).status().keyID();
			rp = &(*rit);

			(*rit).setResource(r);

			if (!local && !u->isHidden())
				doPopup = true;
		}

		rp->setPGPVerifyStatus(-1);
		if(!rp->status().xsigned().isEmpty())
			tryVerify(u, rp);

		u->setPresenceError("");
		cpUpdate(*u, r.name(), true);

		if(doAnim && option.rosterAnim)
			d->animateNick(u->jid());
	}

	if(doSound)
		playSound(eOnline);

#if !defined(Q_WS_MAC) || !defined(HAVE_GROWL)
	// Do the popup test earlier (to avoid needless JID lookups)
	if ((popupType == PopupOnline && option.ppOnline) || (popupType == PopupStatusChange && option.ppStatus))
#endif
	if(notifyOnlineOk && doPopup && !d->blockTransportPopupList->find(j, popupType == PopupOnline) && !d->noPopup(IncomingStanza)) {
		QString name;
		UserListItem *u = findFirstRelevant(j);

		PsiPopup::PopupType pt = PsiPopup::AlertNone;
		if ( popupType == PopupOnline )
			pt = PsiPopup::AlertOnline;
		else if ( popupType == PopupStatusChange )
			pt = PsiPopup::AlertStatusChange;

		if ((popupType == PopupOnline && option.ppOnline) || (popupType == PopupStatusChange && option.ppStatus)) {
			PsiPopup *popup = new PsiPopup(pt, this);
			popup->setData(j, r, u);
		}
#if defined(Q_WS_MAC) && defined(HAVE_GROWL)
		PsiGrowlNotifier::instance()->popup(this, pt, j, r, u);
#endif
	}
	else if ( !notifyOnlineOk )
		d->userCounter++;

	// Update entity capabilities.
	// This has to happen after the userlist item has been created.
	if (!r.status().capsNode().isEmpty()) {
		capsManager()->updateCaps(j,r.status().capsNode(),r.status().capsVersion(),r.status().capsExt());

		// Update the client version
		foreach(UserListItem* u, findRelevant(j)) {
			UserResourceList::Iterator rit = u->userResourceList().find(j.resource());
			if (rit != u->userResourceList().end()) {
				(*rit).setClient(capsManager()->clientName(j),capsManager()->clientVersion(j),"");
				cpUpdate(*u,(*rit).name());
			}
		}
	}

#ifdef YAPSI
	if (d->client && d->client->jid().compare(j, true) && isYaAccount()) {
		// need to be careful here, as we also get notifications of status
		// changes initiated by ourselves
		if (d->mood != r.status().status() && r.status().status() != d->loginStatus.status()) {
			d->mood = r.status().status();
			emit moodChanged();
		}
	}
#endif
}

void PsiAccount::client_resourceUnavailable(const Jid &j, const Resource &r)
{
	bool doSound = false;
	bool doPopup = false;

	if ( j.user().isEmpty() )
		new BlockTransportPopup(d->blockTransportPopupList, j);

	foreach(UserListItem* u, findRelevant(j)) {
		bool local = false;
		if(u->isSelf() && r.name() == d->client->resource())
			local = true;

		// remove resource
		UserResourceList::Iterator rit = u->userResourceList().find(j.resource());
		bool found = (rit == u->userResourceList().end()) ? false: true;
		if(found) {
			u->setLastUnavailableStatus(r.status());
			u->userResourceList().remove(rit);

			if(!u->isAvailable())
				u->setLastAvailable(QDateTime::currentDateTime());

			if(!u->isAvailable() || u->isSelf()) {
				// don't sound for our own resource
				if(!isDisconnecting && !local && !u->isHidden()) {
					doSound = true;
					doPopup = true;
				}
			}
		}

		u->setPresenceError("");
		cpUpdate(*u, r.name(), true);
	}
	if(doSound)
		playSound(eOffline);

#if !defined(Q_WS_MAC) || !defined(HAVE_GROWL)
	// Do the popup test earlier (to avoid needless JID lookups)
	if (option.ppOffline)
#endif
	if(doPopup && !d->blockTransportPopupList->find(j) && !d->noPopup(IncomingStanza)) {
		QString name;
		UserListItem *u = findFirstRelevant(j);

		if (option.ppOffline) {
			PsiPopup *popup = new PsiPopup(PsiPopup::AlertOffline, this);
			popup->setData(j, r, u);
		}
#if defined(Q_WS_MAC) && defined(HAVE_GROWL)
		PsiGrowlNotifier::instance()->popup(this, PsiPopup::AlertOffline, j, r, u);
#endif
	}
}

void PsiAccount::client_presenceError(const Jid &j, int, const QString &str)
{
	foreach(UserListItem *u, findRelevant(j)) {
		simulateContactOffline(u);
		u->setPresenceError(str);
		cpUpdate(*u, j.resource(), false);
	}
}

void PsiAccount::client_messageReceived(const Message &m)
{
	//check if it's a server message without a from, and set the from appropriately
	Message _m(m);
	if (_m.from().isEmpty())
	{
		_m.setFrom(jid().domain());
	}

	// if the sender is already in the queue, then queue this message also
	foreach(Message* mi, d->messageQueue) {
		if(mi->from().compare(_m.from())) {
			Message *m = new Message(_m);
			d->messageQueue.append(m);
			return;
		}
	}

	// check to see if message was forwarded from another resource
	if (jid().compare(_m.from(),false)) {
		AddressList oFrom = _m.findAddresses(Address::OriginalFrom);
		AddressList oTo = _m.findAddresses(Address::OriginalTo);
		if ((oFrom.count() > 0) && (oTo.count() > 0)) {
			// might want to store current values in MessageEvent object
			// replace out the from and to addresses with the original addresses
			_m.setFrom(oFrom[0].jid());
			_m.setTo(oTo[0].jid());
		}
	}

	// encrypted message?
	if(PGPUtil::instance().pgpAvailable() && !_m.xencrypted().isEmpty()) {
		Message *m = new Message(_m);
		d->messageQueue.append(m);
		processMessageQueue();
		return;
	}

	processIncomingMessage(_m);
}

/**
 * Handles the passed Message \param m. Also message's type could be modified
 * here, if certain options are set.
 *
 * TODO: Generalize option.incomingAs and EventDlg::messagingEnabled()
 * processing in one common function that could be applied to the messages
 * loaded from the event queue, and settings were changed prior to that.
 */
void PsiAccount::processIncomingMessage(const Message &_m)
{
	// skip empty messages, but not if the message contains a data form
	if(_m.body().isEmpty() && _m.urlList().isEmpty() && _m.invite().isEmpty() && !_m.containsEvents() && _m.chatState() == StateNone && _m.subject().isEmpty() && _m.rosterExchangeItems().isEmpty() && _m.mucInvites().isEmpty() &&  _m.getForm().fields().empty())
		return;

#ifdef YAPSI
	if (!_m.lastMailNotify().isNull()) {
		emit lastMailNotify(_m);
		emit lastMailNotify(_m.lastMailNotify());
		return;
	}
#endif

	// skip headlines?
	if(_m.type() == "headline" && option.ignoreHeadline)
		return;

	if(_m.type() == "groupchat") {
#ifdef GROUPCHAT
		if (!GCMainDlg::mucEnabled())
			return;

		GCMainDlg *w = findDialog<GCMainDlg*>(Jid(_m.from().userHost()));
		if(w)
			w->message(_m);
		return;
#endif
	}

	// only toggle if not an invite or body is not empty
	if(_m.invite().isEmpty() && !_m.body().isEmpty() && _m.mucInvites().isEmpty() && _m.rosterExchangeItems().isEmpty())
		toggleSecurity(_m.from(), _m.wasEncrypted());

	UserListItem *u = findFirstRelevant(_m.from());
	if(u) {
		if(_m.type() == "chat") u->setLastMessageType(1);
		else u->setLastMessageType(0);
	}

	Message m = _m;

	// smartchat: try to match up the incoming event to an existing chat
	// (prior to 0.9, m.from() always contained a resource)
	Jid j;
	ChatDlg *c;
	QList<UserListItem*> ul = findRelevant(m.from());

	// ignore events from non-roster JIDs?
	if (ul.isEmpty() && option.ignoreNonRoster)
	{
		if (option.excludeGroupChatsFromIgnore)
		{
#ifdef GROUPCHAT
			if (!GCMainDlg::mucEnabled())
				return;

			GCMainDlg *w = findDialog<GCMainDlg*>(Jid(_m.from().userHost()));
			if(!w)
			{
				return;
			}
#endif
		}
		else
		{
			return;
		}
	}

	if(ul.isEmpty())
		j = m.from().userHost();
	else
		j = ul.first()->jid();

	// Roster item exchange
	if (!_m.rosterExchangeItems().isEmpty()) {
		RosterExchangeEvent* ree = new RosterExchangeEvent(j,_m.rosterExchangeItems(), _m.body(), this);
		handleEvent(ree, IncomingStanza);
		return;
	}

	c = findChatDialog(j);
	if(!c)
		c = findChatDialog(m.from().full());

	if(m.type() == "error") {
		Stanza::Error err = m.error();
		QPair<QString, QString> desc = err.description();
		QString msg = desc.first + ".\n" + desc.second;
		if (!err.text.isEmpty())
			msg += "\n" + err.text;

		m.setBody(msg + "\n------\n" + m.body());
	}

	// change the type?
	if (!EventDlg::messagingEnabled()) {
		m.setType("chat");
	}
	else if (m.type() != "headline" && m.invite().isEmpty() && m.mucInvites().isEmpty()) {
		if (option.incomingAs == 1)
			m.setType("");
		else if (option.incomingAs == 2)
			m.setType("chat");
		else if (option.incomingAs == 3) {
			if (c != NULL && !c->isHidden())
				m.setType("chat");
			else
				m.setType("");
		}
	}

	// urls or subject on a chat message?  convert back to regular message
	//if(m.type() == "chat" && (!m.urlList().isEmpty() || !m.subject().isEmpty()))
	//	m.setType("");

	//We must not respond to recepit request automatically - KIS
	/*if(m.messageReceipt() == ReceiptRequest && !m.id().isEmpty()) {
		Message tm(m.from());
		tm.setId(m.id());
		tm.setMessageReceipt(ReceiptReceived);
		dj_sendMessage(tm, false);
	}*/

	MessageEvent *me = new MessageEvent(m, this);
	me->setOriginLocal(false);
	handleEvent(me, IncomingStanza);
}

void PsiAccount::client_subscription(const Jid &j, const QString &str, const QString& nick)
{
	// if they remove our subscription, then we lost presence
	if(str == "unsubscribed") {
		UserListItem *u = d->userList.find(j);
		if(u)
			simulateContactOffline(u);
	}

	AuthEvent *ae = new AuthEvent(j, str, this);
	ae->setTimeStamp(QDateTime::currentDateTime());
	ae->setNick(nick);
	handleEvent(ae, IncomingStanza);
}

void PsiAccount::client_debugText(const QString &)
{
	//printf("%s", str.latin1());
	//fflush(stdout);
}

#ifdef GOOGLE_FT
void PsiAccount::incomingGoogleFileTransfer(GoogleFileTransfer* ft)
{
	if (QMessageBox::information(0, tr("Incoming file"), QString(tr("Do you want to accept %1 (%2 kb) from %3?")).arg(ft->fileName()).arg((float) ft->fileSize() / 1000).arg(ft->peer().full()), QMessageBox::Yes,QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) == QMessageBox::Yes) {
		GoogleFileTransferProgressDialog* d = new GoogleFileTransferProgressDialog(ft);
		d->show();
		ft->accept();
	}
	else
		ft->reject();
}
#endif

void PsiAccount::client_incomingFileTransfer()
{
#ifdef FILETRANSFER
	FileTransfer *ft = d->client->fileTransferManager()->takeIncoming();
	if(!ft)
		return;

	/*printf("psiaccount: incoming file transfer:\n");
	printf("  From: [%s]\n", ft->peer().full().latin1());
	printf("  Name: [%s]\n", ft->fileName().latin1());
	printf("  Size: %d bytes\n", ft->fileSize());*/

	FileEvent *fe = new FileEvent(ft->peer().full(), ft, this);
	fe->setTimeStamp(QDateTime::currentDateTime());
	handleEvent(fe, IncomingStanza);
#endif
}

#ifdef YAPSI
/**
 * Returns true if this is an account on jabberout.com.
 */
bool PsiAccount::isYaAccount() const
{
	return Ya::isYaJid(jid());
}

bool PsiAccount::disableAutoConnect() const
{
	return d->disableAutoConnect;
}
#endif

void PsiAccount::pingServer()
{
	Q_ASSERT(enabled());
	if (!isAvailable())
		return;

	d->pingServer();
}

void PsiAccount::reconnect()
{
	PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::reconnect(); doReconnect = %3;").arg(LOG_THIS)
	                           .arg(name())
	                           .arg(doReconnect));

	if(doReconnect && !isConnected()) {
		//printf("PsiAccount: [%s] reconnecting...\n", name().latin1());
		emit reconnecting();
		v_isActive = false;
		doReconnect = false;
		login();
	}
}

Status PsiAccount::status() const
{
	return d->loginStatus;
}

Status PsiAccount::loggedOutStatus() const
{
	return Status(Status::Offline, "Logged out", 0);
}

void PsiAccount::setStatus(const Status &_s,  bool withPriority)
{
	PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::setStatus(%3, %4) --> resetting doReconnect").arg(LOG_THIS)
	                           .arg(name())
	                           .arg(_s.type())
	                           .arg(withPriority));

	Status s = _s;
	if (!withPriority)
		s.setPriority(d->acc.priority);

#ifdef YAPSI
	// protection against sending presence as result of
	// YaPsiServer::setDND() call even if our current status
	// is the same as requested
	if (s.type() == d->lastManualStatus().type() &&
	    s.status() == d->lastManualStatus().status() &&
	    s.priority() == d->lastManualStatus().priority() &&
	    isActive())
	{
		return;
	}
#endif

	d->setManualStatus(s);

	// Block all transports' contacts' status change popups from popping
	{
		Roster::ConstIterator rit = d->acc.roster.begin();
		for ( ; rit != d->acc.roster.end(); ++rit) {
			const RosterItem &i = *rit;
			if ( i.jid().user().isEmpty() /*&& i.jid().resource() == "registered"*/ ) // it is very likely then, that it's transport
				new BlockTransportPopup(d->blockTransportPopupList, i.jid());
		}
	}

	d->loginStatus = s;
	d->loginWithPriority = withPriority;

	if(s.isAvailable()) {
		// if client is not active then attempt to login
		if(!isActive()) {
			Jid j = d->jid;

			if(!j.isValid()) {
#ifndef YAPSI
				QMessageBox::information(0, CAP(tr("Error")), tr("Unable to login.  Ensure your account information is filled out."));
				modify();
#endif
				return;
			}

#ifndef YAPSI
			if(!d->acc.opt_pass) {
				bool ok = false;
				QString text = QInputDialog::getText(
					tr("Need Password"),
					( d->psi->contactList()->enabledAccounts().count() > 1 ?
					  tr("Please enter the password for %1:").arg(JIDUtil::toString(j,true))
					  : tr("Please enter your password:") ),
					QLineEdit::Password, QString::null, &ok, 0);
				if(ok && !text.isEmpty())
					d->acc.pass = text;
				else
					return;
			}
#endif

			login();
			setStatusDirect(s, withPriority);
			emit moodChanged();
		}
		// change status
		else {
			if(rosterDone)
				setStatusDirect(s, withPriority);

			if(s.isInvisible()) {//&&Pass invis to transports KEVIN
				//this is a nasty hack to let the transports know we're invisible, since they get an offline packet when we go invisible
				foreach(UserListItem* u, d->userList) {
					if(u->isTransport()) {
						JT_Presence *j = new JT_Presence(d->client->rootTask());
						j->pres(u->jid(), s);
						j->go(true);
					}
				}
			}
		}
	}
	else {
		if(isActive())
			logout(false, s);
	}
}

void PsiAccount::setStatusDirect(const Status &_s, bool withPriority)
{
	Status s = _s;
	if (!withPriority)
		s.setPriority(d->acc.priority);

	//printf("setting status to [%s]\n", s.status().latin1());

	// using pgp?
	if(!d->cur_pgpSecretKey.isNull()) {
		d->loginStatus = s;

		// sign presence
		trySignPresence();
	}
	else {
		/*if(d->psi->pgp() && !d->cur_pgpSecretKeyID.isEmpty())
			s.setKeyID(d->cur_pgpSecretKeyID);
		else
			s.setKeyID("");*/

		// send presence normally
		setStatusActual(s);
	}
}

void PsiAccount::setStatusActual(const Status &_s)
{
	PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::setStatusActual(); isConnected = %3; isActive = %4; presenceSent = %5; askingForVCard = %6; rosterDone = %7").arg(LOG_THIS)
	                           .arg(name())
	                           .arg(isConnected())
	                           .arg(isActive())
	                           .arg(presenceSent)
	                           .arg(d->askingForVCard)
	                           .arg(rosterDone));

	Status s = _s;

	// Add entity capabilities information
	if (capsManager()->isEnabled()) {
		s.setCapsNode(d->client->capsNode());
		s.setCapsVersion(d->client->capsVersion());
		s.setCapsExt(d->client->capsExt());
	}

	if (!presenceSent) {
		simulateRosterOffline();
	}

        // Add vcard photo hash if available
	if(!d->photoHash.isEmpty()) {
		s.setPhotoHash(d->photoHash);
	}

	// Set the status
	d->loginStatus = s;
#ifdef INITIAL_PRESENCE_AFTER_VCARD
	// HACKHACKHACK
	if (presenceSent) {
		d->client->setPresence(s);
		stateChanged();
	}

	if (!presenceSent && !d->askingForVCard && rosterDone) {
		PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::setStatusActual(): getVcard").arg(LOG_THIS)
		                           .arg(name()));
		d->askingForVCard = true;
		VCardFactory::instance()->getVCard(d->jid, d->client->rootTask(), this, SLOT(slotCheckVCard()));
	}
#else
	d->client->setPresence(s);
	if(presenceSent) {
		stateChanged();
	}
	else {
		clearCurrentConnectionError();

		presenceSent = true;
		stateChanged();
		sentInitialPresence();
	}
#endif
}

void PsiAccount::sentInitialPresence()
{
	QTimer::singleShot(15000, this, SLOT(enableNotifyOnline()));

	// Get the bookmarks
	if (PsiOptions::instance()->getOption("options.muc.bookmarks.auto-join").toBool()) {
		connect(d->bookmarkManager, SIGNAL(getBookmarks_success(const QList<URLBookmark>&, const QList<ConferenceBookmark>&)), SLOT(getBookmarks_success(const QList<URLBookmark>&, const QList<ConferenceBookmark>&)));
		d->bookmarkManager->getBookmarks();
	}

#ifndef INITIAL_PRESENCE_AFTER_VCARD
#ifndef YAPSI
	// Get the vcard
	const VCard *vcard = VCardFactory::instance()->vcard(d->jid);
	if (option.autoVCardOnLogin || !vcard || vcard->isEmpty() || vcard->nickName().isEmpty())
#endif
		VCardFactory::instance()->getVCard(d->jid, d->client->rootTask(), this, SLOT(slotCheckVCard()));
#ifndef YAPSI
	else {
		d->nickFromVCard = true;
		setNick(vcard->nickName());
	}
#endif
#endif
}

void PsiAccount::capsChanged(const Jid& j)
{
	if (!loggedIn())
		return;

	QString name = capsManager()->clientName(j);
	QString version = (name.isEmpty() ? QString() : capsManager()->clientVersion(j));

	foreach(UserListItem *u, findRelevant(j)) {
		UserResourceList::Iterator rit = u->userResourceList().find(j.resource());
		bool found = (rit == u->userResourceList().end()) ? false: true;
		if(!found)
			continue;
		(*rit).setClient(name,version,"");
		cpUpdate(*u);
	}
}

void PsiAccount::tuneStopped()
{
	if (loggedIn() && d->options->getOption("options.extended-presence.tune.publish").toBool()) {
		publishTune(Tune());
	}
}

void PsiAccount::tunePlaying(const Tune& tune)
{
	if (loggedIn() && d->options->getOption("options.extended-presence.tune.publish").toBool()) {
		publishTune(tune);
	}
}

void PsiAccount::publishTune(const Tune& tune)
{
	QDomDocument* doc = d->client->rootTask()->doc();
	QDomElement t = doc->createElement("tune");
	t.setAttribute("xmlns", "http://jabber.org/protocol/tune");
	if (!tune.artist().isEmpty())
		t.appendChild(textTag(doc, "artist", tune.artist()));
	if (!tune.name().isEmpty())
		t.appendChild(textTag(doc, "title", tune.name()));
	if (!tune.album().isEmpty())
		t.appendChild(textTag(doc, "source", tune.album()));
	if (!tune.track().isEmpty())
		t.appendChild(textTag(doc, "track", tune.track()));
	if (tune.time() != 0)
		t.appendChild(textTag(doc, "length", QString::number(tune.time())));

	d->lastTune = tune;
	d->pepManager->publish("http://jabber.org/protocol/tune",PubSubItem("current",t));
}

void PsiAccount::secondsIdle(int seconds)
{
#define getOption(A, B) PsiOptions::instance()->getOption(A).to##B ()
	int minutes = seconds / 60;
	int required;

	if (seconds < d->lastNumberOfSecondsIdle_) {
		if (getOption("options.general.status.auto.restore", Bool)) {
			d->resetAutoAway();
		}
		return;
	}
	d->lastNumberOfSecondsIdle_ = seconds;

	if ((required = getOption("options.general.status.auto.offline", Int)) > 0 && minutes >= required) {
		d->setAutoAway(Private::AutoAway_Offline);
	} else if ((required = getOption("options.general.status.auto.dnd", Int)) > 0 && minutes >= required) {
		d->setAutoAway(Private::AutoAway_DND);
	} else if ((required = getOption("options.general.status.auto.away", Int)) > 0 && minutes >= required) {
		d->setAutoAway(Private::AutoAway_Away);
	}

// 	if(option.use_asOffline && option.asOffline > 0 && minutes >= option.asOffline)
// 		d->setAutoAway(Private::AutoAway_Offline);
// 	else if(option.use_asXa && option.asXa > 0 && minutes >= option.asXa)
// 		d->setAutoAway(Private::AutoAway_XA);
// 	else if(option.use_asAway && option.asAway > 0 && minutes >= option.asAway)
// 		d->setAutoAway(Private::AutoAway_Away);
// 	else
// 		d->setAutoAway(Private::AutoAway_None);
#undef getOption
}

void PsiAccount::playSound(int onevent)
{
	if (onevent < 0) {
		return;
	}

	QString str = option.onevent[onevent];
	if(str.isEmpty())
		return;

	XMPP::Status::Type s = XMPP::Status::Offline;
	if (loggedIn())
		s = d->lastManualStatus().type();

	if (s == XMPP::Status::DND)
		return;

	// no away sounds?
	if(option.noAwaySound && (s == STATUS_AWAY || s == STATUS_XA))
		return;

#ifdef YAPSI
	// we don't want sound notifications when contacts become online/offline
	// and since we auto-auth contacts, system events' sound notifications may
	// be unwanted too
	if (onevent == eOffline ||
	    onevent == eOnline  ||
	    onevent == eSystem)
	{
		return;
	}
#endif

	d->psi->playSound(str);
}

QString PsiAccount::localHostName()
{
	QString hostname = QHostInfo::localHostName();
	int i = hostname.indexOf('.');
	if (i != -1)
		return hostname.left(hostname.indexOf('.'));
	else
		return hostname;
}

bool PsiAccount::validRosterExchangeItem(const RosterExchangeItem& item)
{
	if (item.action() == RosterExchangeItem::Add) {
		return (d->client->roster().find(item.jid(),true) == d->client->roster().end());
	}
	else if (item.action() == RosterExchangeItem::Delete) {
		LiveRoster::ConstIterator i = d->client->roster().find(item.jid(),true);
		if (i == d->client->roster().end())
			return false;

		foreach(QString group, item.groups()) {
			if (!(*i).groups().contains(group))
				return false;
		}
		return true;
	}
	else if (item.action() == RosterExchangeItem::Modify) {
		// TODO
		return false;
	}
	return false;
}

ChatDlg* PsiAccount::findChatDialog(const Jid& jid) const
{
	return findDialog<ChatDlg*>(jid,
#ifndef YAPSI
	                            true
#else
	                            false
#endif
	                           );
}

QWidget* PsiAccount::findDialog(const QMetaObject& mo, const Jid& jid, bool compareResource) const
{
	return d->findDialog(mo, jid, compareResource);
}

void PsiAccount::findDialogs(const QMetaObject& mo, const Jid& jid, bool compareResource, QList<void*>* list) const
{
	d->findDialogs(mo, jid, compareResource, list);
}

void PsiAccount::findDialogs(const QMetaObject& mo, QList<void*>* list) const
{
	d->findDialogs(mo, list);
}

void PsiAccount::dialogRegister(QWidget *w, const Jid &j)
{
	d->dialogRegister(w, j);
}

void PsiAccount::dialogUnregister(QWidget *w)
{
	d->dialogUnregister(w);
}

void PsiAccount::deleteAllDialogs()
{
	delete d->xmlConsole;
	d->deleteDialogList();
}

bool PsiAccount::checkConnected(QWidget *par)
{
	if (!isAvailable()) {
		QMessageBox::information(par, CAP(tr("Error")),
		                         tr("You must be connected to the server in order to do this."));
		return false;
	}

	return true;
}

void PsiAccount::modify()
{
#ifndef YAPSI
	d->doModify();
#else
	AccountModifyDlg *w = findDialog<AccountModifyDlg*>();
	if (w) {
		bringToFront(w);
	}
	else {
		w = new AccountModifyDlg(this, 0);
		w->show();
	}
#endif
}

void PsiAccount::changeVCard()
{
	actionInfo(d->jid,false);
}

void PsiAccount::changePW()
{
	if (!checkConnected()) {
		return;
	}

	ChangePasswordDlg *w = findDialog<ChangePasswordDlg*>();
	if (w) {
		bringToFront(w);
	}
	else {
		w = new ChangePasswordDlg(this);
		w->show();
	}
}

void PsiAccount::showXmlConsole()
{
	bringToFront(d->xmlConsole);
}

void PsiAccount::openAddUserDlg()
{
	if(!checkConnected())
		return;

	AddUserDlg *w = findDialog<AddUserDlg*>();
	if(w)
		bringToFront(w);
	else {
		QStringList gl, services, names;
		UserListIt it(d->userList);
		for(UserListItem *u; (u = it.current()); ++it) {
			if(u->isTransport()) {
				services += u->jid().full();
				names += JIDUtil::nickOrJid(u->name(), u->jid().full());
			}
			foreach(QString group, u->groups()) {
				if(!gl.contains(group))
					gl.append(group);
			}
		}

		w = new AddUserDlg(services, names, gl, this);
		connect(w, SIGNAL(add(const XMPP::Jid &, const QString &, const QStringList &, bool)), SLOT(dj_add(const XMPP::Jid &, const QString &, const QStringList &, bool)));
		w->show();
	}
}

void PsiAccount::doDisco()
{
	actionDisco(d->jid.host(), "");
}

void PsiAccount::actionDisco(const Jid &j, const QString &node)
{
	DiscoDlg *w = new DiscoDlg(this, j, node);
	connect(w, SIGNAL(featureActivated(QString, Jid, QString)), SLOT(featureActivated(QString, Jid, QString)));
	w->show();
}

void PsiAccount::featureActivated(QString feature, Jid jid, QString node)
{
	Features f(feature);

	if ( f.canRegister() )
		actionRegister(jid);
	else if ( f.canSearch() )
		actionSearch(jid);
	else if ( f.canGroupchat() )
		actionJoin(jid);
	else if ( f.canCommand() )
		actionExecuteCommand(jid, node);
	else if ( f.canDisco() )
		actionDisco(jid, node);
	else if ( f.isGateway() )
		; // TODO
	else if ( f.haveVCard() )
		actionInfo(jid);
	else if ( f.id() == Features::FID_Add ) {
		QStringList sl;
		dj_add(jid, QString::null, sl, true);
	}
}

void PsiAccount::actionJoin(const Jid &j, const QString& password)
{
	MUCJoinDlg *w = new MUCJoinDlg(psi(), this);

	w->le_host->setText ( j.host() );
	w->le_room->setText ( j.user() );
	w->le_pass->setText (password);

	w->show();
}

void PsiAccount::stateChanged()
{
	if(loggedIn())
		d->setState(makeSTATUS(status()));
	else {
		if(isActive()) {
			d->setState(-1);
		}
		else {
			d->setState(STATUS_OFFLINE);
		}
	}

	emit updatedActivity();
}

void PsiAccount::simulateContactOffline(const XMPP::Jid& contact)
{
	foreach(UserListItem* u, findRelevant(contact)) {
		simulateContactOffline(u);
	}
}

void PsiAccount::simulateContactOffline(UserListItem *u)
{
	UserResourceList rl = u->userResourceList();
	u->setPresenceError("");
	if(!rl.isEmpty()) {
		for(UserResourceList::ConstIterator rit = rl.begin(); rit != rl.end(); ++rit) {
			const UserResource &r = *rit;
			Jid j = u->jid();
			if(u->jid().resource().isEmpty())
				j.setResource(r.name());
			client_resourceUnavailable(j, r);
		}
	}
	u->setLastUnavailableStatus(makeStatus(STATUS_OFFLINE,""));
	u->setLastAvailable(QDateTime());
	cpUpdate(*u);
}

void PsiAccount::simulateRosterOffline()
{
	emit beginBulkContactUpdate();

	UserListIt it(d->userList);
	for(UserListItem *u; (u = it.current()); ++it)
		simulateContactOffline(u);

	// self
	{
		UserListItem *u = &d->self;
		UserResourceList rl = u->userResourceList();
		for(UserResourceList::ConstIterator rit = rl.begin(); rit != rl.end(); ++rit) {
			Jid j = u->jid();
			if(u->jid().resource().isEmpty())
				j.setResource((*rit).name());
			u->setPresenceError("");
			client_resourceUnavailable(j, *rit);
		}
	}

	while (!d->gcbank.isEmpty())
		delete d->gcbank.takeFirst();

	emit endBulkContactUpdate();
}

bool PsiAccount::notifyOnline() const
{
	return notifyOnlineOk;
}

void PsiAccount::enableNotifyOnline()
{
	if ( d->userCounter > 1 ) {
		QTimer::singleShot(15000, this, SLOT(enableNotifyOnline()));
		d->userCounter = 0;
	}
	else
		notifyOnlineOk = true;
}

void PsiAccount::itemRetracted(const Jid& j, const QString& n, const PubSubRetraction& item)
{
	Q_UNUSED(item);
	// User Tune
	if (n == "http://jabber.org/protocol/tune") {
		// Parse tune
		foreach(UserListItem* u, findRelevant(j)) {
			// FIXME: try to find the right resource using JEP-33 'replyto'
			//UserResourceList::Iterator rit = u->userResourceList().find(<resource>);
			//bool found = (rit == u->userResourceList().end()) ? false: true;
			//if(found)
			//	(*rit).setTune(tune);
			u->setTune(QString());
			cpUpdate(*u);
		}
	}
	else if (n == "http://jabber.org/protocol/mood") {
		foreach(UserListItem* u, findRelevant(j)) {
			u->setMood(Mood());
			cpUpdate(*u);
		}
	}
	else if (n == "http://jabber.org/protocol/geoloc") {
		// FIXME: try to find the right resource using JEP-33 'replyto'
		// see tune case above
		foreach(UserListItem* u, findRelevant(j)) {
			u->setGeoLocation(GeoLocation());
			cpUpdate(*u);
		}
	}
	else if (n == "http://jabber.org/protocol/physloc") {
		// FIXME: try to find the right resource using JEP-33 'replyto'
		// see tune case above
		foreach(UserListItem* u, findRelevant(j)) {
			u->setPhysicalLocation(PhysicalLocation());
			cpUpdate(*u);
		}
	}
}

void PsiAccount::itemPublished(const Jid& j, const QString& n, const PubSubItem& item)
{
	// User Tune
	if (n == "http://jabber.org/protocol/tune") {
		// Parse tune
		QDomElement element = item.payload();
		QDomElement e;
		QString tune;
		bool found;

		e = findSubTag(element, "artist", &found);
		if (found)
			tune += e.text() + " - ";

		e = findSubTag(element, "title", &found);
		if (found)
			tune += e.text();

		foreach(UserListItem* u, findRelevant(j)) {
			// FIXME: try to find the right resource using JEP-33 'replyto'
			//UserResourceList::Iterator rit = u->userResourceList().find(<resource>);
			//bool found = (rit == u->userResourceList().end()) ? false: true;
			//if(found)
			//	(*rit).setTune(tune);
			u->setTune(tune);
			cpUpdate(*u);
		}
	}
	else if (n == "http://jabber.org/protocol/mood") {
		Mood mood(item.payload());
		foreach(UserListItem* u, findRelevant(j)) {
			u->setMood(mood);
			cpUpdate(*u);
		}
	}
	else if (n == "http://jabber.org/protocol/geoloc") {
		// FIXME: try to find the right resource using JEP-33 'replyto'
		// see tune case above
		GeoLocation geoloc(item.payload());
		foreach(UserListItem* u, findRelevant(j)) {
			u->setGeoLocation(geoloc);
			cpUpdate(*u);
		}
	}
	else if (n == "http://jabber.org/protocol/physloc") {
		// FIXME: try to find the right resource using JEP-33 'replyto'
		// see tune case above
		PhysicalLocation physloc(item.payload());
		foreach(UserListItem* u, findRelevant(j)) {
			u->setPhysicalLocation(physloc);
			cpUpdate(*u);
		}
	}
}

QList<UserListItem*> PsiAccount::findRelevant(const Jid &j) const
{
	QList<UserListItem*> list;

	// self?
	if(j.compare(d->self.jid(), false))
		list.append(&d->self);
	else {
		foreach(UserListItem* u, d->userList) {
			if(!u->jid().compare(j, false))
				continue;

			if(!u->jid().resource().isEmpty()) {
				if(u->jid().resource() != j.resource())
					continue;
			} else {
				// skip status changes from muc participants
				// if the MUC somehow got into userList.
				if (!j.resource().isEmpty() && d->groupchats.contains(j.bare())) continue;
			}
			list.append(u);
		}
	}

	return list;
}

UserListItem *PsiAccount::findFirstRelevant(const Jid &j) const
{
	QList<UserListItem*> list = findRelevant(j);
	if(list.isEmpty())
		return 0;
	else
		return list.first();
}

PsiContact* PsiAccount::selfContact() const
{
	return d->selfContact;
}

const QList<PsiContact*>& PsiAccount::contactList() const
{
	return d->contacts;
}

PsiContact* PsiAccount::findContact(const Jid& jid) const
{
	return d->findContact(jid);
}

UserListItem *PsiAccount::find(const Jid &j) const
{
	UserListItem *u;
	if(j.compare(d->self.jid()))
		u = &d->self;
	else
		u = d->userList.find(j);

	return u;
}

void PsiAccount::cpUpdate(const UserListItem &u, const QString &rname, bool fromPresence)
{
	PsiEvent *e = d->eventQueue->peek(u.jid());

	if(e) {
		d->setAlert(u.jid(), PsiIconset::instance()->event2icon(e));
	}
	else {
		d->clearAlert(u.jid());
	}

	d->updateEntry(u);
	emit updateContact(u);
	Jid j = u.jid();
	if(!rname.isEmpty())
		j.setResource(rname);
	emit updateContact(j);
	emit updateContact(j, fromPresence);
	d->psi->updateContactGlobal(this, j);
}

EventDlg *PsiAccount::ensureEventDlg(const Jid &j)
{
	EventDlg *w = findDialog<EventDlg*>(j);
	if (!w && EventDlg::messagingEnabled())
		w = new EventDlg(j, this, true);

	if (w) {
		connect(w, SIGNAL(aReadNext(const Jid &)), SLOT(processReadNext(const Jid &)));
		connect(w, SIGNAL(aChat(const Jid &)), SLOT(actionOpenChat(const Jid&)));
//		connect(w, SIGNAL(aCall(const Jid &)), SLOT(actionMakeCall(const Jid&)));
		connect(w, SIGNAL(aReply(const Jid &, const QString &, const QString &, const QString &)), SLOT(dj_composeMessage(const Jid &, const QString &, const QString &, const QString &)));
		connect(w, SIGNAL(aAuth(const Jid &)), SLOT(dj_addAuth(const Jid &)));
		connect(w, SIGNAL(aDeny(const Jid &)), SLOT(dj_deny(const Jid &)));
		connect(w, SIGNAL(aHttpConfirm(const PsiHttpAuthRequest &)), SLOT(dj_confirmHttpAuth(const PsiHttpAuthRequest &)));
		connect(w, SIGNAL(aHttpDeny(const PsiHttpAuthRequest &)), SLOT(dj_denyHttpAuth(const PsiHttpAuthRequest &)));
		connect(w, SIGNAL(aRosterExchange(const RosterExchangeItems &)), SLOT(dj_rosterExchange(const RosterExchangeItems &)));
		connect(d->psi, SIGNAL(emitOptionsUpdate()), w, SLOT(optionsUpdate()));
		connect(this, SIGNAL(updateContact(const Jid &)), w, SLOT(updateContact(const Jid &)));
		connect(w, SIGNAL(aFormSubmit(const XData&, const QString&, const Jid&)), SLOT(dj_formSubmit(const XData&, const QString&, const Jid&)));
		connect(w, SIGNAL(aFormCancel(const XData&, const QString&, const Jid&)), SLOT(dj_formCancel(const XData&, const QString&, const Jid&)));
	}

	return w;
}

ChatDlg *PsiAccount::ensureChatDlg(const Jid &j)
{
	ChatDlg *c = findChatDialog(j);
#ifdef YAPSI
	if (c)
		c->setJid(j);
#endif
	if(!c) {
		// create the chatbox
		c = ChatDlg::create(j, this, d->tabManager);
		connect(c, SIGNAL(aSend(const Message &)), SLOT(dj_sendMessage(const Message &)));
		connect(c, SIGNAL(messagesRead(const Jid &)), SLOT(chatMessagesRead(const Jid &)));
//		connect(c, SIGNAL(aCall(const Jid &)), SLOT(actionMakeCall(const Jid &)));
		connect(c, SIGNAL(aInfo(const Jid &)), SLOT(actionInfo(const Jid &)));
		connect(c, SIGNAL(aHistory(const Jid &)), SLOT(actionHistory(const Jid &)));
		connect(c, SIGNAL(aFile(const Jid &)), SLOT(actionSendFile(const Jid &)));
		connect(d->psi, SIGNAL(emitOptionsUpdate()), c, SLOT(optionsUpdate()));
		connect(this, SIGNAL(updateContact(const Jid &, bool)), c, SLOT(updateContact(const Jid &, bool)));

#ifdef YAPSI
		processChatsHelper(j, false);
#endif
	}
	else {
		// on X11, do a special reparent to open on the right desktop
#ifdef Q_WS_X11
		/* KIS added an exception for tabs here. We do *not* want chats flying
		 * randomlyi, it pulls them out of tabsets. So instead, we move the
		 * tabset instead. It's just as filthy, unfortunately, but it's the
		 * only way */
		//TODO: This doesn't work as expected atm, it doesn't seem to reparent the tabset
		QWidget *window=c;
		if ( option.useTabs )
			window = d->tabManager->getManagingTabs(c);
		if(window && window->isHidden()) {
			const QPixmap *pp = c->icon();
			QPixmap p;
			if(pp)
				p = *pp;
#ifdef __GNUC__
#warning "Removed reparenting call from qwextend"
#endif
			//reparent_good(window, 0, false);
			if(!p.isNull())
				c->setIcon(p);
		}
#endif
	}

	Q_ASSERT(c);
	return c;
}

/*
 * ivan: disabling old CallDlg
 * CallDlg *PsiAccount::ensureCallDlg(const Jid &j)
{
	CallDlg *c = findDialog<CallDlg*>(j);

//	CallDlg *c = (CallDlg *)dialogFind("CallDlg", j);
	if(!c) {
		// create the chatbox
		c = new CallDlg(j, this, voiceCaller());
		CallDialog::instance()->init(j, this, voiceCaller());
	}
	else {
		// on X11, do a special reparent to open on the right desktop
#ifdef Q_WS_X11
		/ KIS added an exception for tabs here. We do *not* want chats flying
		 * randomlyi, it pulls them out of tabsets. So instead, we move the
		 * tabset instead. It's just as filthy, unfortunately, but it's the
		 * only way /
		//TODO: This doesn't work as expected atm, it doesn't seem to reparent the tabset
		QWidget *window = c;
		if(window && window->isHidden()) {
			const QPixmap *pp = c->icon();
			QPixmap p;
			if(pp)
				p = *pp;
			//reparent_good(window, 0, false);
			if(!p.isNull())
				c->setIcon(p);
		}
        c->call();
#endif
	}
	return c;
}
*/

void PsiAccount::changeStatus(int x)
{
	PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::changeStatus(%3)").arg(LOG_THIS)
	                           .arg(name())
	                           .arg(x));
#ifdef YAPSI
	setStatus(Status(static_cast<XMPP::Status::Type>(x), d->psi->currentStatusMessage()));
#else
	if(x == STATUS_OFFLINE && !option.askOffline) {
		setStatus(loggedOutStatus());
	}
	else {
		if(x == STATUS_ONLINE && !option.askOnline) {
			setStatus(Status());
		}
		else if(x == STATUS_INVISIBLE){
			Status s("","",0,true);
			s.setIsInvisible(true);
			setStatus(s);
		}
		else {
			StatusSetDlg *w = new StatusSetDlg(this, makeStatus(x, ""));
			connect(w, SIGNAL(set(const XMPP::Status &, bool)), SLOT(setStatus(const XMPP::Status &, bool)));
			w->show();
		}
	}
#endif
}

/*
void PsiAccount::actionVoice(const Jid &j)
{
	Q_ASSERT(voiceCaller() != NULL);

	Jid jid;
	if (j.resource().isEmpty()) {
		bool found = false;
		UserListItem *u = find(j);
		if (u) {
			const UserResourceList &rl = u->userResourceList();
			for (UserResourceList::ConstIterator it = rl.begin(); it != rl.end() && !found; ++it) {
				if (capsManager()->features(j.withResource((*it).name())).canVoice()) {
					jid = j.withResource((*it).name());
					found = true;
				}
			}
		}

		if (!found)
			return;
	}
	else {
		jid = j;
	}

	VoiceCallDlg* vc = new VoiceCallDlg(jid,voiceCaller());
	vc->show();
	vc->call();
}
*/
void PsiAccount::sendFiles(const Jid& j, const QStringList& l, bool direct)
{
#ifdef FILETRANSFER
	Jid j2 = j;
	if(j.resource().isEmpty()) {
		UserListItem *u = find(j);
		if(u && u->isAvailable())
			j2.setResource((*u->userResourceList().priority()).name());
	}

	// Create a dialog for each file in the list. Once the xfer dialog itself
	// supports multiple files, only the 'else' branch needs to stay.
	if (!l.isEmpty()) {
		for (QStringList::ConstIterator f = l.begin(); f != l.end(); ++f ) {
			QStringList fl(*f);
			FileRequestDlg *w = new FileRequestDlg(j2, d->psi, this, fl, direct);
			w->show();
		}
	}
	else {
		FileRequestDlg *w = new FileRequestDlg(j2, d->psi, this, l, direct);
		w->show();
	}
#else
	Q_UNUSED(j);
	Q_UNUSED(l);
	Q_UNUSED(direct);
#endif
}

void PsiAccount::actionSendFile(const Jid &j)
{
	QStringList l;
	sendFiles(j,l);
}

void PsiAccount::actionSendFiles(const Jid &j, const QStringList& l)
{
	sendFiles(j, l);
}

void PsiAccount::actionExecuteCommand(const Jid& j, const QString& node)
{
	Jid j2 = j;
	if(j.resource().isEmpty()) {
		UserListItem *u = find(j);
		if(u && u->isAvailable())
			j2.setResource((*u->userResourceList().priority()).name());
	}

	actionExecuteCommandSpecific(j2, node);
}

void PsiAccount::actionExecuteCommandSpecific(const Jid& j, const QString& node)
{
	if (node.isEmpty()) {
		AHCommandDlg *w = new AHCommandDlg(this,j);
		w->show();
	}
	else {
		AHCommandDlg::executeCommand(d->client,j,node);
	}
}

void PsiAccount::actionSetMood()
{
	MoodDlg *w = new MoodDlg(this);
	w->show();
}

void PsiAccount::actionSetAvatar()
{
	while(1) {
		if(option.lastPath.isEmpty())
			option.lastPath = QDir::homeDirPath();
		QString str = QFileDialog::getOpenFileName(0,tr("Choose a file"),option.lastPath, tr("Images (*.png *.xpm *.jpg *.PNG *.XPM *.JPG)"));
		if(!str.isEmpty()) {
			QFileInfo fi(str);
			if(!fi.exists()) {
				QMessageBox::critical(0, tr("Error"), tr("The file specified does not exist."));
				continue;
			}
			option.lastPath = fi.dirPath();
			avatarFactory()->setSelfAvatar(str);
		}
		break;
	}
}

void PsiAccount::actionUnsetAvatar()
{
	avatarFactory()->setSelfAvatar("");
}

void PsiAccount::actionDefault(const Jid &j)
{
	UserListItem *u = find(j);
	if(!u)
		return;

#ifndef YAPSI
	if(d->eventQueue->count(u->jid()) > 0)
		openNextEvent(*u, UserAction);
	else {
#endif
		if(option.defaultAction == 0)
			actionSendMessage(u->jid());
		else
			actionOpenChat(u->jid());
#ifndef YAPSI
	}
#endif
}

void PsiAccount::actionRecvEvent(const Jid &j)
{
	UserListItem *u = find(j);
	if(!u)
		return;

	openNextEvent(*u, UserAction);
}

void PsiAccount::actionRecvRosterExchange(const Jid& j, const RosterExchangeItems& items)
{
	handleEvent(new RosterExchangeEvent(j,items,"", this), IncomingStanza);
}

void PsiAccount::actionSendMessage(const Jid &j)
{
	EventDlg *w = d->psi->createEventDlg(j.full(), this);
	if (!w)
		return;
	w->show();
}

void PsiAccount::actionSendMessage(const QList<XMPP::Jid> &j)
{
	QString str;
	bool first = true;
	for(QList<Jid>::ConstIterator it = j.begin(); it != j.end(); ++it) {
		if(!first)
			str += ", ";
		first = false;

		str += (*it).full();
	}

	EventDlg *w = d->psi->createEventDlg(str, this);
	if (!w)
		return;
	w->show();
}

void PsiAccount::actionSendUrl(const Jid &j)
{
	EventDlg *w = d->psi->createEventDlg(j.full(), this);
	if (!w)
		return;
	w->setUrlOnShow();
	w->show();
}

void PsiAccount::actionRemove(const Jid &j)
{
	avatarFactory()->removeManualAvatar(j);
	dj_remove(j);
}

void PsiAccount::actionRename(const Jid &j, const QString &name)
{
	dj_rename(j, name);
}

void PsiAccount::actionGroupRename(const QString &oldname, const QString &newname)
{
	QList<UserListItem*> nu;
	foreach(UserListItem* u, d->userList) {
		if(u->inGroup(oldname)) {
			u->removeGroup(oldname);
			u->addGroup(newname);
			cpUpdate(*u);
			if(u->inList())
				nu.append(u);
		}
	}

	if(!nu.isEmpty()) {
		JT_Roster *r = new JT_Roster(d->client->rootTask());
		foreach(UserListItem* u, nu) {
			r->set(u->jid(), u->name(), u->groups());
		}

		r->go(true);
	}
}

void PsiAccount::actionHistory(const Jid &j)
{
	HistoryDlg *w = findDialog<HistoryDlg*>(j);
	if(w)
		bringToFront(w);
	else {
		w = new HistoryDlg(j, this);
		connect(w, SIGNAL(openEvent(PsiEvent *)), SLOT(actionHistoryBox(PsiEvent *)));
		w->show();
	}
}

void PsiAccount::actionHistoryBox(PsiEvent *e)
{
	if (!EventDlg::messagingEnabled())
		return;

	EventDlg *w = new EventDlg(e->from(), this, false);
	connect(w, SIGNAL(aChat(const Jid &)), SLOT(actionOpenChat(const Jid&)));
	connect(w, SIGNAL(aReply(const Jid &, const QString &, const QString &, const QString &)), SLOT(dj_composeMessage(const Jid &, const QString &, const QString &, const QString &)));
	connect(w, SIGNAL(aAuth(const Jid &)), SLOT(dj_addAuth(const Jid &)));
	connect(w, SIGNAL(aDeny(const Jid &)), SLOT(dj_deny(const Jid &)));
	connect(w, SIGNAL(aRosterExchange(const RosterExchangeItems &)), SLOT(dj_rosterExchange(const RosterExchangeItems &)));
	connect(d->psi, SIGNAL(emitOptionsUpdate()), w, SLOT(optionsUpdate()));
	connect(this, SIGNAL(updateContact(const Jid &)), w, SLOT(updateContact(const Jid &)));
	w->updateEvent(e);
	w->show();
}

void PsiAccount::actionOpenChat(const Jid &j)
{
	UserListItem *u = find(j);
	if(!u)
		return;

	// if 'j' is bare, we might want to switch to a specific resource
	QString res;
	if(j.resource().isEmpty()) {
		// first, are there any queued chats?
		/*PsiEvent *e = d->eventQueue->peekFirstChat(j, false);
		if(e) {
			res = e->from().resource();
			// if we have a bare chat, change to 'res'
			ChatDlg *c = findChatDialog(j);
			if(c)
				c->setJid(j.withResource(res));
		}
		// else, is there a priority chat window available?
		else*/ if(u->isAvailable()) {
			QString pr = (*u->userResourceList().priority()).name();
			if(!pr.isEmpty() && findChatDialog(j.withResource(pr)))
				res = pr;
		}
		else {
			QStringList list = hiddenChats(j);
			if(!list.isEmpty())
				res = list.first();
		}
	}

	if(!res.isEmpty())
		openChat(j.withResource(res), UserAction);
	else
		openChat(j, UserAction);
}

void PsiAccount::actionOpenSavedChat(const Jid& j)
{
	openChat(j, FromXml);
}

void PsiAccount::actionOpenChatSpecific(const Jid &j)
{
	openChat(j, UserAction);
}

void PsiAccount::actionMakeCall(const Jid &j)
{
    qDebug("PsiAccount::actionMakeCall");
	UserListItem *u = find(j.bare());
	if(!u) {
		qDebug( tr("PsiAccount::actionMakeCall, u %1 not found").arg( j.full() ) );
		return;
	}

    qDebug( tr("jid %1").arg(j.full()) );

	// if 'j' is bare, we might want to switch to a specific resource
	QString res;
	if(j.resource().isEmpty()) {
        const UserResourceList &rl = u->userResourceList();
        if(!rl.isEmpty()) {
			UserResourceList::ConstIterator it = rl.priority();
            const UserResource &r = *it;
            res = r.name();
		}
	}

    qDebug( tr("resource").arg(res) );

	if(!res.isEmpty())
		openCall(j.withResource(res));
	else
		openCall(j);
}

void PsiAccount::actionPhoneCall()
{
    qDebug("PsiAccount::actionPhoneCall");

    Jid j = Jid(userAccount().dtmf_resource);

    /*
     * ivan: disabling old CallDlg
     * CallDlg *c = ensureCallDlg( j );
	processCalls( j );
	bringToFront(c);*/
    //c->phoneCall();
}


#ifdef WHITEBOARDING
void PsiAccount::actionOpenWhiteboard(const Jid &j)
{
	UserListItem *u = find(j);
	if(!u)
		return;

	// if 'j' is bare, we might want to switch to a specific resource
	QString res;
	if(j.resource().isEmpty()) {
		if(u->isAvailable()) {
			QString pr = (*u->userResourceList().priority()).name();
			if(!pr.isEmpty())
				res = pr;
		}
	}

	if(!res.isEmpty())
	{
		actionOpenWhiteboardSpecific(j.withResource(res));
	}
	else
	{
		actionOpenWhiteboardSpecific(j);
	}
}

/*! \brief Opens a whiteboard to \a target.
 *  \a ownJid and \a groupChat should be specified in the case of a group chat session.
 */

void PsiAccount::actionOpenWhiteboardSpecific(const Jid &target, Jid ownJid, bool groupChat)
{
	if(ownJid.isEmpty())
		ownJid = jid();
	d->wbManager->openWhiteboard(target, ownJid, groupChat);
}
#endif

void PsiAccount::actionAgentSetStatus(const Jid &j, Status &s)
{
	if ( j.user().isEmpty() ) // add all transport popups to block list
		new BlockTransportPopup(d->blockTransportPopupList, j);

	JT_Presence *p = new JT_Presence(d->client->rootTask());
	p->pres(j, s);
	p->go(true);
}

void PsiAccount::actionInfo(const Jid &_j, bool showStatusInfo)
{
	bool useCache = true;
	Jid j;
	if(findGCContact(_j)) {
		useCache = false;
		j = _j;
	}
	else {
		j = _j.userHost();
	}

	InfoDlg *w = findDialog<InfoDlg*>(j);
	if(w) {
		w->updateStatus();
		w->setStatusVisibility(showStatusInfo);
		bringToFront(w);
	}
	else {
		const VCard *vcard = VCardFactory::instance()->vcard(j);

		VCard tmp;
		if ( vcard )
			tmp = *vcard;
		w = new InfoDlg(j.compare(d->jid) ? InfoDlg::Self : InfoDlg::Contact, j, tmp, this, 0, useCache);

		w->setStatusVisibility(showStatusInfo);
		w->show();

		// automatically retrieve info if it doesn't exist
		if(!vcard && loggedIn())
			w->doRefresh();
	}
}

void PsiAccount::actionAuth(const Jid &j)
{
	dj_auth(j);
}

void PsiAccount::actionAuthRequest(const Jid &j)
{
	dj_authReq(j);
}

void PsiAccount::actionAuthRemove(const Jid &j)
{
	dj_deny(j);
}

void PsiAccount::actionAdd(const Jid &j)
{
	dj_addAuth(j);
}

void PsiAccount::actionGroupAdd(const Jid &j, const QString &g)
{
	UserListItem *u = d->userList.find(j);
	if(!u)
		return;

	if(!u->addGroup(g))
		return;
	cpUpdate(*u);

	JT_Roster *r = new JT_Roster(d->client->rootTask());
	r->set(u->jid(), u->name(), u->groups());
	r->go(true);
}

void PsiAccount::actionGroupRemove(const Jid &j, const QString &g)
{
	UserListItem *u = d->userList.find(j);
	if(!u)
		return;

	if(!u->removeGroup(g))
		return;
	cpUpdate(*u);

	JT_Roster *r = new JT_Roster(d->client->rootTask());
	r->set(u->jid(), u->name(), u->groups());
	r->go(true);
}

void PsiAccount::actionGroupsSet(const Jid &j, const QStringList &g)
{
	UserListItem *u = d->userList.find(j);
	if(!u)
		return;

	QStringList g1 = g;
	g1.sort();
	QStringList g2 = u->groups();
	g2.sort();
	if (g1 == g2)
		return;

	u->setGroups(g);
	cpUpdate(*u);

	JT_Roster *r = new JT_Roster(d->client->rootTask());
	r->set(u->jid(), u->name(), u->groups());
	r->go(true);
}

void PsiAccount::actionRegister(const Jid &j)
{
	if(!checkConnected())
		return;

	RegistrationDlg *w = findDialog<RegistrationDlg*>(j);
	if(w)
		bringToFront(w);
	else {
		w = new RegistrationDlg(j, this);
		w->show();
	}
}

void PsiAccount::actionSearch(const Jid &j)
{
	if(!checkConnected())
		return;

	SearchDlg *w = findDialog<SearchDlg*>(j);
	if(w)
		bringToFront(w);
	else {
		w = new SearchDlg(j, this);
		connect(w, SIGNAL(add(const XMPP::Jid &, const QString &, const QStringList &, bool)), SLOT(dj_add(const XMPP::Jid &, const QString &, const QStringList &, bool)));
		connect(w, SIGNAL(aInfo(const Jid &)), SLOT(actionInfo(const Jid &)));
		w->show();
	}
}

void PsiAccount::actionInvite(const Jid &j, const QString &gc)
{
	Message m;
	Jid room(gc);
	m.setTo(room);
	m.addMUCInvite(MUCInvite(j));

	QString password = d->client->groupChatPassword(room.user(),room.host());
	if (!password.isEmpty())
		m.setMUCPassword(password);
	m.setTimeStamp(QDateTime::currentDateTime());
	dj_sendMessage(m);
}

void PsiAccount::actionAssignKey(const Jid &j)
{
	if(ensureKey(j)) {
		UserListItem *u = findFirstRelevant(j);
		if(u)
			cpUpdate(*u);
	}
}

void PsiAccount::actionUnassignKey(const Jid &j)
{
	UserListItem *u = findFirstRelevant(j);
	if(u) {
		u->setPublicKeyID("");
		cpUpdate(*u);
	}
}

void PsiAccount::dj_sendMessage(const Message &m, bool log)
{
	UserListItem *u = findFirstRelevant(m.to());
	Message nm = m;

	if(option.incomingAs == 3) {
		if(u) {
			switch(u->lastMessageType()) {
				case 0: nm.setType(""); break;
				case 1: nm.setType("chat"); break;
			}
		}
	}

	if (!nm.body().isEmpty()) {
		UserListItem *u = findFirstRelevant(m.to());
		if (!u || u->subscription().type() != Subscription::Both && u->subscription().type() != Subscription::From) {
			nm.setNick(nick());
		}
	}

	d->client->sendMessage(nm);

	// only toggle if not an invite or body is not empty
	if(m.invite().isEmpty() && !m.body().isEmpty())
		toggleSecurity(m.to(), m.wasEncrypted());

	// don't log groupchat, private messages, or encrypted messages
	if(d->acc.opt_log && log) {
		if(m.type() != "groupchat" && m.xencrypted().isEmpty() && !findGCContact(m.to())) {
			MessageEvent *me = new MessageEvent(m, this);
			me->setOriginLocal(true);
			me->setTimeStamp(QDateTime::currentDateTime());
			logEvent(m.to(), me);
			delete me;
		}
	}

	// don't sound when sending groupchat messages or message events
	if(m.type() != "groupchat" && !m.body().isEmpty())
		playSound(eSend);

	// auto close an open messagebox (if non-chat)
	if(m.type() != "chat" && !m.body().isEmpty()) {
		UserListItem *u = findFirstRelevant(m.to());
		if(u) {
			EventDlg *e = findDialog<EventDlg*>(u->jid());
			if(e)
				e->closeAfterReply();
		}
	}
}

void PsiAccount::dj_composeMessage(const Jid &jid, const QString &body, const QString &subject, const QString &thread)
{
	EventDlg *w = d->psi->createEventDlg(jid.full(), this);
	if (!w)
		return;

	if(!body.isEmpty())
		w->setHtml(TextUtil::plain2rich(TextUtil::quote(body)));

	if(!subject.isEmpty() && subject.left(3) != "Re:")
		w->setSubject("Re: " + subject);
	else if (subject.left(3) == "Re:")
		w->setSubject(subject);

	if(!thread.isEmpty())
		w->setThread(thread);

	w->show();
}

void PsiAccount::dj_composeMessage(const Jid &j, const QString &body)
{
	dj_composeMessage(j, body, QString::null, QString::null);
}

void PsiAccount::dj_addAuth(const Jid &j)
{
	dj_addAuth(j,QString());
}


void PsiAccount::dj_addAuth(const Jid &j, const QString& nick)
{
	QString name;
	QStringList groups;
	UserListItem *u = d->userList.find(j);
	if(u) {
		name = u->name();
		groups = u->groups();
	}
	else if (!nick.isEmpty()){
		name = nick;
	}

	dj_add(j, name, groups, true);
	dj_auth(j);
}

void PsiAccount::dj_confirmHttpAuth(const PsiHttpAuthRequest &req)
{
	d->httpAuthManager->confirm(req);
}

void PsiAccount::dj_denyHttpAuth(const PsiHttpAuthRequest &req)
{
	d->httpAuthManager->deny(req);
}

void PsiAccount::dj_formSubmit(const XData& data, const QString& thread, const Jid& jid)
{
	Message m;

	m.setTo(jid);
	m.setThread(thread, true);
	m.setForm(data);

	d->client->sendMessage(m);
}

void PsiAccount::dj_formCancel(const XData& data, const QString& thread, const Jid& jid)
{
	Message m;

	m.setTo(jid);
	m.setThread(thread, true);
	m.setForm(data);

	d->client->sendMessage(m);
}

void PsiAccount::dj_add(const XMPP::Jid &j, const QString &name, const QStringList &groups, bool authReq)
{
	JT_Roster *r = new JT_Roster(d->client->rootTask());
	r->set(j, name, groups);
	r->go(true);

	if(authReq)
		dj_authReq(j);
}

void PsiAccount::dj_authReq(const Jid &j)
{
	d->client->sendSubscription(j, "subscribe", nick());
}

#ifdef YAPSI
void PsiAccount::dj_authInternal(const Jid &j)
{
	d->client->sendSubscription(j, "subscribed");
	dj_authReq(j);
}
#endif

void PsiAccount::dj_auth(const Jid &j)
{
#ifdef YAPSI
	UserListItem* u = d->userList.find(j);
	if (!u || !u->inList()) {
		// TODO: need some sort of central dispatcher for this kind of stuff
		emit YaRosterToolTip::instance()->addContact(j);
		return;
	}

	dj_authInternal(j);
#else
	d->client->sendSubscription(j, "subscribed");
#endif
}

void PsiAccount::dj_deny(const Jid &j)
{
	d->client->sendSubscription(j, "unsubscribed");

	UserListItem* userListItem = d->userList.find(j);
	if (userListItem && !userListItem->inList()) {
		actionRemove(j);
	}
}

void PsiAccount::dj_rename(const Jid &j, const QString &name)
{
	UserListItem *u = d->userList.find(j);
	if(!u)
		return;

	QString str;
	if(name == u->jid().full())
		str = "";
	else
		str = name;

	// strange workaround to avoid a null string ??
	QString uname;
	if(u->name().isEmpty())
		uname = "";
	else
		uname = u->name();

	if(uname == str)
		return;
	u->setName(str);

	cpUpdate(*u);

	if(u->inList()) {
		JT_Roster *r = new JT_Roster(d->client->rootTask());
		r->set(u->jid(), u->name(), u->groups());
		r->go(true);
	}
}

void PsiAccount::dj_remove(const Jid &j)
{
	UserListItem *u = d->userList.find(j);
	if(!u)
		return;

	// remove all events from the queue
	d->eventQueue->clear(j);
	updateReadNext(j);

	// TODO: delete the item immediately (to simulate local change)
	// should be done with care though, as the server could refuse
	// to delete a contact
	if(!u->inList()) {
		d->removeEntry(u->jid());
		d->userList.removeRef(u);
	}
	else {
		JT_Roster *r = new JT_Roster(d->client->rootTask());
		r->remove(j);
		r->go(true);

		// if it looks like a transport, unregister (but not if it is the server!!)
		if(u->isTransport() && !Jid(d->client->host()).compare(u->jid())) {
			JT_UnRegister *ju = new JT_UnRegister(d->client->rootTask());
			ju->unreg(j);
			ju->go(true);
		}
	}
}

void PsiAccount::dj_rosterExchange(const RosterExchangeItems& items)
{
	foreach(RosterExchangeItem item, items) {
		if (!validRosterExchangeItem(item))
			continue;

		if (item.action() == RosterExchangeItem::Add) {
			if (d->client->roster().find(item.jid(),true) == d->client->roster().end()) {
				dj_add(item.jid(),item.name(),item.groups(),true);
			}
		}
		else if (item.action() == RosterExchangeItem::Delete) {
			//dj_remove(item.jid());
		}
		else if (item.action() == RosterExchangeItem::Modify) {
			// TODO
		}
	}
}

void PsiAccount::eventFromXml(PsiEvent* e)
{
	handleEvent(e, FromXml);
}

// handle an incoming event
void PsiAccount::handleEvent(PsiEvent* e, ActivationType activationType)
{
	if (e && activationType != FromXml) {
		setEnabled();
	}

	bool doPopup    = false;
	bool putToQueue = true;
	PsiPopup::PopupType popupType = PsiPopup::AlertNone;
	int soundType = -1;

	// find someone to accept the event
	Jid j;
	QList<UserListItem*> ul = findRelevant(e->from());
	if(ul.isEmpty()) {
		// if groupchat, then we want the full JID
		if(findGCContact(e->from())) {
			j = e->from();
		}
		else {
			Jid bare = e->from().userHost();
			Jid reg = bare.withResource("registered");

			// see if we have a "registered" variant of the jid
			if(findFirstRelevant(reg)) {
				j = reg;
				e->setFrom(reg); // HACK!!
			}
			// retain full jid if sent to "registered"
			else if(e->from().resource() == "registered")
				j = e->from();
			// otherwise don't use the resource for new entries
			else
				j = bare;
		}
	}
	else {
		j = ul.first()->jid();
	}

	e->setJid(j);

#ifdef PSI_PLUGINS
	QDomElement eXml = e->toXml(new QDomDocument());
	if (!PluginManager::instance()->processEvent(this, eXml)) {
		delete e;
		return;
	}
	//FIXME(KIS): must now cause the event to be recreated from this xml or such. Horrid.
#endif

	if (d->psi->filterEvent(this, e)) {
		delete e;
		return;
	}

	if (d->acc.opt_log && activationType != FromXml) {
		if (e->type() == PsiEvent::Message ||
		    e->type() == PsiEvent::Auth
#ifdef YAPSI
		    || e->type() == PsiEvent::Mood
#endif
		   )
		{
			bool found = false;
#ifdef YAPSI
			// FIXME: WARNING: This triggers qApp->processEvents()
			// which could have nasty side effects inside
			if (e->type() == PsiEvent::Mood) {
				MoodEvent* moodEvent = static_cast<MoodEvent*>(e);
				foreach(Ya::SpooledMessage spooledMessage, Ya::lastMessages(this, e->from(), 5)) {
					if (spooledMessage.isMood && spooledMessage.mood == moodEvent->mood()) {
						found = true;
						break;
					}
				}
			}
#endif

			// don't log private messages
			if (!found &&
			    !findGCContact(e->from()) &&
			    !(e->type() == PsiEvent::Message &&
			      ((MessageEvent *)e)->message().body().isEmpty()))
			{
				logEvent(e->from(), e);
			}
		}
	}

	if(e->type() == PsiEvent::Message) {
		MessageEvent *me = (MessageEvent *)e;
		const Message &m = me->message();

		// Pass message events to chat window
		if ((m.containsEvents() || m.chatState() != StateNone) && m.body().isEmpty()) {
			if (option.messageEvents) {
				ChatDlg *c = findChatDialog(e->from());
				if (!c) {
					c = findChatDialog(e->jid());
				}
				if (c) {
					c->incomingMessage(m);
				}
			}
			return;
		}

		// pass chat messages directly to a chat window if possible (and deal with sound)
		if(m.type() == "chat") {
			ChatDlg *c = findChatDialog(e->from());
			if(!c)
				c = findChatDialog(e->jid());

			if(c)
				c->setJid(e->from());

			//if the chat exists, and is either open in a tab,
			//or in a window
#ifdef YAPSI_ACTIVEX_SERVER
			// since the event could be deleted later in YaOnline::doToasterIgnored()
			// we must add the message to the dialog so it will appear in timeline
			if( c ) {
#else
			if( c && ( d->tabManager->isChatTabbed(c) || !c->isHidden() ) ) {
#endif
				c->incomingMessage(m);
				soundType = eChat2;
				if(option.alertOpenChats && !c->isActiveTab()) {
					// to alert the chat also, we put it in the queue
					me->setSentToChatWindow(true);
				}
				else {
					putToQueue = false;
#ifdef YAPSI
					if (!d->noPopup(activationType) && option.popupChats) {
						openChat(e->from(), activationType);
					}
#endif
				}
			}
			else {
				bool firstChat = !d->eventQueue->hasChats(e->from());
				soundType = firstChat ? eChat1: eChat2;
			}

			if (putToQueue) {
				doPopup = true;
				popupType = PsiPopup::AlertChat;
			}
		} // /chat
		else if (m.type() == "headline") {
			soundType = eHeadline;
			doPopup = true;
			popupType = PsiPopup::AlertHeadline;
		} // /headline
		else if (m.type() == "") {
			soundType = eMessage;
			if (m.type() == "") {
				doPopup = true;
				popupType = PsiPopup::AlertMessage;
			}
		} // /""
		else {
			soundType = eSystem;
		}

		if(m.type() == "error") {
			// FIXME: handle message errors
			//msg.text = QString(tr("<big>[Error Message]</big><br>%1").arg(plain2rich(msg.text)));
		}
#ifdef PSI_PLUGINS
		UserListItem *ulItem=NULL;
		if ( !ul.isEmpty() )
			ulItem=ul.first();
		PluginManager::instance()->message(this,e->from(),ulItem,((MessageEvent*)e)->message().body());
#endif
	}
	else if(e->type() == PsiEvent::HttpAuth) {
		soundType = eSystem;
	}
	else if(e->type() == PsiEvent::File) {
		soundType = eIncomingFT;
		doPopup = true;
		popupType = PsiPopup::AlertFile;
	}
	else if(e->type() == PsiEvent::Call) {
// 		playContinuosSound(SJabbinOptions::instance()->onevent[eIncomingCall]);
//
		doPopup = true;
		popupType = PsiPopup::AlertCall;
	}
	else if(e->type() == PsiEvent::RosterExchange) {
		RosterExchangeEvent* re = (RosterExchangeEvent*) e;
		RosterExchangeItems items;
		foreach(RosterExchangeItem item, re->rosterExchangeItems()) {
			if (validRosterExchangeItem(item))
				items += item;
		}
		if (items.isEmpty()) {
			delete e;
			return;
		}
		re->setRosterExchangeItems(items);
		soundType = eSystem;
	}
	else if (e->type() == PsiEvent::Auth) {
		soundType = eSystem;

		AuthEvent *ae = (AuthEvent *)e;
		if(ae->authType() == "subscribe") {
#ifdef YAPSI
			UserListItem *userListItem = d->userList.find(ae->from());
			Q_ASSERT(dynamic_cast<YaPrivacyManager*>(privacyManager()));
			if (dynamic_cast<YaPrivacyManager*>(privacyManager())->isContactBlocked(ae->from())) {
				dj_deny(ae->from());
				putToQueue = false;
			}
			else if (userListItem && userListItem->inList()) {
#else
			if(option.autoAuth) {
#endif
				// Check if we want to request auth as well
				UserListItem *u = d->userList.find(ae->from());
				if (!u || (u->subscription().type() != Subscription::Both && u->subscription().type() != Subscription::To)) {
					dj_addAuth(ae->from(),ae->nick());
				}
				else {
					dj_auth(ae->from());
				}
				putToQueue = false;
			}
		}
		else if(ae->authType() == "subscribed") {
			if(!option.notifyAuth)
				putToQueue = false;
		}
		else if(ae->authType() == "unsubscribe") {
			putToQueue = false;
		}
#ifdef YAPSI
		if (!putToQueue) {
			delete e;
			return;
		}
#endif
	}
#ifdef YAPSI
	else if (e->type() == PsiEvent::Mood) {
		putToQueue = true;

		ChatDlg* c = findChatDialog(e->jid());
		YaChatDlg* yaChat = c ? dynamic_cast<YaChatDlg*>(c) : 0;
		if (yaChat && activationType == IncomingStanza) {
			MoodEvent* moodEvent = static_cast<MoodEvent*>(e);
			yaChat->addMoodChange(ChatDlg::Spooled_None, moodEvent->mood(), moodEvent->timeStamp());

			if (yaChat->isActiveTab()) {
				putToQueue = false;
			}
		}

#ifndef YAPSI_ACTIVEX_SERVER
		// if (!PsiOptions::instance()->getOption("options.ya.popups.moods.enable").toBool()) {
			putToQueue = false;
		// }
#endif
	}
#endif
	else {
		putToQueue = false;
		doPopup = false;
	}

	if (doPopup && !d->noPopup(activationType)) {
		Resource r;
		UserListItem *u = findFirstRelevant(j);
		if (u && u->priority() != u->userResourceList().end()) {
			r = *(u->priority());
		}

		if ((popupType == PsiPopup::AlertChat     && option.ppChat)     ||
		    (popupType == PsiPopup::AlertMessage  && option.ppMessage)  ||
		    (popupType == PsiPopup::AlertHeadline && option.ppHeadline) ||
		    (popupType == PsiPopup::AlertFile     && option.ppFile)		||
			(popupType == PsiPopup::AlertCall	  && option.ppCall))
		{
			PsiPopup *popup = new PsiPopup(popupType, this);
			popup->setData(j, r, u, e);
		}
#if defined(Q_WS_MAC) && defined(HAVE_GROWL)
		PsiGrowlNotifier::instance()->popup(this, popupType, j, r, u, e);
#endif
		emit startBounce();
	}

#ifdef YAPSI
	int id = -1;
#ifdef YAPSI_ACTIVEX_SERVER
	QPointer<PsiEvent> backupEvent = e;
#else
	PsiEvent* backupEvent = e->copy(); // FIXME: temporary workaround for braindead queueEvent
#endif
	if (putToQueue) {
		QList<int> ids = GlobalEventQueue::instance()->ids();
		int lastId = ids.isEmpty() ? -1 : ids.last();

		queueEvent(e, activationType);

		// insurance
		ids = GlobalEventQueue::instance()->ids();
		int lastId2 = ids.isEmpty() ? -1 : ids.last();
		if ((lastId2 != lastId) && (lastId2 != -1))
			id = lastId2;
	}

	if (activationType != FromXml) {
#ifdef YAPSI_ACTIVEX_SERVER
		Q_ASSERT(!backupEvent.isNull());
		PsiEvent* notificationEvent = backupEvent;
#else
		PsiEvent* notificationEvent = id == -1 ? backupEvent : e;
#endif
		if (notificationEvent) {
			YaPopupNotification::notify(id, notificationEvent);
		}
// #ifdef YAPSI_ACTIVEX_SERVER
// 		if (notificationEvent && !notificationEvent->shownInOnline()) {
// #endif
			if (soundType >= 0) {
				playSound(soundType);
			}
// #ifdef YAPSI_ACTIVEX_SERVER
// 		}
// #endif
	}
#ifndef YAPSI_ACTIVEX_SERVER
	delete backupEvent;
#endif

	if (!putToQueue)
		delete e;
#else
	if (soundType >= 0 && activationType != FromXml) {
		playSound(soundType);
	}

	if ( putToQueue )
		queueEvent(e, activationType);
	else
		delete e;
#endif
}

// put an event into the event queue, and update the related alerts
void PsiAccount::queueEvent(PsiEvent* e, ActivationType activationType)
{
	// do we have roster item for this?
	UserListItem *u = find(e->jid());
	if(!u) {
		// create item
		u = new UserListItem;
		u->setJid(e->jid());
		u->setInList(false);
		u->setAvatarFactory(avatarFactory());
		QString nick;
		if (e->type() == PsiEvent::Auth) {
			AuthEvent* ae = (AuthEvent*) e;
			nick = ae->nick();
		}
		else if (e->type() == PsiEvent::Message) {
			MessageEvent* me = (MessageEvent*) e;
			if (me->message().type()  != "error")
				nick = me->nick();
		}
		u->setName(nick);

		// is it a private groupchat?
		Jid j = u->jid();
		GCContact *c = findGCContact(j);
		if(c) {
			u->setName(j.resource());
			u->setPrivate(true);

			// make a resource so the contact appears online
			UserResource ur;
			ur.setName(j.resource());
			ur.setStatus(c->status);
			u->userResourceList().append(ur);
		}

		// treat it like a push  [pushinfo]
		//VCard info;
		//if(readUserInfo(item->jid, &info) && !info.field[vNickname].isEmpty())
		//	item->nick = info.field[vNickname];
		//else {
		//	if(localStatus != STATUS_OFFLINE)
		//		serv->getVCard(item->jid);
		//}

		d->userList.append(u);
	}

	//printf("queuing message from [%s] for [%s].\n", e->from().full().latin1(), e->jid().full().latin1());
	d->eventQueue->enqueue(e);

	updateReadNext(e->jid());
	if(option.raise)
		d->psi->raiseMainwin();

	// udpate the roster
	cpUpdate(*u);

#ifndef YAPSI
	// if we have both (option.popupMsgs || option.popupChats) && option.alertOpenChats,
	// then option.alertOpenChats will have NO USER-VISIBLE EFFECT as the
	// events will be immediately deleted from the event queue

	// FIXME: We shouldn't be doing this kind of stuff here, because this
	// function is named *queue*Event() not deleteThisMessageSometimes()
	if (!d->noPopup(activationType)) {
		bool doPopup = false;

		// Check to see if we need to popup
		if (e->type() == PsiEvent::Message) {
			MessageEvent *me = (MessageEvent *)e;
			const Message &m = me->message();
			if (m.type() == "chat")
				doPopup = option.popupChats;
			else if (m.type() == "headline")
				doPopup = option.popupHeadlines;
			else
				doPopup = option.popupMsgs;
		}
		else if (e->type() == PsiEvent::File) {
			doPopup = option.popupFiles;
		}
		else if (e->type() == PsiEvent::Auth && !EventDlg::messagingEnabled())
			doPopup = false;
		}
		else if (e->type() == PsiEvent::Call) {
			doPopup = option.popupCalls;
		}
		else {
			doPopup = option.popupMsgs;
		}

		// Popup
		if (doPopup) {
			UserListItem *u = find(e->jid());
			if (u && (!option.noUnlistedPopup || u->inList()))
				openNextEvent(*u, activationType);
		}
	}
#else
	Q_UNUSED(activationType);
#endif
}

// take the next event from the queue and display it
void PsiAccount::openNextEvent(const UserListItem& u, ActivationType activationType)
{
	PsiEvent *e = d->eventQueue->peek(u.jid());
	if(!e)
		return;

	psi()->processEvent(e, activationType);
}

void PsiAccount::openNextEvent(ActivationType activationType)
{
	PsiEvent *e = d->eventQueue->peekNext();
	if(!e)
		return;

	if(e->type() == PsiEvent::PGP) {
		psi()->processEvent(e, activationType);
		return;
	}

	UserListItem *u = find(e->jid());
	if(!u)
		return;
	openNextEvent(*u, activationType);
}

int PsiAccount::forwardPendingEvents(const Jid &jid)
{
	QList<PsiEvent*> chatList;
	d->eventQueue->extractMessages(&chatList);
	foreach(PsiEvent* e, chatList) {
		MessageEvent *me = (MessageEvent *) e;
		Message m = me->message();

		AddressList oFrom = m.findAddresses(Address::OriginalFrom);
		AddressList oTo = m.findAddresses(Address::OriginalTo);

		if (oFrom.count() == 0)
			m.addAddress(Address(Address::OriginalFrom, m.from()));
		if (oTo.count() == 0)
			m.addAddress(Address(Address::OriginalTo, m.to()));

		m.setTimeStamp(m.timeStamp(), true);
		m.setTo(jid);
		m.setFrom("");

		d->client->sendMessage(m);

		// update the eventdlg
		UserListItem *u = find(e->jid());
		delete e;

		// update the contact
		if(u)
			cpUpdate(*u);

		updateReadNext(u->jid());
	}
	return chatList.count();
}

void PsiAccount::updateReadNext(const Jid &j)
{
	// update eventdlg's read-next
	EventDlg *w = findDialog<EventDlg*>(j);
	if(w) {
		PsiIcon *nextAnim = 0;
		int nextAmount = d->eventQueue->count(j);
		if(nextAmount > 0)
			nextAnim = PsiIconset::instance()->event2icon(d->eventQueue->peek(j));
		w->updateReadNext(nextAnim, nextAmount);
	}

	queueChanged();
}

void PsiAccount::processReadNext(const Jid &j)
{
	UserListItem *u = find(j);
	if(u)
		processReadNext(*u);
}

void PsiAccount::processReadNext(const UserListItem &u)
{
	EventDlg *w = findDialog<EventDlg*>(u.jid());
	if(!w) {
		// this should NEVER happen
		return;
	}

	// peek the event
	PsiEvent *e = d->eventQueue->peek(u.jid());
	if(!e)
		return;

	bool isChat = false;
	if(e->type() == PsiEvent::Message) {
		MessageEvent *me = (MessageEvent *)e;
		const Message &m = me->message();
		if(m.type() == "chat" && m.getForm().fields().empty())
			isChat = true;
	}

	// if it's a chat message, just open the chat window.  there is no need to do
	// further processing.  the chat window will remove it from the queue, update
	// the cvlist, etc etc.
	if(isChat) {
		openChat(e->from(), UserAction);
		return;
	}

	// remove from queue
	e = d->eventQueue->dequeue(u.jid());

	// update the eventdlg
	w->updateEvent(e);
	delete e;

	// update the contact
	cpUpdate(u);

	updateReadNext(u.jid());
}

void PsiAccount::processChatsHelper(const Jid& j, bool removeEvents)
{
	//printf("processing chats for [%s]\n", j.full().latin1());
	ChatDlg *c = findChatDialog(j);
	if(!c)
		return;

	// extract the chats
	QList<PsiEvent*> chatList;
	bool compareResources = true;
#ifdef YAPSI
	compareResources = false;
#endif
	d->eventQueue->extractChats(&chatList, j, compareResources);

	if(!chatList.isEmpty()) {
		// dump the chats into the chat window, and remove the related cvlist alerts

		// TODO FIXME: Contact's status changes should also be cached in order to
		// insert them to chatdlg correctly.
		//
		// 15:07 *mblsha is Online
		// 15:15 *mblsha is Offline
		// 15:10 <mblsha> hello!

		foreach(PsiEvent *e, chatList) {
			if (e->type() == PsiEvent::Message) {
				MessageEvent* me = static_cast<MessageEvent*>(e);
				const Message &m = me->message();

				// process the message
				if(!me->sentToChatWindow()) {
					c->incomingMessage(m);
					me->setSentToChatWindow(true);
				}
			}
		}

		if (removeEvents) {
			while (!chatList.isEmpty())
				delete chatList.takeFirst();

			QList<UserListItem*> ul = findRelevant(j);
			if(!ul.isEmpty()) {
				UserListItem *u = ul.first();
				cpUpdate(*u);
				updateReadNext(u->jid());
			}
		}
	}
}

void PsiAccount::processChats(const Jid &j)
{
	processChatsHelper(j, true);
}

void PsiAccount::processCalls(const Jid &j)
{
	printf("processing calls for [%s]\n", j.full().latin1());
	/* ivan: disabling old CallDlg:
         CallDlg *c = findDialog<CallDlg*>(j);
         */

	/*if(!c)
		return;*/

}

void PsiAccount::openChat(const Jid& j, ActivationType activationType)
{
#ifdef YAPSI_ACTIVEX_SERVER
	if (activationType == IncomingStanza) {
		return;
	}
#endif

	ChatDlg* chat = ensureChatDlg(j);
	chat->ensureTabbedCorrectly();

	if (activationType == UserAction) {
		processChats(j);
		chat->bringToFront(!d->psi->restoringSavedChats());
	}

#ifdef YAPSI_ACTIVEX_SERVER
	if (!psi()->restoringSavedChats() && activationType == UserAction) {
		psi()->yaOnline()->doShowIgnoredToasters();
	}
#endif
}

void PsiAccount::openCall(const Jid &j)
{
    qDebug() << "Call?";
    // CallDlg *c = ensureCallDlg(j);
    // processCalls(j);
    // bringToFront(c);
    //c->call();
    CallDialog::instance()->init(j, this, voiceCaller());
    CallDialog::instance()->call();
}

void PsiAccount::chatMessagesRead(const Jid &j)
{
	if(option.alertOpenChats) {
		processChats(j);
	}
}

void PsiAccount::logEvent(const Jid &j, PsiEvent *e)
{
	EDBHandle *h = new EDBHandle(d->psi->edb());
	connect(h, SIGNAL(finished()), SLOT(edb_finished()));
	h->append(j, e);
}

void PsiAccount::edb_finished()
{
	EDBHandle *h = (EDBHandle *)sender();
	delete h;
}

void PsiAccount::openGroupChat(const Jid &j, ActivationType activationType)
{
#ifdef GROUPCHAT
	if (!GCMainDlg::mucEnabled())
		return;

	QString str = j.userHost();
	bool found = false;
	for(QStringList::ConstIterator it = d->groupchats.begin(); it != d->groupchats.end(); ++it) {
		if((*it) == str) {
			found = true;
			break;
		}
	}
	if(!found)
		d->groupchats += str;

	GCMainDlg *w = new GCMainDlg(this, j, d->tabManager);
	w->setPassword(d->client->groupChatPassword(j.user(),j.host()));
	connect(w, SIGNAL(aSend(const Message &)), SLOT(dj_sendMessage(const Message &)));
	connect(d->psi, SIGNAL(emitOptionsUpdate()), w, SLOT(optionsUpdate()));
	w->ensureTabbedCorrectly();
	if (activationType == UserAction)
		w->bringToFront();
#endif
}

bool PsiAccount::groupChatJoin(const QString &host, const QString &room, const QString &nick, const QString& pass, bool nohistory)
{
	if (nohistory)
		return d->client->groupChatJoin(host, room, nick, pass, 0);
	else {
		Status s = d->loginStatus;
		s.setXSigned("");
		return d->client->groupChatJoin(host, room, nick, pass, d->options->getOption("options.muc.context.maxchars").toInt(),d->options->getOption("options.muc.context.maxstanzas").toInt(),d->options->getOption("options.muc.context.seconds").toInt(),s);
	}
}

void PsiAccount::groupChatChangeNick(const QString &host, const QString &room, const QString& nick, const Status &s)
{
	d->client->groupChatChangeNick(host, room, nick, s);
}

void PsiAccount::groupChatSetStatus(const QString &host, const QString &room, const Status &s)
{
	d->client->groupChatSetStatus(host, room, s);
}

void PsiAccount::groupChatLeave(const QString &host, const QString &room)
{
	d->groupchats.remove(room + '@' + host);
	d->client->groupChatLeave(host, room);
}

GCContact *PsiAccount::findGCContact(const Jid &j)
{
	foreach(GCContact *c, d->gcbank) {
		if(c->jid.compare(j))
			return c;
	}
	return 0;
}

QStringList PsiAccount::groupchats() const
{
	return d->groupchats;
}

void PsiAccount::client_groupChatJoined(const Jid &j)
{
#ifdef GROUPCHAT
	if (!GCMainDlg::mucEnabled())
		return;

	//d->client->groupChatSetStatus(j.host(), j.user(), d->loginStatus);

	GCMainDlg *m = findDialog<GCMainDlg*>(Jid(j.userHost()));
	if(m) {
		m->setPassword(d->client->groupChatPassword(j.user(),j.host()));
		m->joined();
		return;
	}
	MUCJoinDlg *w = findDialog<MUCJoinDlg*>(j);
	if(!w)
		return;
	w->joined();

	// TODO: Correctly handle auto-join groupchats
	openGroupChat(j, UserAction);
#endif
}

void PsiAccount::client_groupChatLeft(const Jid &j)
{
	// remove all associated groupchat contacts from the bank
	for(QList<GCContact*>::Iterator it = d->gcbank.begin(); it != d->gcbank.end(); ) {
		GCContact *c = *it;

		// contact from this room?
		if(!c->jid.compare(j, false)) {
			++it;
			continue;
		}
		UserListItem *u = find(c->jid);
		if(!u) {
			++it;
			continue;
		}

		simulateContactOffline(u);
		it = d->gcbank.erase(it);
		delete c;
	}
}

void PsiAccount::client_groupChatPresence(const Jid &j, const Status &s)
{
#ifdef GROUPCHAT
	if (!GCMainDlg::mucEnabled())
		return;

	GCMainDlg *w = findDialog<GCMainDlg*>(Jid(j.userHost()));
	if(!w)
		return;

	GCContact *c = findGCContact(j);
	if(!c) {
		c = new GCContact;
		c->jid = j;
		c->status = s;
		d->gcbank.append(c);
	}

	w->presence(j.resource(), s);

	// pass through the core presence handling also (so that roster items
	// from groupchat contacts get a resource as well
	Resource r;
	r.setName(j.resource());
	r.setStatus(s);
	if(s.isAvailable())
		client_resourceAvailable(j, r);
	else
		client_resourceUnavailable(j, j.resource());
#endif
}

void PsiAccount::client_groupChatError(const Jid &j, int code, const QString &str)
{
#ifdef GROUPCHAT
	if (!GCMainDlg::mucEnabled())
		return;

	GCMainDlg *w = findDialog<GCMainDlg*>(Jid(j.userHost()));
	if(w) {
		w->error(code, str);
	}
	else {
		MUCJoinDlg *w = findDialog<MUCJoinDlg*>(j);
		if(w) {
			w->error(code, str);
		}
	}
#endif
}

QStringList PsiAccount::hiddenChats(const Jid &j) const
{
	QStringList list;

	foreach(ChatDlg* chat, findDialogs<ChatDlg*>(j, false))
		list += chat->jid().resource();

	return list;
}

QStringList PsiAccount::hiddenCalls(const Jid &j) const
{
	QStringList list;

        /*
	foreach(CallDlg* call, findDialogs<CallDlg*>(j, false))
		list += call->jid().resource();
                */

	return list;
}


void PsiAccount::slotCheckVCard()
{
	PsiLogger::instance()->log(QString("%1 PsiAccount(%2)::slotCheckVCard(); isConnected = %3; isActive = %4").arg(LOG_THIS)
	                           .arg(name())
	                           .arg(isConnected())
	                           .arg(isActive()));

	if (!isConnected() || !isActive())
		return;

	QString nick = d->jid.user();
	JT_VCard* j = static_cast<JT_VCard*>(sender());
	if (j->success() && j->statusCode() == Task::ErrDisc) {
		if (!j->vcard().nickName().isEmpty()) {
			d->nickFromVCard = true;
			nick = j->vcard().nickName();
		}
#ifdef YAPSI
		else if (!j->vcard().fullName().isEmpty()) {
			d->nickFromVCard = true;
			nick = j->vcard().fullName();
		}
#endif

#ifdef YAPSI
		if (isYaAccount()) {
			d->mood = j->vcard().mood();
			emit moodChanged();
		}
#endif
	}

#ifndef YAPSI
	if (j->vcard().isEmpty()) {
		changeVCard();
		return;
	}
#endif

	if (!j->vcard().photo().isEmpty()) {
		d->vcardPhotoUpdate(j->vcard().photo());
	}

	setNick(nick);

#ifdef INITIAL_PRESENCE_AFTER_VCARD
	clearCurrentConnectionError();

	// HACKHACKHACK
	Q_ASSERT(!presenceSent);
	d->client->setPresence(d->loginStatus);
	presenceSent = true;
	d->askingForVCard = false;
	stateChanged();
	sentInitialPresence();
#endif
}

void PsiAccount::setNick(const QString &nick)
{
	d->self.setName(nick);
	cpUpdate(d->self);
	nickChanged();
}

QString PsiAccount::nick() const
{
	return d->self.name();
}

#ifdef YAPSI
QString PsiAccount::mood() const
{
	return d->mood;
}
#endif

//void PsiAccount::pgpToggled(bool b)
//{
//	QCA::PGPKey oldkey = d->cur_pgpSecretKey;
//
//	// gaining pgp?
//	if(b)
//		d->cur_pgpSecretKey = d->acc.pgpSecretKey;
//	// losing it?
//	else {
//		d->cur_pgpSecretKey = QCA::PGPKey();
//	}
//
//	if(!PGPUtil::instance().equals(oldkey,d->cur_pgpSecretKey)) {
//		pgpKeyChanged();
//		// resend status if online
//		if(loggedIn())
//			setStatusDirect(d->loginStatus);
//	}
//}

void PsiAccount::pgpKeysUpdated()
{
	// are there any sigs that need verifying?
	foreach(UserListItem* u, d->userList) {
		UserResourceList &rl = u->userResourceList();
		for(UserResourceList::Iterator rit = rl.begin(); rit != rl.end(); ++rit) {
			UserResource &r = *rit;
			if(!r.status().xsigned().isEmpty() && r.pgpVerifyStatus() == QCA::SecureMessageSignature::NoKey) {
				QCA::KeyStoreEntry e = PGPUtil::instance().getPublicKeyStoreEntry(r.publicKeyID());
				if (!e.isNull())
					tryVerify(u, &r);
			}
		}
	}
}

void PsiAccount::trySignPresence()
{
	QCA::SecureMessageKey skey;
	skey.setPGPSecretKey(d->cur_pgpSecretKey);
	QByteArray plain = d->loginStatus.status().utf8();

	PGPTransaction *t = new PGPTransaction(new QCA::OpenPGP());
	connect(t, SIGNAL(finished()), SLOT(pgp_signFinished()));
	t->setFormat(QCA::SecureMessage::Ascii);
	t->setSigner(skey);
	t->startSign(QCA::SecureMessage::Detached);
	t->update(plain);
	t->end();
}

void PsiAccount::pgp_signFinished()
{
	PGPTransaction *t = (PGPTransaction*) sender();
	if (t->success()) {
		Status s = d->loginStatus;
		s.setXSigned(PGPUtil::instance().stripHeaderFooter(QString(t->signature())));
		setStatusActual(s);
	}
	else {
		// Clear passphrase from cache
		if (t->errorCode() == QCA::SecureMessage::ErrorPassphrase) {
			QCA::KeyStoreEntry ke = PGPUtil::instance().getSecretKeyStoreEntry(d->cur_pgpSecretKey.keyId());
			if (!ke.isNull())
				PGPUtil::instance().removePassphrase(ke.id());
		}

		QMessageBox::critical(0, tr("Error"), tr("There was an error trying to sign your status.\nReason: %1.").arg(PGPUtil::instance().messageErrorString(t->errorCode())));

		logout();
		return;
	}
	t->deleteLater();
}


void PsiAccount::verifyStatus(const Jid &j, const Status &s)
{
	PGPTransaction *t = new PGPTransaction(new QCA::OpenPGP());
	t->setJid(j);
	connect(t, SIGNAL(finished()), SLOT(pgp_verifyFinished()));
	t->startVerify(PGPUtil::instance().addHeaderFooter(s.xsigned(),1).utf8());
	t->update(s.status().utf8());
	t->end();
}


void PsiAccount::pgp_verifyFinished()
{
	PGPTransaction *t = (PGPTransaction*) sender();
	Jid j = t->jid();
	foreach(UserListItem *u, findRelevant(j)) {
		UserResourceList::Iterator rit = u->userResourceList().find(j.resource());
		bool found = (rit == u->userResourceList().end()) ? false: true;
		if(!found)
			continue;
		UserResource &ur = *rit;

		QCA::SecureMessageSignature signer;
		if(t->success()) {
			signer = t->signer();

			ur.setPublicKeyID(signer.key().pgpPublicKey().keyId());
			ur.setPGPVerifyStatus(signer.identityResult());
			ur.setSigTimestamp(signer.timestamp());

			// if the key doesn't match the assigned key, unassign it
			if(signer.key().pgpPublicKey().keyId() != u->publicKeyID())
				u->setPublicKeyID("");
		}
		else {
			ur.setPGPVerifyStatus(-1);
		}
		cpUpdate(*u);
	}

	t->deleteLater();
}

int PsiAccount::sendMessageEncrypted(const Message &_m)
{
	if(!ensureKey(_m.to()))
		return -1;

	QString keyID = findFirstRelevant(_m.to())->publicKeyID();
	QCA::KeyStoreEntry keyEntry = PGPUtil::instance().getPublicKeyStoreEntry(keyID);
	if (keyEntry.isNull())
		return -1;

	QCA::SecureMessageKey key;
	key.setPGPPublicKey(keyEntry.pgpPublicKey());

	PGPTransaction *t = new PGPTransaction(new QCA::OpenPGP());
	t->setMessage(_m);
	connect(t, SIGNAL(finished()), SLOT(pgp_encryptFinished()));
	t->setFormat(QCA::SecureMessage::Ascii);
	t->setRecipient(key);
	t->startEncrypt();
	t->update(_m.body().utf8());
	t->end();

	return t->id();
}

void PsiAccount::pgp_encryptFinished()
{
	PGPTransaction *pt = (PGPTransaction *)sender();
	int x = pt->id();

	if(pt->success()) {
		Message m = pt->message();
		// log the message here, before we encrypt it
		if(d->acc.opt_log) {
			MessageEvent *me = new MessageEvent(m, this);
			me->setOriginLocal(true);
			me->setTimeStamp(QDateTime::currentDateTime());
			logEvent(m.to(), me);
			delete me;
		}

		Message mwrap;
		mwrap.setTo(m.to());
		mwrap.setType(m.type());
		QString enc = PGPUtil::instance().stripHeaderFooter(pt->read());
		mwrap.setBody(tr("[ERROR: This message is encrypted, and you are unable to decrypt it.]"));
		mwrap.setXEncrypted(enc);
		mwrap.setWasEncrypted(true);
		// FIXME: Should be done cleaner, with an extra method in Iris
		if (m.containsEvent(OfflineEvent)) mwrap.addEvent(OfflineEvent);
		if (m.containsEvent(DeliveredEvent)) mwrap.addEvent(DeliveredEvent);
		if (m.containsEvent(DisplayedEvent)) mwrap.addEvent(DisplayedEvent);
		if (m.containsEvent(ComposingEvent)) mwrap.addEvent(ComposingEvent);
		if (m.containsEvent(CancelEvent)) mwrap.addEvent(CancelEvent);
		mwrap.setChatState(m.chatState());
		dj_sendMessage(mwrap);
	}
	emit encryptedMessageSent(x, pt->success(), pt->errorCode());
	pt->deleteLater();
}


void PsiAccount::processEncryptedMessage(const Message &m)
{
	PGPTransaction *t = new PGPTransaction(new QCA::OpenPGP());
	t->setMessage(m);
	connect(t, SIGNAL(finished()), SLOT(pgp_decryptFinished()));
	t->setFormat(QCA::SecureMessage::Ascii);
	t->startDecrypt();
	t->update(PGPUtil::instance().addHeaderFooter(m.xencrypted(),0).utf8());
	t->end();
}


void PsiAccount::pgp_decryptFinished()
{
	PGPTransaction *pt = (PGPTransaction*) sender();
	bool tryAgain = false;
	if (pt->success()) {
		Message m = pt->message();
		m.setBody(QString::fromUtf8(pt->read()));
		m.setXEncrypted("");
		m.setWasEncrypted(true);
		processIncomingMessage(m);
	}
	else {
		if (loggedIn()) {
			Message m;
			m.setTo(pt->message().from());
			m.setType("error");
			if (!pt->message().id().isEmpty())
				m.setId(pt->message().id());
			m.setBody(pt->message().body());
			m.setError(Stanza::Error(Stanza::Error::Wait,
			                         Stanza::Error::NotAcceptable,
			                         "Unable to decrypt"));
			d->client->sendMessage(m);
		}
	}

	pt->deleteLater();

	if (tryAgain) {
		processEncryptedMessageNext();
	}
	else {
		processEncryptedMessageDone();
	}
}

void PsiAccount::processMessageQueue()
{
	while(!d->messageQueue.isEmpty()) {
		Message *mp = d->messageQueue.first();

		// encrypted?
		if(PGPUtil::instance().pgpAvailable() && !mp->xencrypted().isEmpty()) {
			processEncryptedMessageNext();
			break;
		}

		processIncomingMessage(*mp);
		d->messageQueue.remove(mp);
		delete mp;
	}
}

void PsiAccount::processEncryptedMessageNext()
{
	// 'peek' and try to process it
	Message *mp = d->messageQueue.first();
	processEncryptedMessage(*mp);
}

void PsiAccount::processEncryptedMessageDone()
{
	// 'pop' the message
	if (!d->messageQueue.isEmpty())
		delete d->messageQueue.takeFirst();

	// do the rest of the queue
	processMessageQueue();
}

void PsiAccount::optionsUpdate()
{
	d->updateEntry(d->self);

	// Tune
#ifdef USE_PEP
	bool publish = d->options->getOption("options.extended-presence.tune.publish").toBool();
	if (!d->lastTune.isNull() && !publish) {
		publishTune(Tune());
	}
	else if (d->lastTune.isNull() && publish) {
		Tune current = d->psi->tuneController()->currentTune();
		if (!current.isNull())
			publishTune(current);
	}
#endif

	// Chat states
	setSendChatState(option.messageEvents);

	// Remote Controlling
	setRCEnabled(option.useRC);

	// Roster item exchange
	d->rosterItemExchangeTask->setIgnoreNonRoster(option.ignoreNonRoster);

	// Caps manager
	d->capsManager->setEnabled(option.useCaps);
}


void PsiAccount::setRCEnabled(bool b)
{
	if (b && !d->rcSetStatusServer) {
		d->rcSetStatusServer = new RCSetStatusServer(d->ahcManager);
		d->rcForwardServer = new RCForwardServer(d->ahcManager);
		d->rcSetOptionsServer = new RCSetOptionsServer(d->ahcManager, d->psi);
	}
	else if (!b && d->rcSetStatusServer) {
		delete d->rcSetStatusServer;
		d->rcSetStatusServer = 0;
		delete d->rcForwardServer;
		d->rcForwardServer = 0;
		delete d->rcSetOptionsServer;
		d->rcSetOptionsServer = 0;
	}
}

void PsiAccount::setSendChatState(bool b)
{
	if (b && !d->client->extensions().contains("cs")) {
		d->client->addExtension("cs",Features("http://jabber.org/protocol/chatstates"));
		if (isConnected())
			setStatusActual(d->loginStatus);
	}
	else if (!b && d->client->extensions().contains("cs")) {
		d->client->removeExtension("cs");
		if (isConnected())
			setStatusActual(d->loginStatus);
	}
}


void PsiAccount::invokeGCMessage(const Jid &j)
{
	GCContact *c = findGCContact(j);
	if(!c)
		return;

	// create dummy item, open chat, then destroy item.  HORRIBLE HACK!
	UserListItem *u = new UserListItem;
	u->setJid(j);
	u->setInList(false);
	u->setName(j.resource());
	u->setPrivate(true);

	// make a resource so the contact appears online
	UserResource ur;
	ur.setName(j.resource());
	ur.setStatus(c->status);
	u->userResourceList().append(ur);

	d->userList.append(u);
	actionSendMessage(j);
	d->userList.remove(u);
}

void PsiAccount::invokeGCChat(const Jid &j)
{
	GCContact *c = findGCContact(j);
	if(!c)
		return;

	// create dummy item, open chat, then destroy item.  HORRIBLE HACK!
	UserListItem *u = new UserListItem;
	u->setJid(j);
	u->setInList(false);
	u->setName(j.resource());
	u->setPrivate(true);

	// make a resource so the contact appears online
	UserResource ur;
	ur.setName(j.resource());
	ur.setStatus(c->status);
	u->userResourceList().append(ur);

	d->userList.append(u);
	actionOpenChat(j);
	cpUpdate(*u);
	//d->userList.remove(u);
}

void PsiAccount::invokeGCInfo(const Jid &j)
{
	actionInfo(j);
}

void PsiAccount::invokeGCFile(const Jid &j)
{
	actionSendFile(j);
}

void PsiAccount::toggleSecurity(const Jid &j, bool b)
{
	UserListItem *u = findFirstRelevant(j);
	if(!u)
		return;

	bool isBare = j.resource().isEmpty();
	bool isPriority = false;

	// sick sick sick sick sick sick
	UserResource *r1, *r2;
	r1 = r2 = 0;
	UserResourceList::Iterator it1 = u->userResourceList().find(j.resource());
	UserResourceList::Iterator it2 = u->userResourceList().priority();
	r1 = (it1 != u->userResourceList().end() ? &(*it1) : 0);
	r2 = (it2 != u->userResourceList().end() ? &(*it2) : 0);
	if(r1 && (r1 == r2))
		isPriority = true;

	bool needUpdate = false;
	bool sec = u->isSecure(j.resource());
	if(sec != b) {
		u->setSecure(j.resource(), b);
		needUpdate = true;
	}
	if(isBare && r2) {
		sec = u->isSecure(r2->name());
		if(sec != b) {
			u->setSecure(r2->name(), b);
			needUpdate = true;
		}
	}
	if(isPriority) {
		sec = u->isSecure("");
		if(sec != b) {
			u->setSecure("", b);
			needUpdate = true;
		}
	}

	if(needUpdate)
		cpUpdate(*u);
}

bool PsiAccount::ensureKey(const Jid &j)
{
	if(!PGPUtil::instance().pgpAvailable())
		return false;

	UserListItem *u = findFirstRelevant(j);
	if(!u)
		return false;

	// no key?
	if(u->publicKeyID().isEmpty()) {
		// does the user have any presence signed with a key?
		QString akey;
		const UserResourceList &rl = u->userResourceList();
		for(UserResourceList::ConstIterator it = rl.begin(); it != rl.end(); ++it) {
			const UserResource &r = *it;
			if(!r.publicKeyID().isEmpty()) {
				akey = r.publicKeyID();
				break;
			}
		}

		if(akey.isEmpty() || PGPUtil::instance().getPublicKeyStoreEntry(akey).isNull()) {
			int n = QMessageBox::information(0, CAP(tr("No key")), tr(
				"<p>Psi was unable to locate the OpenPGP key to use for <b>%1</b>.<br>"
				"<br>"
				"This can happen if you do not have the key that the contact is advertising "
				"via signed presence, or if the contact is not advertising any key at all.</p>"
				).arg(JIDUtil::toString(u->jid(),true)), tr("&Choose key manually"), tr("Do &nothing"));
			if(n != 0)
				return false;
		}

		// Select a key
		PGPKeyDlg *w = new PGPKeyDlg(PGPKeyDlg::Public, akey, 0);
		w->setWindowTitle(tr("Public Key: %1").arg(JIDUtil::toString(j,true)));
		int r = w->exec();
		QCA::KeyStoreEntry entry;
		if(r == QDialog::Accepted)
			entry = w->keyStoreEntry();
		delete w;
		if(entry.isNull())
			return false;
		u->setPublicKeyID(entry.pgpPublicKey().keyId());
		cpUpdate(*u);
	}

	return true;
}

ServerInfoManager* PsiAccount::serverInfoManager()
{
	return d->serverInfoManager;
}

PEPManager* PsiAccount::pepManager()
{
	return d->pepManager;
}

BookmarkManager* PsiAccount::bookmarkManager()
{
	return d->bookmarkManager;
}

/**
 * TODO
 */
bool PsiAccount::getExpandedState(QString name) const
{
	QMap<QString, UserAccount::GroupData>::ConstIterator it = d->acc.groupState.find(name);
	if (it != d->acc.groupState.end())
		return it.value().open;
	return true;
}

/**
 * TODO
 */
void PsiAccount::setExpandedState(QString name, bool expanded)
{
	UserAccount::GroupData gd;
	gd.open = expanded;
	gd.rank = 0;
	d->acc.groupState.insert(name, gd);
}

/*
bool PsiAccount::expanded() const
{
	return getExpandedState("/\\/" + name() + "\\/\\");
}

void PsiAccount::setExpanded(bool expanded)
{
	setExpandedState("/\\/" + name() + "\\/\\", expanded);
}

bool PsiAccount::shouldBeVisible() const
{
	return enabled();
}

ContactListGroupItem* PsiAccount::desiredParent() const
{
	return defaultParent();
}

ContactListItemMenu* PsiAccount::contextMenu()
{
	return new PsiAccountMenu(this);
}

QString PsiAccount::toolTip() const
{
	if (!d->selfContact)
		return QString();
	return "<qt><center><b>" + name() + " " + groupInfo() + "</b></center> " + d->selfContact->toolTip() + "</qt>";
}

QIcon PsiAccount::statusIcon() const
{
	if (d->alerting())
		return d->currentAlertFrame();

	return PsiIconset::instance()->status(jid(), status()).icon();
}

bool PsiAccount::compare(const ContactListItem* other) const
{
	const PsiAccount* account = dynamic_cast<const PsiAccount*>(other);
	if (account)
		return name() < account->name();

	return ContactListGroupItem::compare(other);
}
*/

bool PsiAccount::usingSecurityLayer() const
{
	return d->usingSSL;
}

QStringList PsiAccount::groupList() const
{
	return d->groupList();
}

/**
 * FIXME: Make InfoDlg use some other way of updating items.
 */
void PsiAccount::updateEntry(const UserListItem& u)
{
	d->updateEntry(u);
}

/**
 * \brief Returns the current contents of the debug ringbuffer.
 * it doesn't clear the ringbuffer
 * \return a QList<xmlRingElem> of the current debug buffer items.
 */
QList<PsiAccount::xmlRingElem> PsiAccount::dumpRingbuf()
{
	return d->dumpRingbuf();
}

#ifdef YAPSI
void PsiAccount::moodChanged(const QString& mood)
{
	PsiContact* contact = dynamic_cast<PsiContact*>(sender());
	Q_ASSERT(contact);
	if (!contact)
		return;

	if (mood.trimmed().isEmpty()) {
		return;
	}

	{
		static const int moodUpdateTimeoutSecs = 60;
		QDateTime currentDateTime = QDateTime::currentDateTime();
		QMutableHashIterator<QString, QDateTime> it(d->lastTimeMoodChanged_);
		while (it.hasNext()) {
			it.next();
			if (it.value().secsTo(currentDateTime) > moodUpdateTimeoutSecs) {
				it.remove();
			}
		}

		if (d->lastTimeMoodChanged_.contains(contact->jid().bare())) {
			contact->startDelayedMoodUpdate(moodUpdateTimeoutSecs - d->lastTimeMoodChanged_[contact->jid().bare()].secsTo(currentDateTime));
			return;
		}

		d->lastTimeMoodChanged_[contact->jid().bare()] = currentDateTime;
	}

	bool moodSet = false;
	foreach(UserListItem* u, findRelevant(contact->jid())) {
		if (u->yaMoodSet()) {
			moodSet = true;
		}
		u->setYaMood(mood, u->yaMoodSet() || (mood != u->yaMood()));
	}

	// we need to write updated yaMoods to the config.xml
	updatedAccount();

	if (!moodSet) {
		return;
	}

	if (psi()->yaToasterCentral()->moodNotificationsDisabled(contact->jid())) {
		return;
	}

	MoodEvent* moodEvent = new MoodEvent(contact->jid(), mood.trimmed(), this);
	handleEvent(moodEvent, IncomingStanza);
}
#endif

#include "psiaccount.moc"
