/*
 * psicon.cpp - core of Psi
 * Copyright (C) 2001, 2002  Justin Karneges
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

#include "psicon.h"

#include <q3ptrlist.h>
#include <qapplication.h>
#include <qdesktopwidget.h>
#include <QMenuBar>
#include <qpointer.h>
#include <qicon.h>
#include <qcolor.h>
#include <qimage.h>
#include <qpixmapcache.h>
#include <QFile>
#include <QPixmap>
#include <QList>
#include <QImageReader>
#include <QMessageBox>

#include "s5b.h"
#include "psiaccount.h"
#include "activeprofiles.h"
#include "accountadddlg.h"
#include "psiiconset.h"
#include "contactview.h"
#include "psievent.h"
#include "passphrasedlg.h"
#include "common.h"
#ifdef YAPSI
#include "yacommon.h"
#include "yamainwin.h"
#include "yatrayicon.h"
#else
#include "mainwin.h"
#endif
#include "idle.h"
#include "accountmanagedlg.h"
#include "statusdlg.h"
#include "options/optionsdlg.h"
#include "options/opt_toolbars.h"
#include "accountregdlg.h"
#include "combinedtunecontroller.h"
#include "mucjoindlg.h"
#include "userlist.h"
#include "eventdlg.h"
#include "pgputil.h"
#include "eventdb.h"
#include "proxy.h"
#ifdef PSIMNG
#include "psimng.h"
#endif
#include "alerticon.h"
#include "iconselect.h"
#include "psitoolbar.h"
#ifdef FILETRANSFER
#include "filetransfer.h"
#include "filetransdlg.h"
#endif
#include "accountmodifydlg.h"
#include "psiactionlist.h"
#include "applicationinfo.h"
#include "jidutil.h"
#include "systemwatch.h"
#include "accountscombobox.h"
#include "tabdlg.h"
#include "chatdlg.h"
#include "capsregistry.h"
#include "urlobject.h"
#include "anim.h"
#include "psioptions.h"
#ifdef PSI_PLUGINS
#include "pluginmanager.h"
#endif
#include "psicontactlist.h"
#include "dbus.h"
#include "tipdlg.h"
#include "shortcutmanager.h"
#include "globalshortcutmanager.h"
#include "desktoputil.h"
#include "tabmanager.h"
#include "xmpp_xmlcommon.h"
#include "psicontact.h"
#include "psilogger.h"

#ifdef Q_WS_MAC
#include "mac_dock.h"
#endif

#ifdef YAPSI
#include "buzzer.h"
#include "yatoastercentral.h"
#endif
#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif

static const QString lastOpenedTabsOptionPath = "options.ya.chat-window.last-opened-tabs";

//----------------------------------------------------------------------------
// PsiConObject
//----------------------------------------------------------------------------
class PsiConObject : public QObject
{
	Q_OBJECT
public:
	PsiConObject(QObject *parent)
	: QObject(parent, "PsiConObject")
	{
		QDir p(ApplicationInfo::homeDir());
		QDir v(ApplicationInfo::homeDir() + "/tmp-sounds");
		if(!v.exists())
			p.mkdir("tmp-sounds");
		Iconset::setSoundPrefs(v.absPath(), this, SLOT(playSound(QString)));
		connect(URLObject::getInstance(), SIGNAL(openURL(QString)), SLOT(openURL(QString)));
	}

	~PsiConObject()
	{
		// removing temp dirs
		QDir p(ApplicationInfo::homeDir());
		QDir v(ApplicationInfo::homeDir() + "/tmp-sounds");
		folderRemove(v);
	}

public slots:
	void playSound(QString file)
	{
		if ( file.isEmpty() || !useSound )
			return;

		soundPlay(file);
	}

	void openURL(QString url)
	{
		DesktopUtil::openUrl(url);
	}

#ifdef YAPSI
	void updateStyleSheet()
	{
		loadStyleSheet("JoimPsi");
	}
#endif

private:
	// ripped from profiles.cpp
	bool folderRemove(const QDir &_d)
	{
		QDir d = _d;

		QStringList entries = d.entryList();
		for(QStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
			if(*it == "." || *it == "..")
				continue;
			QFileInfo info(d, *it);
			if(info.isDir()) {
				if(!folderRemove(QDir(info.filePath())))
					return FALSE;
			}
			else {
				//printf("deleting [%s]\n", info.filePath().latin1());
				d.remove(info.fileName());
			}
		}
		QString name = d.dirName();
		if(!d.cdUp())
			return FALSE;
		//printf("removing folder [%s]\n", d.filePath(name).latin1());
		d.rmdir(name);

		return TRUE;
	}

#ifdef YAPSI
	static void loadStyleSheet(const QString& sheetName)
	{
		QFile file(":/" + sheetName.toLower() + ".qss");
		file.open(QFile::ReadOnly);
		QString styleSheet = QLatin1String(file.readAll());

#ifndef YAPSI_NO_STYLESHEETS
		qApp->setStyleSheet(styleSheet);
#endif
	}
#endif
};

//----------------------------------------------------------------------------
// PsiCon::Private
//----------------------------------------------------------------------------

struct item_dialog
{
	QWidget *widget;
	QString className;
};

class PsiCon::Private : public QObject
{
	Q_OBJECT
public:
	Private(PsiCon *parent)
		: QObject(parent)
		, contactList(0)
		, iconSelect(0)
#ifdef YAPSI
		, buzzer(new Buzzer())
#endif
		, restoringSavedChats(false)
		, quitting(false)
	{
		psi = parent;
#ifdef YAPSI
		buzzer->setTrayIcon(YaTrayIcon::instance(psi));
#endif
		restoringSavedChats = false;
	}

	~Private()
	{
		if ( iconSelect )
			delete iconSelect;
#ifdef YAPSI
		delete buzzer;
#endif
	}

	void saveProfile(UserAccountList acc)
	{
#ifdef YAPSI_STRESSTEST_ACCOUNTS
		return;
#endif
		pro.recentGCList = recentGCList;
		pro.recentBrowseList = recentBrowseList;
		pro.lastStatusString = lastStatusString;
		pro.useSound = useSound;
		pro.prefs = option;
		if ( proxy )
			pro.proxyList = proxy->itemList();

		pro.acc = acc;

		pro.toFile(pathToProfileConfig(activeProfile));
	}

	void updateIconSelect()
	{
		Iconset iss;
		Q3PtrListIterator<Iconset> iconsets(PsiIconset::instance()->emoticons);
		Iconset *iconset;
		while ( (iconset = iconsets.current()) != 0 ) {
			iss += *iconset;

			++iconsets;
		}

		iconSelect->setIconset(iss);
	}

public slots:
	void saveOpenedChats()
	{
		if (quitting)
			return;

		QDomDocument doc;
		QDomElement root = doc.createElement("items");
		root.setAttribute("version", "1.0");
		doc.appendChild(root);

		foreach(TabDlg* tabDlg, tabManager->tabSets()) {
			for (uint j = 0; j < tabDlg->tabCount(); ++j) {
				ChatDlg* chatDlg = dynamic_cast<ChatDlg*>(tabDlg->getTab(j));
				// TODO: also save groupchats
				if (!chatDlg)
					return;

				QDomElement tag = textTag(&doc, "tab", chatDlg->jid().bare());
				tag.setAttribute("account", chatDlg->account()->id());
				if (tabDlg->tabOnTop(chatDlg))
					tag.setAttribute("current", "true");
				root.appendChild(tag);
			}
		}

		PsiOptions::instance()->setOption(lastOpenedTabsOptionPath, doc.toString());
	}

public:
	void setRestoringSavedChats(bool restoring)
	{
		if (restoring)
			QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		else
			QApplication::restoreOverrideCursor();

		restoringSavedChats = restoring;
		emit psi->restoringSavedChatsChanged();
	}

	void restoreSavedChats()
	{
#ifdef YAPSI_STRESSTEST_ACCOUNTS
		return;
#endif
		QDomDocument doc;
		if (!doc.setContent(PsiOptions::instance()->getOption(lastOpenedTabsOptionPath).toString()))
			return;

		QDomElement root = doc.documentElement();
		if (root.tagName() != "items" || root.attribute("version") != "1.0")
			return;

		setRestoringSavedChats(true);

		QList<PsiContact*> restoredContacts;
		PsiContact* currentContact = 0;

		for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
			QDomElement e = n.toElement();
			if (e.isNull())
				continue;

			if (e.tagName() == "tab") {
				PsiAccount* account = contactList->getAccount(e.attribute("account"));
				if (account) {
					PsiContact* contact = account->findContact(e.text());
					if (contact && !restoredContacts.contains(contact)) {
						restoredContacts << contact;
						if (e.attribute("current") == "true")
							currentContact = contact;

						account->actionOpenSavedChat(contact->jid());
					}
				}
			}
		}

		if (currentContact) {
			// currentContact->openChat();

			foreach(TabDlg* tabDlg, tabManager->tabSets()) {
				TabbableWidget* tab = tabDlg->getTabPointer(currentContact->account(), currentContact->jid().full());
				if (tab) {
					Q_ASSERT(!tab->isActiveWindow());
					tabDlg->hide();
					tabDlg->selectTab(tab);
					tabDlg->showWithoutActivation();
					break;
				}
			}
		}

		setRestoringSavedChats(false);
	}

	PsiCon* psi;
	PsiContactList* contactList;
	UserProfile pro;
	QString lastStatusString;
#ifdef YAPSI
	YaMainWin* mainwin;
#else
	MainWin *mainwin;
#endif
	Idle idle;
	QList<item_dialog*> dialogList;
	QStringList recentGCList, recentBrowseList, recentNodeList;
	EDB *edb;
	S5BServer *s5bServer;
	ProxyManager *proxy;
	IconSelectPopup *iconSelect;
#ifdef FILETRANSFER
	FileTransDlg *ftwin;
#endif
	PsiActionList *actionList;
	//GlobalAccelManager *globalAccelManager;
	TuneController* tuneController;
	QMenuBar* defaultMenuBar;
	CapsRegistry* capsRegistry;
	TabManager *tabManager;
#ifdef YAPSI
	Buzzer* buzzer;
#endif
	bool restoringSavedChats;
	bool quitting;
	QTimer* updatedAccountTimer_;
};

//----------------------------------------------------------------------------
// PsiCon
//----------------------------------------------------------------------------

PsiCon::PsiCon()
	: QObject(0)
#ifdef YAPSI_ACTIVEX_SERVER
	, yaOnline_(0)
#endif
{
	//pdb(DEBUG_JABCON, QString("%1 v%2\n By Justin Karneges\n    infiniti@affinix.com\n\n").arg(PROG_NAME).arg(PROG_VERSION));
	d = new Private(this);
	d->tabManager = new TabManager(this);
	connect(d->tabManager, SIGNAL(openedChatsChanged()), d, SLOT(saveOpenedChats()));
	connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(aboutToQuit()));

#ifdef YAPSI_ACTIVEX_SERVER
	yaOnline_ = YaOnlineHelper::instance();
#endif

	d->lastStatusString = "";
	useSound = true;
	d->mainwin = 0;
#ifdef FILETRANSFER
	d->ftwin = 0;
#endif

	d->edb = new EDBFlatFile;

	d->s5bServer = 0;
	d->proxy = 0;
	d->tuneController = 0;

	d->actionList = 0;
	d->defaultMenuBar = new QMenuBar(0);

	d->capsRegistry = new CapsRegistry();
	connect(d->capsRegistry, SIGNAL(registered(const CapsSpec&)), SLOT(saveCapabilities()));
}

PsiCon::~PsiCon()
{
	deinit();

	saveCapabilities();
	delete d->capsRegistry;

	delete d->actionList;
	delete d->edb;
	delete d->defaultMenuBar;
	delete d->tabManager;
	delete d;
}

bool PsiCon::init()
{
	LOG_TRACE;
	// check active profiles
	if (!ActiveProfiles::instance()->setThisProfile(activeProfile))
		return false;

	connect(qApp, SIGNAL(forceSavePreferences()), SLOT(forceSavePreferences()));

	// PGP initialization (needs to be before any gpg usage!)
	PGPUtil::instance();

	d->contactList = new PsiContactList(this);

	connect(d->contactList, SIGNAL(accountAdded(PsiAccount*)), SIGNAL(accountAdded(PsiAccount*)));
	connect(d->contactList, SIGNAL(accountRemoved(PsiAccount*)), SIGNAL(accountRemoved(PsiAccount*)));
	connect(d->contactList, SIGNAL(accountCountChanged()), SIGNAL(accountCountChanged()));
	connect(d->contactList, SIGNAL(accountActivityChanged()), SIGNAL(accountActivityChanged()));
	connect(d->contactList, SIGNAL(saveAccounts()), SLOT(saveAccounts()));
	connect(d->contactList, SIGNAL(queueChanged()), SLOT(queueChanged()));

	d->updatedAccountTimer_ = new QTimer(this);
	d->updatedAccountTimer_->setSingleShot(true);
	d->updatedAccountTimer_->setInterval(1000);
	connect(d->updatedAccountTimer_, SIGNAL(timeout()), SLOT(saveAccounts()));

	// since these settings are currently stored in config.xml, we need to trigger an account save
	connect(d->contactList, SIGNAL(showAgentsChanged(bool)), d->updatedAccountTimer_, SLOT(start()));
	connect(d->contactList, SIGNAL(showHiddenChanged(bool)), d->updatedAccountTimer_, SLOT(start()));
	connect(d->contactList, SIGNAL(showSelfChanged(bool)), d->updatedAccountTimer_, SLOT(start()));
	connect(d->contactList, SIGNAL(showOfflineChanged(bool)), d->updatedAccountTimer_, SLOT(start()));
	connect(d->contactList, SIGNAL(showGroupsChanged(bool)), d->updatedAccountTimer_, SLOT(start()));

	// To allow us to upgrade from old hardcoded options gracefully, be careful about the order here
	PsiOptions *options=PsiOptions::instance();
	//load the system-wide defaults, if they exist
	QString systemDefaults=ApplicationInfo::resourcesDir();
	systemDefaults += "/options-default.xml";
	//qWarning(qPrintable(QString("Loading system defaults from %1").arg(systemDefaults)));
	options->load(systemDefaults);

#ifdef USE_PEP
	// Create the tune controller
	d->tuneController = new CombinedTuneController();
#endif

	// load the old profile
	d->pro.reset();
	d->pro.fromFile(pathToProfileConfig(activeProfile));

#ifdef YAPSI_STRESSTEST_ACCOUNTS
	{
		d->pro.acc.clear();

		// for (int i = 0; i < 100; i++) {
		// 	UserAccount acc;
		// 	acc.name = QString::number(i);
		// 	acc.jid  = QString("helloworld_%1@om.beeonline.ru").arg(i);
		// 	acc.pass = "foobar";
		// 	acc.resource = QString::number(i);
		// 	acc.priority = 0;
		//
		// 	acc.opt_ignoreSSLWarnings = true;
		// 	acc.opt_host = true;
		// 	acc.host = "om.beeonline.ru";
		// 	acc.port = 5222;
		//
		// 	d->pro.acc << acc;
		// }

		// TODO: there is a bug that reproduces when there are lots of messageboxes saying
		// Conflict (remote login replacing this one)

		for (int i = 0; i < 100; i++) {
			UserAccount acc;
			acc.name = QString::number(i);
			acc.jid  = "user@jabber.com");
			acc.pass = "secretpassword";
			acc.opt_automatic_resource = false;
			acc.resource = QString::number(i);
			acc.priority = 0;

			acc.opt_ignoreSSLWarnings = true;
			acc.opt_host = true;
			acc.host = "jabber.com";
			acc.port = 5222;

			d->pro.acc << acc;
		}
	}
#endif

	//load the new profile
	options->load(optionsFile());
	//Save every time an option is changed
	options->autoSave(true, optionsFile());

	//just set a dummy option to trigger saving
	options->setOption("trigger-save",false);
	options->setOption("trigger-save",true);

	connect(options, SIGNAL(optionChanged(const QString&)), SLOT(optionsUpdate()));

#ifdef YAPSI
	PsiOptions::instance()->setOption("options.ui.message.enabled", false);
	PsiOptions::instance()->setOption("options.ya.main-window.status-bar.show", false);
	PsiOptions::instance()->setOption("options.muc.bookmarks.auto-join", false);
	PsiOptions::instance()->setOption("options.ui.muc.enabled", false);
#endif

#ifdef YAPSI
	yaToasterCentral_ = new YaToasterCentral(this);
#endif

	QDir profileDir( pathToProfile( activeProfile ) );
	profileDir.rmdir( "info" ); // remove unused dir

	d->recentGCList = d->pro.recentGCList;
	d->recentBrowseList = d->pro.recentBrowseList;
	d->lastStatusString = d->pro.lastStatusString;
	useSound = d->pro.useSound;

	option = d->pro.prefs;

	// first thing, try to load the iconset
	if( !PsiIconset::instance()->loadAll() ) {
		//option.iconset = "stellar";
		//if(!is.load(option.iconset)) {
			QMessageBox::critical(0, tr("Error"), tr("Unable to load iconset!  Please make sure Psi is properly installed."));
			return false;
		//}
	}

	if ( !d->actionList )
		d->actionList = new PsiActionList( this );

	PsiConObject* psiConObject = new PsiConObject(this);

	Anim::setMainThread(QThread::currentThread());

	d->iconSelect = new IconSelectPopup(0);
	d->updateIconSelect();

	// setup the main window
#ifdef YAPSI
	d->mainwin = new YaMainWin(option.alwaysOnTop, (option.useDock && option.dockToolMW), this, "joimpsimain");
#else
	d->mainwin = new MainWin(option.alwaysOnTop, (option.useDock && option.dockToolMW), this, "psimain");
#endif
	d->mainwin->setUseDock(option.useDock);

#ifdef YAPSI
// #ifndef Q_WS_MAC
	psiConObject->updateStyleSheet();
// #else
// 	QTimer::singleShot(10, psiConObject, SLOT(updateStyleSheet()));
// #endif
#else
	Q_UNUSED(psiConObject);
#endif

	connect(d->mainwin, SIGNAL(closeProgram()), SLOT(closeProgram()));
	connect(d->mainwin, SIGNAL(changeProfile()), SLOT(changeProfile()));
#ifndef YAPSI
	connect(d->mainwin, SIGNAL(doManageAccounts()), SLOT(doManageAccounts()));
#endif
	connect(d->mainwin, SIGNAL(doGroupChat()), SLOT(doGroupChat()));
	connect(d->mainwin, SIGNAL(blankMessage()), SLOT(doNewBlankMessage()));
	connect(d->mainwin, SIGNAL(statusChanged(int)), SLOT(statusMenuChanged(int)));
	connect(d->mainwin, SIGNAL(doOptions()), SLOT(doOptions()));
	connect(d->mainwin, SIGNAL(doToolbars()), SLOT(doToolbars()));
	connect(d->mainwin, SIGNAL(doFileTransDlg()), SLOT(doFileTransDlg()));
	connect(d->mainwin, SIGNAL(recvNextEvent()), SLOT(recvNextEvent()));
	connect(this, SIGNAL(emitOptionsUpdate()), d->mainwin, SLOT(optionsUpdate()));

#ifdef YAPSI
	{
		QString geometryOptionPath = "options.ya.main-window.last-geometry";
		QRect mwgeom;

		{
			mwgeom.setRect(64, 64, 260, 453);

			QDesktopWidget* desktop = qApp->desktop();
			if (desktop) {
				QRect r = desktop->availableGeometry();
				mwgeom.setHeight(qMax(mwgeom.height(), r.height() - 270));
				mwgeom.moveTopLeft(QPoint(
				                       r.width() / 2 + (r.width() / 2 - mwgeom.width()) / 2,
				                       (r.height() - mwgeom.height()) / 2)
				                  );
			}
		}

		Ya::initializeDefaultGeometry(d->mainwin, geometryOptionPath, mwgeom, false);
		d->mainwin->setGeometryOptionPath(geometryOptionPath);
	}
#else
	d->mainwin->setGeometryOptionPath("options.ui.contactlist.saved-window-geometry");
#endif

#ifdef FILETRANSFER
	d->ftwin = new FileTransDlg(this);
#endif

	d->idle.start();

	// S5B
	d->s5bServer = new S5BServer;
	s5b_init();

	// proxy
	d->proxy = new ProxyManager(this);
	d->proxy->setItemList(d->pro.proxyList);
	connect(d->proxy, SIGNAL(settingsChanged()), SLOT(proxy_settingsChanged()));

	// Disable accounts if necessary, and overwrite locked properties
	if (PsiOptions::instance()->getOption("options.ui.account.single").toBool() || !PsiOptions::instance()->getOption("options.account.domain").toString().isEmpty()) {
		bool haveEnabled = false;
		for(UserAccountList::Iterator it = d->pro.acc.begin(); it != d->pro.acc.end(); ++it) {
			// With single accounts, only modify the first account
			if (PsiOptions::instance()->getOption("options.ui.account.single").toBool()) {
				if (!haveEnabled) {
					haveEnabled = it->opt_enabled;
					if (it->opt_enabled) {
						if (!PsiOptions::instance()->getOption("options.account.domain").toString().isEmpty())
							it->jid = JIDUtil::accountFromString(Jid(it->jid).user()).bare();
					}
				}
				else
					it->opt_enabled = false;
			}
			else {
				// Overwirte locked properties
				if (!PsiOptions::instance()->getOption("options.account.domain").toString().isEmpty())
					it->jid = JIDUtil::accountFromString(Jid(it->jid).user()).bare();
			}
		}
	}

	// Connect to the system monitor
	SystemWatch* sw = SystemWatch::instance();
	connect(sw, SIGNAL(sleep()), this, SLOT(doSleep()));
	connect(sw, SIGNAL(wakeup()), this, SLOT(doWakeup()));

#ifdef PSI_PLUGINS
	// Plugin Manager
	PluginManager::instance();
#endif

	// Global shortcuts
	setShortcuts();

	// FIXME
#ifdef __GNUC__
#warning "Temporary hard-coding caps registration of own version"
#endif
	// client()->identity()

	registerCaps(ApplicationInfo::capsVersion(), QStringList()
	             << "http://jabber.org/protocol/bytestreams"
	             << "http://jabber.org/protocol/si"
	             << "http://jabber.org/protocol/si/profile/file-transfer"
	             << "http://jabber.org/protocol/disco#info"
	             << "http://jabber.org/protocol/commands"
	             << "http://jabber.org/protocol/rosterx"
	             << "http://jabber.org/protocol/muc"
	             << "jabber:x:data"
	            );

	registerCaps("ep", QStringList()
	             << "http://jabber.org/protocol/mood"
	             << "http://jabber.org/protocol/tune"
	             << "http://jabber.org/protocol/physloc"
	             << "http://jabber.org/protocol/geoloc"
	             << "http://www.xmpp.org/extensions/xep-0084.html#ns-data"
	             << "http://www.xmpp.org/extensions/xep-0084.html#ns-metadata"
	            );

	registerCaps("ep-notify", QStringList()
	             << "http://jabber.org/protocol/mood+notify"
	             << "http://jabber.org/protocol/tune+notify"
	             << "http://jabber.org/protocol/physloc+notify"
	             << "http://jabber.org/protocol/geoloc+notify"
	             << "http://www.xmpp.org/extensions/xep-0084.html#ns-metadata+notify"
	            );

	registerCaps("html", QStringList("http://jabber.org/protocol/xhtml-im"));
	registerCaps("cs", QStringList("http://jabber.org/protocol/chatstates"));
	//I've commented out the automatic replies, so commenting out support as well - KIS
	registerCaps("mr", QStringList("urn:xmpp:receipts"));

	// load accounts
	d->contactList->loadAccounts(d->pro.acc);
	checkAccountsEmpty();

	d->mainwin->hide();

#if !defined(YAPSI_ACTIVEX_SERVER) && !defined(YAPSI_STRESSTEST_ACCOUNTS)
#ifndef YAPSI
	if(!(option.useDock && option.dockHideMW))
#endif
		d->mainwin->show();
#endif

	// try autologin if needed
	foreach(PsiAccount* account, d->contactList->accounts()) {
		account->autoLogin();
	}

#ifndef YAPSI
	// show tip of the day
	if ( PsiOptions::instance()->getOption("options.ui.tip.show").toBool() ) {
		TipDlg::show(this);
	}
#endif

#ifdef USE_DBUS
	addPsiConAdapter(this);
#endif
	connect(ActiveProfiles::instance(), SIGNAL(raiseMainWindow()), SLOT(raiseMainwin()));

	d->restoreSavedChats();
	LOG_TRACE;
#ifdef YAPSI_ACTIVEX_SERVER
	// Loading of individual account's events is in QTimer::singleShot(0, d, SLOT(loadQueue())),
	// so we're postponing re-sending of toasters till 10 seconds later
	QTimer::singleShot(10 * 1000, yaOnline(), SLOT(notifyAllUnshownEvents()));
#endif

	return true;
}

void PsiCon::registerCaps(const QString& ext, const QStringList& features)
{
	DiscoItem::Identity identity = { "client", ApplicationInfo::name(), "pc" };
	DiscoItem::Identities identities;
	identities += identity;

	d->capsRegistry->registerCaps(CapsSpec(ApplicationInfo::capsNode(),
	                                       ApplicationInfo::capsVersion(), ext),
	                              identities,
	                              Features(features));
}

void PsiCon::deinit()
{
	LOG_TRACE;

#ifdef YAPSI
	delete yaToasterCentral_;
	yaToasterCentral_ = 0;
#endif

	// this deletes all dialogs except for mainwin
	deleteAllDialogs();

	d->idle.stop();

	// shut down all accounts
	UserAccountList acc = d->contactList->getUserAccountList();
	delete d->contactList;

	// delete s5b server
	delete d->s5bServer;

#ifdef FILETRANSFER
	delete d->ftwin;
#endif

	if(d->mainwin) {
		delete d->mainwin;
		d->mainwin = 0;
	}

	// TuneController
	delete d->tuneController;

	// save profile
	d->saveProfile(acc);
	LOG_TRACE;

	GlobalShortcutManager::clear();
}

void PsiCon::optionsUpdate()
{
	// Global shortcuts
	setShortcuts();
}

void PsiCon::setShortcuts()
{
	// FIX-ME: GlobalShortcutManager::clear() is one big hack,
	// but people wanted to change global hotkeys without restarting in 0.11
	GlobalShortcutManager::clear();
	ShortcutManager::connect("global.event", this, SLOT(recvNextEvent()));
	ShortcutManager::connect("global.toggle-visibility", d->mainwin, SLOT(toggleVisible()));
	ShortcutManager::connect("global.bring-to-front", d->mainwin, SLOT(trayShow()));
	ShortcutManager::connect("global.new-blank-message", this, SLOT(doNewBlankMessage()));
#ifdef YAPSI
	ShortcutManager::connect("global.filter-contacts", d->mainwin, SLOT(filterContacts()));
#endif
}

PsiContactList* PsiCon::contactList() const
{
	return d->contactList;
}

EDB *PsiCon::edb() const
{
	return d->edb;
}

ProxyManager *PsiCon::proxy() const
{
	return d->proxy;
}

FileTransDlg *PsiCon::ftdlg() const
{
#ifdef FILETRANSFER
	return d->ftwin;
#else
	return 0;
#endif
}

TabManager *PsiCon::tabManager() const
{
	return d->tabManager;
}

TuneController *PsiCon::tuneController() const
{
	return d->tuneController;
}

void PsiCon::closeProgram()
{
	doQuit(QuitProgram);
}

void PsiCon::changeProfile()
{
	ActiveProfiles::instance()->unsetThisProfile();
	if(d->contactList->haveActiveAccounts()) {
		QMessageBox::information(0, CAP(tr("Error")), tr("Please disconnect before changing the profile."));
		return;
	}

	doQuit(QuitProfile);
}

void PsiCon::doManageAccounts()
{
	if (!PsiOptions::instance()->getOption("options.ui.account.single").toBool()) {
		AccountManageDlg *w = (AccountManageDlg *)dialogFind("AccountManageDlg");
		if(w)
			bringToFront(w);
		else {
			w = new AccountManageDlg(this);
			w->show();
		}
	}
	else {
		PsiAccount *account = d->contactList->defaultAccount();
		if(account) {
			account->modify();
		}
		else {
			promptUserToCreateAccount();
		}
	}
}

void PsiCon::doGroupChat()
{
	PsiAccount *account = d->contactList->defaultAccount();
	if(!account)
		return;

	MUCJoinDlg *w = new MUCJoinDlg(this, account);
	w->show();
}

void PsiCon::doNewBlankMessage()
{
	PsiAccount *account = d->contactList->defaultAccount();
	if(!account)
		return;

	EventDlg *w = createEventDlg("", account);
	if (!w)
		return;

	w->show();
}

// FIXME: smells fishy. Refactor! Probably create a common class for all dialogs and
// call optionsUpdate() automatically.
EventDlg *PsiCon::createEventDlg(const QString &to, PsiAccount *pa)
{
	if (!EventDlg::messagingEnabled())
		return 0;

	EventDlg *w = new EventDlg(to, this, pa);
	connect(this, SIGNAL(emitOptionsUpdate()), w, SLOT(optionsUpdate()));
	return w;
}

// FIXME: WTF? Refactor! Refactor!
void PsiCon::updateContactGlobal(PsiAccount *pa, const Jid &j)
{
	foreach(item_dialog* i, d->dialogList) {
		if(i->className == "EventDlg") {
			EventDlg *e = (EventDlg *)i->widget;
			if(e->psiAccount() == pa)
				e->updateContact(j);
		}
	}
}

// FIXME: make it work like QObject::findChildren<ChildName>()
QWidget *PsiCon::dialogFind(const char *className)
{
	foreach(item_dialog *i, d->dialogList) {
		// does the classname and jid match?
		if(i->className == className) {
			return i->widget;
		}
	}
	return 0;
}

QMenuBar* PsiCon::defaultMenuBar() const
{
	return d->defaultMenuBar;
}

void PsiCon::dialogRegister(QWidget *w)
{
	item_dialog *i = new item_dialog;
	i->widget = w;
	i->className = w->className();
	d->dialogList.append(i);
}

void PsiCon::dialogUnregister(QWidget *w)
{
	for (QList<item_dialog*>::Iterator it = d->dialogList.begin(); it != d->dialogList.end(); ) {
		item_dialog* i = *it;
		if(i->widget == w) {
			it = d->dialogList.erase(it);
			delete i;
		}
		else
			++it;
	}
}

void PsiCon::deleteAllDialogs()
{
	while(!d->dialogList.isEmpty()) {
		item_dialog* i = d->dialogList.takeFirst();
		delete i->widget;
		delete i;
	}
	d->tabManager->deleteAll();
}

AccountsComboBox *PsiCon::accountsComboBox(QWidget *parent, bool online_only)
{
	AccountsComboBox *acb = new AccountsComboBox(this, parent, online_only);
	return acb;
}

PsiAccount* PsiCon::createAccount(const QString &name, const Jid &j, const QString &pass, bool opt_host, const QString &host, int port, bool legacy_ssl_probe, UserAccount::SSLFlag ssl, int proxy, bool modify)
{
	return d->contactList->createAccount(name, j, pass, opt_host, host, port, legacy_ssl_probe, ssl, proxy, modify);
}

#ifdef YAPSI
PsiAccount* PsiCon::createAccount()
{
	PsiAccount* account = createAccount("Account", "", "",
	                                    false, "", 5222,
	                                    false, UserAccount::SSL_Auto, 0, false);
	Q_ASSERT(account);
	return account;

}
#endif

PsiAccount *PsiCon::createAccount(const UserAccount& acc)
{
	PsiAccount *pa = new PsiAccount(acc, d->contactList, d->capsRegistry, d->tabManager);
	connect(&d->idle, SIGNAL(secondsIdle(int)), pa, SLOT(secondsIdle(int)));
	connect(pa, SIGNAL(updatedActivity()), SLOT(pa_updatedActivity()));
	connect(pa, SIGNAL(updatedAccount()), SLOT(pa_updatedAccount()));
	connect(pa, SIGNAL(queueChanged()), SLOT(queueChanged()));
	connect(pa, SIGNAL(startBounce()), SLOT(startBounce()));
	if (d->s5bServer) {
		pa->client()->s5bManager()->setServer(d->s5bServer);
	}
	return pa;
}

void PsiCon::removeAccount(PsiAccount *pa)
{
	d->contactList->removeAccount(pa);
}

void PsiCon::statusMenuChanged(int x)
{
#ifndef YAPSI
	if(x == STATUS_OFFLINE && !option.askOffline) {
		setGlobalStatus(Status(Status::Offline, "Logged out", 0));
		if(option.useDock == true)
			d->mainwin->setTrayToolTip(Status(Status::Offline, "", 0));
	}
	else {
		if(x == STATUS_ONLINE && !option.askOnline) {
			setGlobalStatus(Status());
			if(option.useDock == true)
				d->mainwin->setTrayToolTip(Status());
		}
		else if(x == STATUS_INVISIBLE){
			Status s("","",0,true);
			s.setIsInvisible(true);
			setGlobalStatus(s);
			if(option.useDock == true)
				d->mainwin->setTrayToolTip(s);
		}
		else {
			// Create a dialog with the last status message
			StatusSetDlg *w = new StatusSetDlg(this, makeStatus(x, currentStatusMessage()));
			connect(w, SIGNAL(set(const XMPP::Status &, bool)), SLOT(setStatusFromDialog(const XMPP::Status &, bool)));
			connect(w, SIGNAL(cancelled()), SLOT(updateMainwinStatus()));
			if(option.useDock == true)
				connect(w, SIGNAL(set(const XMPP::Status &, bool)), d->mainwin, SLOT(setTrayToolTip(const XMPP::Status &, bool)));
			w->show();
		}
	}
#else
	setGlobalStatus(makeStatus(x, currentStatusMessage()));
#endif
}

XMPP::Status::Type PsiCon::currentStatusType() const
{
#ifdef YAPSI
	if (!d->mainwin)
		return XMPP::Status::Offline;
	return d->mainwin->statusType();
#else
	bool active = false;
	bool loggedIn = false;
	XMPP::Status::Type state = XMPP::Status::Online;
	foreach(PsiAccount* account, d->contactList->enabledAccounts()) {
		if(account->isActive())
			active = true;
		if(account->loggedIn()) {
			loggedIn = true;
			state = account->status()->type();
		}
	}

	if (!loggedIn) {
		state = XMPP::Status::Offline;
	}

	return state;
#endif
}

#ifdef YAPSI
XMPP::Status::Type PsiCon::lastLoggedInStatusType() const
{
	return d->mainwin->lastLoggedInStatusType();
}
#endif

QString PsiCon::currentStatusMessage() const
{
#ifdef YAPSI
	if (!d->mainwin)
		return QString();
	return d->mainwin->statusMessage();
#else
	return d->lastStatusString;
#endif
}

void PsiCon::setStatusFromDialog(const XMPP::Status &s, bool withPriority)
{
	d->lastStatusString = s.status();
	setGlobalStatus(s, withPriority);
}

void PsiCon::setGlobalStatus(const Status &s,  bool withPriority)
{
	PsiLogger::instance()->log(QString("%1 PsiCon()::setGlobalStatus(%3, %4)").arg(LOG_THIS)
	                           .arg(s.type())
	                           .arg(withPriority));

	// Check whether all accounts are logged off
	bool allOffline = true;
	foreach(PsiAccount* account, d->contactList->enabledAccounts()) {
		if ( account->isActive() ) {
			allOffline = false;
			break;
		}
	}

	// globally set each account which is logged in
	foreach(PsiAccount* account, d->contactList->enabledAccounts())
		if (allOffline || account->isActive())
			account->setStatus(s, withPriority);
}

void PsiCon::pa_updatedActivity()
{
	PsiAccount *pa = (PsiAccount *)sender();
	emit accountUpdated(pa);

	// update s5b server
	updateS5BServerAddresses();

	updateMainwinStatus();
}

void PsiCon::pa_updatedAccount()
{
	PsiAccount *pa = (PsiAccount *)sender();
	emit accountUpdated(pa);

	d->updatedAccountTimer_->start();
}

void PsiCon::saveAccounts()
{
	d->updatedAccountTimer_->stop();

	UserAccountList acc = d->contactList->getUserAccountList();

	d->pro.proxyList = d->proxy->itemList();
	//d->pro.acc = acc;
	//d->pro.toFile(pathToProfileConfig(activeProfile));
	d->saveProfile(acc);
}

void PsiCon::saveCapabilities()
{
	QFile file(ApplicationInfo::homeDir() + "/caps.xml");
	d->capsRegistry->save(file);
}

void PsiCon::updateMainwinStatus()
{
	bool active = false;
	bool loggedIn = false;
	int state = STATUS_ONLINE;
	foreach(PsiAccount* account, d->contactList->enabledAccounts()) {
		if(account->isActive())
			active = true;
		if(account->loggedIn()) {
			loggedIn = true;
			state = makeSTATUS(account->status());
		}
	}
	if(loggedIn)
		d->mainwin->decorateButton(state);
	else {
		if(active)
			d->mainwin->decorateButton(-1);
		else
			d->mainwin->decorateButton(STATUS_OFFLINE);
	}
}

void PsiCon::setToggles(bool tog_offline, bool tog_away, bool tog_agents, bool tog_hidden, bool tog_self)
{
	if(d->contactList->enabledAccounts().count() > 1)
		return;

	if (tog_offline == d->contactList->showOffline() &&
	    tog_agents  == d->contactList->showAgents() &&
	    tog_hidden  == d->contactList->showHidden() &&
	    tog_self    == d->contactList->showSelf())
	{
		return;
	}

	d->contactList->setShowOffline(tog_offline);
	// d->contactList->setShowAway(tog_away);
	Q_UNUSED(tog_away);
	d->contactList->setShowAgents(tog_agents);
	d->contactList->setShowHidden(tog_hidden);
	d->contactList->setShowSelf(tog_self);

	d->updatedAccountTimer_->start();
}

void PsiCon::getToggles(bool *tog_offline, bool *tog_away, bool *tog_agents, bool *tog_hidden, bool *tog_self)
{
	*tog_offline = d->contactList->showOffline();
	// *tog_away    = d->contactList->showAway();
	Q_UNUSED(tog_away);
	*tog_agents  = d->contactList->showAgents();
	*tog_hidden  = d->contactList->showHidden();
	*tog_self    = d->contactList->showSelf();
}

void PsiCon::doOptions()
{
#ifdef YAPSI
	Q_ASSERT(false);
#else
	OptionsDlg *w = (OptionsDlg *)dialogFind("OptionsDlg");
	if(w)
		bringToFront(w);
	else {
		w = new OptionsDlg(this, option);
		connect(w, SIGNAL(applyOptions(const Options &)), SLOT(slotApplyOptions(const Options &)));
		w->show();
	}
#endif
}

void PsiCon::doFileTransDlg()
{
#ifdef FILETRANSFER
	bringToFront(d->ftwin);
#endif
}

void PsiCon::checkAccountsEmpty()
{
	while (d->pro.acc.count() == 0) {
#ifndef YAPSI
		promptUserToCreateAccount();
#else
		// PsiAccount* account = createAccount("jabberout.com", "", "",
		//                                     true, "jabberout.com", 5222,
		//                                     false, UserAccount::SSL_Yes, 0, false);
		// Q_ASSERT(account);
		break;
#endif
	}
}

void PsiCon::doOpenUri(const QUrl &uriToOpen)
{/*
	QUrl uri(uriToOpen);	// got to copy, because setQueryDelimiters() is not const

	qWarning("uri:  " + uri.toString());

	// scheme

	if (uri.scheme() != "xmpp") {	// try handling legacy URIs
		QMessageBox::warning(0, tr("Warning"), QString("URI (link) type \"%1\" is unsupported.").arg(uri.scheme()));
	}

	// authority

	PsiAccount *pa = 0;
	if (uri.authority().isEmpty()) {
		pa = d->contactList->defaultAccount();
		if (!pa) {
			QMessageBox::warning(0, tr("Warning"), QString("You don't have any account enabled."));
		}
	}
	else {
		qWarning("uri auth: [" + uri.authority() + "]");

		Jid authJid = JIDUtil::fromString(uri.authority());
		foreach(PsiAccount* acc, d->contactList->enabledAccounts()) {
			if (acc->jid().compare(authJid, false)) {
				pa = acc;
			}
		}

		if (!pa) {
			foreach(PsiAccount* acc, d->contactList->accounts()) {
				if (acc->jid().compare(authJid, false)) {
					QMessageBox::warning(0, tr("Warning"), QString("The account for %1 JID is disabled right now.").arg(authJid.bare()));
					return;	// FIX-ME: Should suggest enabling it now
				}
			}
		}
		if (!pa) {
			QMessageBox::warning(0, tr("Warning"), QString("You don't have an account for %1.").arg(authJid.bare()));
			return;
		}
	}

	// entity

	QString path = uri.path();
	if (path.startsWith('/'))	// this happens when authority part is present
		path = path.mid(1);
	Jid entity = JIDUtil::fromString(path);

	// query

	uri.setQueryDelimiters('=', ';');

	QString querytype = uri.queryItems().value(0).first;	// defaults to empty string

	if (querytype == "message") {
		if (uri.queryItemValue("type") == "chat")
			pa->actionOpenChat(entity);
		else {
			pa->dj_newMessage(entity, uri.queryItemValue("body"), uri.queryItemValue("subject"), uri.queryItemValue("thread"));
		}
	}
	else if (querytype == "roster") {
		pa->openAddUserDlg(entity, uri.queryItemValue("name"), uri.queryItemValue("group"));
	}
	else if (querytype == "join") {
		pa->actionJoin(entity, uri.queryItemValue("password"));
	}
	else if (querytype == "vcard") {
		pa->actionInfo(entity);
	}
	else if (querytype == "disco") {
		pa->actionDisco(entity, uri.queryItemValue("node"));
	}
	else {
		pa->actionSendMessage(entity);
	}
*/}

void PsiCon::doToolbars()
{
	OptionsDlg *w = (OptionsDlg *)dialogFind("OptionsDlg");
	if (w) {
		w->openTab("toolbars");
		bringToFront(w);
	}
	else {
		w = new OptionsDlg(this, option);
		connect(w, SIGNAL(applyOptions(const Options &)), SLOT(slotApplyOptions(const Options &)));
		w->openTab("toolbars");
		w->show();
	}
}

void PsiCon::slotApplyOptions(const Options &opt)
{
	Options oldOpt = option;
	bool notifyRestart = true;

	option = opt;

#ifndef Q_WS_MAC
	if (option.hideMenubar) {
		// check if all toolbars are disabled
		bool toolbarsVisible = false;
		QList<Options::ToolbarPrefs>::ConstIterator it = option.toolbars["mainWin"].begin();
		for ( ; it != option.toolbars["mainWin"].end() && !toolbarsVisible; ++it) {
			toolbarsVisible = toolbarsVisible || (*it).on;
		}

		// Check whether it is legal to disable the menubar
		if ( !toolbarsVisible ) {
			QMessageBox::warning(0, tr("Warning"),
				tr("You can not disable <i>all</i> toolbars <i>and</i> the menubar. If you do so, you will be unable to enable them back, when you'll change your mind.\n"
					"<br><br>\n"
					"If you really-really want to disable all toolbars and the menubar, you need to edit the config.xml file by hand."),
				tr("I understand"));
			option.hideMenubar = false;
		}
	}
#endif

	if ( option.useTabs != oldOpt.useTabs ) {
		QMessageBox::information(0, tr("Information"), tr("Some of the options you changed will only have full effect upon restart."));
		notifyRestart = false;
	}

	// change icon set
	if ( option.systemIconset		!= oldOpt.systemIconset		||
	     option.emoticons			!= oldOpt.emoticons		||
	     option.defaultRosterIconset	!= oldOpt.defaultRosterIconset	||
	     operator!=(option.serviceRosterIconset,oldOpt.serviceRosterIconset)	||
	     operator!=(option.customRosterIconset,oldOpt.customRosterIconset) )
	{
		if ( notifyRestart && PsiIconset::instance()->optionsChanged(&oldOpt) )
			QMessageBox::information(0, tr("Information"), tr("The complete iconset update will happen on next Psi start."));

		// update icon selector
		d->updateIconSelect();

		// flush the QPixmapCache
		QPixmapCache::clear();
	}

	if ( oldOpt.alertStyle != option.alertStyle )
		alertIconUpdateAlertStyle();

#ifndef YAPSI
	d->mainwin->buildToolbars();
#endif

	/*// change pgp engine
	if(option.pgp != oldpgp) {
		if(d->pgp) {
			delete d->pgp;
			d->pgp = 0;
			pgpToggled(false);
		}
		pgp_init(option.pgp);
	}*/

	// update s5b
	if(oldOpt.dtPort != option.dtPort)
		s5b_init();
	updateS5BServerAddresses();

	// mainwin stuff
	d->mainwin->setWindowOpts(option.alwaysOnTop, (option.useDock && option.dockToolMW));
	d->mainwin->setUseDock(option.useDock);

	// notify about options change
	emitOptionsUpdate();

	// save just the options
	//d->pro.prefs = option;
	//d->pro.toFile(pathToProfileConfig(activeProfile));
	d->saveProfile(d->pro.acc);
}

void PsiCon::queueChanged()
{
	PsiIcon *nextAnim = 0;
	int nextAmount = d->contactList->queueCount();
	PsiAccount *pa = d->contactList->queueLowestEventId();
	if(pa)
		nextAnim = PsiIconset::instance()->event2icon(pa->eventQueue()->peekNext());

#ifdef Q_WS_MAC
	{
		// Update the event count
		MacDock::overlay(nextAmount ? QString::number(nextAmount) : QString());

		if (!nextAmount) {
			MacDock::stopBounce();
		}
	}
#endif

	d->mainwin->updateReadNext(nextAnim, nextAmount);
}

void PsiCon::startBounce()
{
#ifdef Q_WS_MAC
	if (option.bounceDock != Options::NoBounce) {
		MacDock::startBounce();
		if (option.bounceDock == Options::BounceOnce) {
			MacDock::stopBounce();
		}
	}
#endif
}

void PsiCon::recvNextEvent()
{
	/*printf("--- Queue Content: ---\n");
	PsiAccountListIt it(d->list);
	for(PsiAccount *pa; (pa = it.current()); ++it) {
		printf(" Account: [%s]\n", pa->name().latin1());
		pa->eventQueue()->printContent();
	}*/
	PsiAccount *pa = d->contactList->queueLowestEventId();
	if(pa)
		pa->openNextEvent(UserAction);
}

void PsiCon::playSound(const QString &str)
{
	if(str.isEmpty() || !useSound)
		return;

	soundPlay(str);
}

void PsiCon::raiseMainwin()
{
	d->mainwin->showNoFocus();
}

bool PsiCon::mainWinVisible() const
{
	return d->mainwin->isVisible();
}

const QStringList & PsiCon::recentGCList() const
{
	return d->recentGCList;
}

void PsiCon::recentGCAdd(const QString &str)
{
	// remove it if we have it
	for(QStringList::Iterator it = d->recentGCList.begin(); it != d->recentGCList.end(); ++it) {
		if(*it == str) {
			d->recentGCList.remove(it);
			break;
		}
	}

	// put it in the front
	d->recentGCList.prepend(str);

	// trim the list if bigger than 10
	while(d->recentGCList.count() > PsiOptions::instance()->getOption("options.muc.recent-joins.maximum").toInt())
		d->recentGCList.remove(d->recentGCList.fromLast());
}

const QStringList & PsiCon::recentBrowseList() const
{
	return d->recentBrowseList;
}

void PsiCon::recentBrowseAdd(const QString &str)
{
	// remove it if we have it
	for(QStringList::Iterator it = d->recentBrowseList.begin(); it != d->recentBrowseList.end(); ++it) {
		if(*it == str) {
			d->recentBrowseList.remove(it);
			break;
		}
	}

	// put it in the front
	d->recentBrowseList.prepend(str);

	// trim the list if bigger than 10
	while(d->recentBrowseList.count() > 10)
		d->recentBrowseList.remove(d->recentBrowseList.fromLast());
}

const QStringList & PsiCon::recentNodeList() const
{
	return d->recentNodeList;
}

void PsiCon::recentNodeAdd(const QString &str)
{
	// remove it if we have it
	for(QStringList::Iterator it = d->recentNodeList.begin(); it != d->recentNodeList.end(); ++it) {
		if(*it == str) {
			d->recentNodeList.remove(it);
			break;
		}
	}

	// put it in the front
	d->recentNodeList.prepend(str);

	// trim the list if bigger than 10
	while(d->recentNodeList.count() > 10)
		d->recentNodeList.remove(d->recentNodeList.fromLast());
}

void PsiCon::proxy_settingsChanged()
{
	// properly index accounts
	foreach(PsiAccount* account, d->contactList->accounts()) {
		UserAccount acc = account->userAccount();
		if(acc.proxy_index > 0) {
			int x = d->proxy->findOldIndex(acc.proxy_index-1);
			if(x == -1)
				acc.proxy_index = 0;
			else
				acc.proxy_index = x+1;
			account->setUserAccount(acc);
		}
	}

	saveAccounts();
}

IconSelectPopup *PsiCon::iconSelectPopup() const
{
	return d->iconSelect;
}

bool PsiCon::filterEvent(const PsiAccount* acc, const PsiEvent* e) const
{
#ifdef YAPSI
#if 0
	// TODO: buzzer is not crash-safe when receiving buzz messages at some
	// weird time. Closing buzzing chat could possible crash the application.
	if(e->type() == PsiEvent::Message) {
		const Message &m = ((MessageEvent *)e)->message();
		if (!m.body().isEmpty() && Ya::isBuzzMessage(m.body())) {
			QWidget* w = acc->findChatDialog(e->from());
			if (!w) {
				w = acc->findChatDialog(e->jid());
			}
			d->buzzer->setWidget(w && w->isVisible() ? w : d->mainwin);
			d->buzzer->buzz();
//			return true;
		}
	}
#endif
#endif
	return false;
}

void PsiCon::processEvent(PsiEvent *e, ActivationType activationType)
{
	if ( e->type() == PsiEvent::PGP ) {
		e->account()->eventQueue()->dequeue(e);
		e->account()->queueChanged();
		return;
	}

	if ( !e->account() )
		return;

	UserListItem *u = e->account()->find(e->jid());
	if ( !u ) {
		qWarning("SYSTEM MESSAGE: Bug #1. Contact the developers and tell them what you did to make this message appear. Thank you.");
		e->account()->eventQueue()->dequeue(e);
		e->account()->queueChanged();
		return;
	}

#ifdef FILETRANSFER
	if( e->type() == PsiEvent::File ) {
		FileEvent *fe = (FileEvent *)e;
		FileTransfer *ft = fe->takeFileTransfer();
		e->account()->eventQueue()->dequeue(e);
		e->account()->queueChanged();
		e->account()->cpUpdate(*u);
		if(ft) {
			FileRequestDlg *w = new FileRequestDlg(fe->timeStamp(), ft, e->account());
			bringToFront(w);
		}
		return;
	}
#endif

	bool isChat = false;
	bool sentToChatWindow = false;
	if ( e->type() == PsiEvent::Message ) {
		MessageEvent *me = (MessageEvent *)e;
		const Message &m = me->message();
		bool emptyForm = m.getForm().fields().empty();
		if ((m.type() == "chat" && emptyForm)
		    || !EventDlg::messagingEnabled()) {
			isChat = true;
			sentToChatWindow = me->sentToChatWindow();
		}
	}

	if (e->type() == PsiEvent::Auth && !EventDlg::messagingEnabled()) {
		if (dynamic_cast<AuthEvent*>(e)->authType() == "subscribe") {
#ifdef YAPSI
			bringToFront(d->mainwin);
			return;
#else
			e->account()->dj_addAuth(e->jid());
#endif
		}
		e->account()->eventQueue()->dequeue(e);
		delete e;
		return;
	}

#ifdef YAPSI
	if (e->type() == PsiEvent::Mood) {
		QPointer<PsiEvent> moodEvent = e;
		if (e->account()) {
			e->account()->actionOpenChat(e->jid());
			// Q_ASSERT(moodEvent.isNull());
			if (!moodEvent.isNull()) {
				moodEvent->account()->eventQueue()->dequeue(moodEvent);
				delete moodEvent;
			}
		}
		return;
	}
#endif

	if ( isChat ) {
		PsiAccount* account = e->account();
		XMPP::Jid from = e->from();

		if ( option.alertOpenChats && sentToChatWindow ) {
			// Message already displayed, need only to pop up chat dialog, so that
			// it will be read (or marked as read)
			ChatDlg *c = account->findChatDialog(from);
			if(!c)
				c = account->findChatDialog(e->jid());
			if(!c)
				return; // should never happen

			account->processChats(from); // this will delete all events, corresponding to that chat dialog
		}

		// as the event could be deleted just above, we're using cached account and from values
		account->openChat(from, activationType);
	}
	else {
		// search for an already opened eventdlg
		EventDlg *w = e->account()->findDialog<EventDlg*>(u->jid());

		if ( !w ) {
			// create the eventdlg
			w = e->account()->ensureEventDlg(u->jid());

			// load next message
			e->account()->processReadNext(*u);
		}

		if (w)
			bringToFront(w);
	}
}

void PsiCon::updateS5BServerAddresses()
{
	if(!d->s5bServer)
		return;

	QList<QHostAddress> list;

	// grab all IP addresses
	foreach(PsiAccount* account, d->contactList->accounts()) {
		QHostAddress *a = account->localAddress();
		if(!a)
			continue;

		// don't take dups
		bool found = false;
		for(QList<QHostAddress>::ConstIterator hit = list.begin(); hit != list.end(); ++hit) {
			const QHostAddress &ha = *hit;
			if(ha == (*a)) {
				found = true;
				break;
			}
		}
		if(!found)
			list += (*a);
	}

	// convert to stringlist
	QStringList slist;
	for(QList<QHostAddress>::ConstIterator hit = list.begin(); hit != list.end(); ++hit)
		slist += (*hit).toString();

	// add external
	if(!option.dtExternal.isEmpty()) {
		bool found = false;
		for(QStringList::ConstIterator sit = slist.begin(); sit != slist.end(); ++sit) {
			const QString &s = *sit;
			if(s == option.dtExternal) {
				found = true;
				break;
			}
		}
		if(!found)
			slist += option.dtExternal;
	}

	// set up the server
	d->s5bServer->setHostList(slist);
}

void PsiCon::s5b_init()
{
	if(d->s5bServer->isActive())
		d->s5bServer->stop();

#ifndef YAPSI
	if (option.dtPort) {
		if(!d->s5bServer->start(option.dtPort)) {
			QMessageBox::warning(0, tr("Warning"), tr("Unable to bind to port %1 for Data Transfer.\nThis may mean you are already running another instance of Psi. You may experience problems sending and/or receiving files.").arg(option.dtPort));
		}
	}
#endif
}

void PsiCon::doSleep()
{
	setGlobalStatus(Status(Status::Offline, tr("Computer went to sleep"), 0));
}

void PsiCon::doWakeup()
{
	PsiLogger::instance()->log(QString("%1 PsiCon(%2)::doWakeup()").arg(LOG_THIS));

	// TODO: Restore the status from before the log out. Make it an (hidden) option for people with a bad wireless connection.
	//setGlobalStatus(Status());

	foreach(PsiAccount* account, d->contactList->enabledAccounts()) {
		if (account->userAccount().opt_reconn) {
			// Should we do this when the network comes up ?
			account->setStatus(Status(lastLoggedInStatusType(), currentStatusMessage(), account->userAccount().priority));
		}
	}
}


QList<PsiToolBar*> PsiCon::toolbarList() const
{
#ifdef YAPSI
	QList<PsiToolBar*> list;
	return list;
#else
	return d->mainwin->toolbars;
#endif
}


PsiToolBar *PsiCon::findToolBar(QString group, int index)
{
#ifdef YAPSI
	Q_UNUSED(group);
	Q_UNUSED(index);
	return 0;
#else
	PsiToolBar *toolBar = 0;

	if (( group == "mainWin" ) && (index < d->mainwin->toolbars.size()))
		toolBar = d->mainwin->toolbars.at(index);

	return toolBar;
#endif
}

void PsiCon::buildToolbars()
{
#ifndef YAPSI
	d->mainwin->buildToolbars();
#endif
}

bool PsiCon::getToolbarLocation(Q3DockWindow* dw, Qt::Dock& dock, int& index, bool& nl, int& extraOffset) const
{
#ifdef YAPSI
	Q_UNUSED(dw);
	Q_UNUSED(dock);
	Q_UNUSED(index);
	Q_UNUSED(nl);
	Q_UNUSED(extraOffset);
	return false;
#else
	return d->mainwin->getLocation(dw, dock, index, nl, extraOffset);
#endif
}

PsiActionList *PsiCon::actionList() const
{
#ifdef YAPSI
	return 0;
#else
	return d->actionList;
#endif
}

/**
 * Prompts user to create new account, if none are currently present in system.
 */
void PsiCon::promptUserToCreateAccount()
{
	QMessageBox msgBox(QMessageBox::Question,tr("Account setup"),tr("You need to set up an account to start. Would you like to register a new account, or use an existing account?"));
	QPushButton *registerButton = msgBox.addButton(tr("Register new account"), QMessageBox::AcceptRole);
	QPushButton *existingButton = msgBox.addButton(tr("Use existing account"),QMessageBox::AcceptRole);
	msgBox.addButton(QMessageBox::Cancel);
	msgBox.exec();
	if (msgBox.clickedButton() ==  existingButton) {
		AccountModifyDlg w(this);
		w.exec();
	}
	else if (msgBox.clickedButton() ==  registerButton) {
		AccountRegDlg w(proxy());
		int n = w.exec();
		if (n == QDialog::Accepted) {
			contactList()->createAccount(w.jid().node(),w.jid(),w.pass(),w.useHost(),w.host(),w.port(),w.legacySSLProbe(),w.ssl(),w.proxy(),false);
		}
	}
}

QString PsiCon::optionsFile() const
{
	return pathToProfile(activeProfile) + "/options.xml";
}

void PsiCon::forceSavePreferences()
{
	slotApplyOptions(option);
	PsiOptions::instance()->save(optionsFile());
}

bool PsiCon::restoringSavedChats() const
{
	return d->restoringSavedChats;
}

void PsiCon::doQuit(int quitCode)
{
	d->quitting = true;
	emit quit(quitCode);
}

void PsiCon::aboutToQuit()
{
	doQuit(QuitProgram);
}

#ifdef YAPSI_ACTIVEX_SERVER
void PsiCon::setYaOnline(YaOnline* yaOnline)
{
	yaOnline_ = yaOnline;
}

YaOnline* PsiCon::yaOnline() const
{
	Q_ASSERT(yaOnline_);
	return yaOnline_;
}
#endif

#ifdef YAPSI
YaToasterCentral* PsiCon::yaToasterCentral() const
{
	return yaToasterCentral_;
}
#endif

#include "psicon.moc"
