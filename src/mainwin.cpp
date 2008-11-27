/*
 * mainwin.cpp - the main window.  holds contactlist and buttons.
 * Copyright (C) 2001-2003  Justin Karneges, Michail Pishchagin
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

#include "mainwin.h"

#include <QDesktopServices>
#include <qmessagebox.h>
#include <qicon.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qobject.h>
#include <qpainter.h>
#include <qsignalmapper.h>
#include <qmenubar.h>
#include <QPixmap>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QMenu>
#include <QMenuItem>

#ifdef Q_WS_WIN
#include <windows.h>
#endif

#include "common.h"
#include "showtextdlg.h"
#include "psicon.h"
#include "psiiconset.h"
#include "serverinfomanager.h"
#include "applicationinfo.h"
#include "psiaccount.h"
#include "psitrayicon.h"
#include "psitoolbar.h"
#include "aboutdlg.h"
#include "psitoolbar.h"
#include "psipopup.h"
#include "psioptions.h"
#include "tipdlg.h"
#include "mucjoindlg.h"
#include "psicontactlist.h"
#include "contactlistmodel.h"
#include "contactlistview.h"
#include "tools/customwidgets/calldialog.h"

#include "mainwin_p.h"

using namespace XMPP;

//----------------------------------------------------------------------------
// MainWin::Private
//----------------------------------------------------------------------------

class MainWin::Private
{
public:
	Private(PsiCon *, MainWin *);
	~Private();

	ContactListView *cvlist;
	ContactListModel *contactListModel;
	QVBoxLayout *vb_main;
	bool onTop, asTool;
	QMenu *mainMenu, *statusMenu, *optionsMenu, *toolsMenu;
	int sbState;
	QString nickname;
	PsiTrayIcon *tray;
	QMenu *trayMenu;
	QString statusTip;

	PopupAction *optionsButton, *statusButton;
	IconActionGroup *statusGroup;
	EventNotifierAction *eventNotifier;
	PsiCon *psi;
	MainWin *mainWin;

	QLineEdit *searchText;
	QToolButton *searchPb;
	QWidget *searchWidget;

	QSignalMapper *statusMapper;

	PsiIcon *nextAnim;
	int nextAmount;

	QMap<QAction *, int> statusActions;

	int lastStatus;
	bool old_trayicon;
	bool filterActive, prefilterShowOffline, prefilterShowAway;

	void registerActions();
	IconAction *getAction( QString name );
	void updateMenu(QStringList actions, QMenu *menu);
};

MainWin::Private::Private(PsiCon *_psi, MainWin *_mainWin) : psi(_psi), mainWin(_mainWin)
{

	statusGroup   = (IconActionGroup *)getAction("status_all");
	eventNotifier = (EventNotifierAction *)getAction("event_notifier");

	optionsButton = (PopupAction *)getAction("button_options");
	statusButton  = (PopupAction *)getAction("button_status");

	statusMapper = new QSignalMapper(mainWin, "statusMapper");
	mainWin->connect(statusMapper, SIGNAL(mapped(int)), mainWin, SLOT(activatedStatusAction(int)));

	filterActive = false;
	prefilterShowOffline = false;
	prefilterShowAway = false;
}

MainWin::Private::~Private()
{
}

void MainWin::Private::registerActions()
{
	struct {
		const char *name;
		int id;
	} statuslist[] = {
		{ "status_chat",      STATUS_CHAT      },
		{ "status_online",    STATUS_ONLINE    },
		{ "status_away",      STATUS_AWAY      },
		{ "status_xa",        STATUS_XA        },
		{ "status_dnd",       STATUS_DND       },
		{ "status_invisible", STATUS_INVISIBLE },
		{ "status_offline",   STATUS_OFFLINE   },
		{ "", 0 }
	};

	int i;
	QString aName;
	for ( i = 0; !(aName = QString(statuslist[i].name)).isEmpty(); i++ ) {
		IconAction *action = getAction( aName );
		connect (action, SIGNAL(activated()), statusMapper, SLOT(map()));

		statusMapper->setMapping(action, statuslist[i].id);
		statusActions[action] = statuslist[i].id;
	}

	// register all actions
	PsiActionList::ActionsType type = PsiActionList::ActionsType( PsiActionList::Actions_MainWin | PsiActionList::Actions_Common );
	ActionList actions = psi->actionList()->suitableActions( type );
	QStringList names = actions.actions();
	QStringList::Iterator it = names.begin();
	for ( ; it != names.end(); ++it ) {
		IconAction *action = actions.action( *it );
		if ( action ) {
			mainWin->registerAction( action );
		}
	}
}

IconAction *MainWin::Private::getAction( QString name )
{
	PsiActionList::ActionsType type = PsiActionList::ActionsType( PsiActionList::Actions_MainWin | PsiActionList::Actions_Common );
	ActionList actions = psi->actionList()->suitableActions( type );
	IconAction *action = actions.action( name );

	if ( !action ) {
		qWarning("MainWin::Private::getAction(): action %s not found!", name.latin1());
	}
	//else
	//	mainWin->registerAction( action );

	return action;
}

void MainWin::Private::updateMenu(QStringList actions, QMenu *menu)
{
	menu->clear();

	IconAction *action;
	foreach (QString name, actions) {
		// workind around Qt/X11 bug, which displays
		// actions's text and the separator bar in Qt 4.1.1
		if ( name == "separator" ) {
			menu->insertSeparator();
			continue;
		}

		if ( (action = getAction(name)) ) {
			action->addTo(menu);
		}
	}
}

//----------------------------------------------------------------------------
// MainWin
//----------------------------------------------------------------------------

//#ifdef Q_WS_X11
//#define TOOLW_FLAGS WStyle_Customize
//#else
//#define TOOLW_FLAGS ((Qt::WFlags) 0)
//#endif

#ifdef Q_WS_WIN
#define TOOLW_FLAGS (Qt::WindowMinimizeButtonHint)
#else
#define TOOLW_FLAGS ((Qt::WFlags) 0)
#endif

MainWin::MainWin(bool _onTop, bool _asTool, PsiCon *psi, const char *name)
:AdvancedWidget<Q3MainWindow>(0, (_onTop ? Qt::WStyle_StaysOnTop : Qt::Widget) | (_asTool ? (Qt::WStyle_Tool |TOOLW_FLAGS) : Qt::Widget))
//: Q3MainWindow(0,name,(_onTop ? Qt::WStyle_StaysOnTop : Qt::Widget) | (_asTool ? (Qt::WStyle_Tool |TOOLW_FLAGS) : Qt::Widget))
{
	setObjectName(name);
	setAttribute(Qt::WA_AlwaysShowToolTips);
  	if ( option.brushedMetal ) {
		setAttribute(Qt::WA_MacMetalStyle);
	}
	d = new Private(psi, this);

	setWindowIcon(PsiIconset::instance()->status(STATUS_OFFLINE).impix());

	d->onTop = _onTop;
	d->asTool = _asTool;

	// sbState:
	//   -1 : connect
	// >= 0 : STATUS_*
	d->sbState = STATUS_OFFLINE;
	d->lastStatus = -2;

	d->nextAmount = 0;
	d->nextAnim = 0;
	d->tray = 0;
	d->trayMenu = 0;
	d->statusTip = "";
	d->nickname = "";
#ifdef Q_WS_MAC
	d->old_trayicon = false;
#else
	d->old_trayicon = PsiOptions::instance()->getOption("options.ui.systemtray.use-old").toBool();
#endif

	QWidget *center = new QWidget (this, "Central widget");
	setCentralWidget ( center );

	d->vb_main = new QVBoxLayout(center);

	d->contactListModel = new ContactListModel(psi->contactList());
	CallDialog::contactList = psi->contactList();
	d->cvlist = new ContactListView(center);
	d->cvlist->setModel(d->contactListModel);

	int layoutMargin = 2;
#ifdef Q_WS_MAC
	layoutMargin = 0;
#endif
	d->vb_main->setMargin(layoutMargin);
	d->vb_main->setSpacing(layoutMargin);

	// create search bar
	d->searchWidget = new QWidget(centralWidget());
	d->vb_main->addWidget(d->searchWidget);
	QHBoxLayout* searchLayout = new QHBoxLayout(d->searchWidget);
	searchLayout->setMargin(0);
	searchLayout->setSpacing(0);
	d->searchText = new QLineEdit(d->searchWidget);
	connect(d->searchText,SIGNAL(textEdited(const QString&)),SLOT(searchTextEntered(const QString&)));
	searchLayout->addWidget(d->searchText);
	d->searchPb = new QToolButton(d->searchWidget);
	d->searchPb->setText("X");
	connect(d->searchPb,SIGNAL(clicked()),SLOT(searchClearClicked()));
	// connect(cvlist, SIGNAL(searchInput(const QString&)), SLOT(searchTextStarted(const QString&)));
	searchLayout->addWidget(d->searchPb);
	d->searchWidget->setVisible(false);
	//add contact view
	d->vb_main->addWidget(d->cvlist);

#ifdef Q_WS_MAC
	// Disable the empty vertical scrollbar:
	// it's here because of weird code in q3scrollview.cpp
	// Q3ScrollView::updateScrollBars() around line 877
	d->vb_main->addSpacing(4);
#endif

	d->statusMenu = new QMenu(this);
	d->optionsMenu = new QMenu(this);
#ifdef Q_WS_MAC
	d->trayMenu = d->statusMenu;
#else
	d->trayMenu = new QMenu(this);
	buildTrayMenu();
	connect(d->trayMenu, SIGNAL(aboutToShow()), SLOT(buildTrayMenu()));
#endif


	buildStatusMenu();
	buildOptionsMenu();
	connect(d->optionsMenu, SIGNAL(aboutToShow()), SLOT(buildOptionsMenu()));


	X11WM_CLASS("main");

	connect(d->psi, SIGNAL(accountCountChanged()), SLOT(numAccountsChanged()));
	numAccountsChanged();

	updateCaption();

	d->registerActions();

	connect(d->psi->contactList(), SIGNAL(accountFeaturesChanged()), SLOT(accountFeaturesChanged()));
	accountFeaturesChanged();

	decorateButton(STATUS_OFFLINE);

	// Mac-only menus
#ifdef Q_WS_MAC
	QMenu *mainMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("Menu"), mainMenu);
	d->getAction("menu_options")->addTo(mainMenu);
	d->getAction("menu_quit")->addTo(mainMenu);
	d->getAction("help_about")->addTo(mainMenu);
	d->getAction("help_about_qt")->addTo(mainMenu);

	d->mainMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("General"), d->mainMenu);
	connect(d->mainMenu, SIGNAL(aboutToShow()), SLOT(buildMainMenu()));
#else
	mainMenuBar()->insertItem(tr("General"), d->optionsMenu);
#endif

	mainMenuBar()->insertItem(tr("Status"), d->statusMenu);

	QMenu *viewMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("View"), viewMenu);
	d->getAction("show_offline")->addTo(viewMenu);
	if (PsiOptions::instance()->getOption("options.ui.menu.view.show-away").toBool()) {
		d->getAction("show_away")->addTo(viewMenu);
	}
	d->getAction("show_hidden")->addTo(viewMenu);
	d->getAction("show_agents")->addTo(viewMenu);
	d->getAction("show_self")->addTo(viewMenu);
	viewMenu->insertSeparator();
	d->getAction("show_statusmsg")->addTo(viewMenu);
	d->getAction("show_icons")->addTo(viewMenu);
	d->getAction("show_largeicons")->addTo(viewMenu);

	// Mac-only menus
#ifdef Q_WS_MAC
	d->toolsMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("Tools"), d->toolsMenu);
	connect(d->toolsMenu, SIGNAL(aboutToShow()), SLOT(buildToolsMenu()));

	QMenu *helpMenu = new QMenu(this);
	mainMenuBar()->insertItem(tr("Help"), helpMenu);
	d->getAction("help_readme")->addTo (helpMenu);
	d->getAction("help_tip")->addTo (helpMenu);
	helpMenu->insertSeparator();
	d->getAction("help_online_help")->addTo (helpMenu);
	d->getAction("help_online_wiki")->addTo (helpMenu);
	d->getAction("help_online_home")->addTo (helpMenu);
	d->getAction("help_psi_muc")->addTo (helpMenu);
	d->getAction("help_report_bug")->addTo (helpMenu);
#else
	if (option.hideMenubar)  {
		mainMenuBar()->hide();
	}
	//else
	//	mainMenuBar()->show();
#endif
	d->optionsButton->setMenu( d->optionsMenu );
	d->statusButton->setMenu( d->statusMenu );

	buildToolbars();

	setWindowOpacity(double(qMax(MINIMUM_OPACITY,PsiOptions::instance()->getOption("options.ui.contactlist.opacity").toInt()))/100);

	connect(qApp, SIGNAL(dockActivated()), SLOT(dockActivated()));

	connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionsUpdate()));
}


MainWin::~MainWin()
{
	PsiPopup::deleteAll();

	if(d->tray) {
		delete d->tray;
		d->tray = 0;
	}

	//saveToolbarsPositions();
	// need to find some workaround to case, when you're logging off. in that case
	// toobars are all disabled, and when you start psi again you need to enable
	// your toolbars

	delete d;
}

void MainWin::registerAction( IconAction *action )
{
	char activated[]  = SIGNAL( activated() );
	char toggled[]    = SIGNAL( toggled(bool) );
	char setChecked[] = SLOT( setChecked(bool) );

	struct {
		const char *name;
		const char *signal;
		QObject *receiver;
		const char *slot;
	} actionlist[] = {
		{ "show_offline",   toggled, d->psi->contactList(), SLOT( setShowOffline(bool) ) },
		{ "show_away",      toggled, d->psi->contactList(), SLOT( setShowAway(bool) ) },
		{ "show_hidden",    toggled, d->psi->contactList(), SLOT( setShowHidden(bool) ) },
		{ "show_agents",    toggled, d->psi->contactList(), SLOT( setShowAgents(bool) ) },
		{ "show_self",      toggled, d->psi->contactList(), SLOT( setShowSelf(bool) ) },
		{ "show_statusmsg", toggled, d->cvlist,             SLOT( setShowStatus(bool) ) },
		{ "show_icons",     toggled, d->cvlist,             SLOT( setShowIcons(bool) ) },
		{ "show_largeicons",toggled, d->cvlist,             SLOT( setLargeIcons(bool) ) },

		{ "button_options", activated, this, SIGNAL( doOptions() ) },

		{ "menu_disco",       SIGNAL( activated(PsiAccount *, int) ), this, SLOT( activatedAccOption(PsiAccount*, int) ) },
		{ "menu_add_contact", SIGNAL( activated(PsiAccount *, int) ), this, SLOT( activatedAccOption(PsiAccount*, int) ) },
		{ "menu_xml_console", SIGNAL( activated(PsiAccount *, int) ), this, SLOT( activatedAccOption(PsiAccount*, int) ) },

		{ "menu_new_message",    activated, this, SIGNAL( blankMessage() ) },
		{ "menu_join_groupchat", activated, this, SIGNAL( doGroupChat() ) },
		{ "menu_account_setup",  activated, this, SIGNAL( doManageAccounts() ) },
		{ "menu_options",        activated, this, SIGNAL( doOptions() ) },
		{ "menu_file_transfer",  activated, this, SIGNAL( doFileTransDlg() ) },
		{ "menu_toolbars",       activated, this, SIGNAL( doToolbars() ) },
		{ "menu_change_profile", activated, this, SIGNAL( changeProfile() ) },
		{ "menu_quit",           activated, this, SLOT( try2tryCloseProgram() ) },
		{ "menu_play_sounds",    toggled,   this, SLOT( actPlaySoundsActivated(bool) ) },
#ifdef USE_PEP
		{ "publish_tune",        toggled,   this, SLOT( actPublishTuneActivated(bool) ) },
#endif

		{ "event_notifier", SIGNAL( clicked(int) ), this, SLOT( statusClicked(int) ) },
		{ "event_notifier", activated, this, SLOT( doRecvNextEvent() ) },

		{ "help_readme",      activated, this, SLOT( actReadmeActivated() ) },
		{ "help_tip",         activated, this, SLOT( actTipActivated() ) },
		{ "help_online_help", activated, this, SLOT( actOnlineHelpActivated() ) },
		{ "help_online_wiki", activated, this, SLOT( actOnlineWikiActivated() ) },
		{ "help_online_home", activated, this, SLOT( actOnlineHomeActivated() ) },
		{ "help_psi_muc",     activated, this, SLOT( actJoinPsiMUCActivated() ) },
		{ "help_report_bug",  activated, this, SLOT( actBugReportActivated() ) },
		{ "help_about",       activated, this, SLOT( actAboutActivated() ) },
		{ "help_about_qt",    activated, this, SLOT( actAboutQtActivated() ) },

		{ "", 0, 0, 0 }
	};

	int i;
	QString aName;
	for ( i = 0; !(aName = QString(actionlist[i].name)).isEmpty(); i++ ) {
		if ( aName == action->name() ) {
#ifdef USE_PEP
			// Check before connecting, otherwise we get a loop
			if ( aName == "publish_tune") {
				action->setChecked( PsiOptions::instance()->getOption("options.extended-presence.tune.publish").toBool() );
			}
#endif

			disconnect( action, actionlist[i].signal, actionlist[i].receiver, actionlist[i].slot ); // for safety
			connect( action, actionlist[i].signal, actionlist[i].receiver, actionlist[i].slot );

			// special cases
			if ( aName == "menu_play_sounds" ) {
				action->setChecked( useSound );
			}
			//else if ( aName == "foobar" )
			//	;
		}
	}

	struct {
		const char *name;
		QObject *sender;
		const char *signal;
		const char *slot;
	} reverseactionlist[] = {
		{ "show_away",      d->psi->contactList(), SIGNAL( showAway(bool) ), setChecked },
		{ "show_hidden",    d->psi->contactList(), SIGNAL( showHiddenChanged(bool) ), setChecked },
		{ "show_offline",   d->psi->contactList(), SIGNAL( showOfflineChanged(bool) ), setChecked },
		{ "show_self",      d->psi->contactList(), SIGNAL( showSelfChanged(bool) ), setChecked },
		{ "show_agents",    d->psi->contactList(), SIGNAL( showAgentsChanged(bool) ), setChecked },
		{ "show_statusmsg", d->cvlist,             SIGNAL( showStatusChanged(bool) ), setChecked },
		{ "show_icons",     d->cvlist,             SIGNAL( showIconsChanged(bool) ), setChecked },
		{ "show_largeicons",d->cvlist,             SIGNAL( largeIconsChanged(bool) ), setChecked },
		{ "", 0, 0, 0 }
	};

	for ( i = 0; !(aName = QString(reverseactionlist[i].name)).isEmpty(); i++ ) {
		if ( aName == action->name() ) {
			disconnect( reverseactionlist[i].sender, reverseactionlist[i].signal, action, reverseactionlist[i].slot ); // for safety
			connect( reverseactionlist[i].sender, reverseactionlist[i].signal, action, reverseactionlist[i].slot );

			if (aName == "show_statusmsg") {
				action->setChecked( PsiOptions::instance()->getOption("options.ui.contactlist.status-messages.show").toBool() );
			}
			else
				action->setChecked( true );
		}
	}
}

PsiCon *MainWin::psiCon() const
{
	return d->psi;
}

void MainWin::setWindowOpts(bool _onTop, bool _asTool)
{
	if(_onTop == d->onTop && _asTool == d->asTool) {
		return;
	}

	d->onTop = _onTop;
	d->asTool = _asTool;

	Qt::WFlags flags = 0;
	if(d->onTop) {
		flags |= Qt::WStyle_StaysOnTop;
	}
	if(d->asTool) {
		flags |= Qt::WStyle_Tool | TOOLW_FLAGS;
	}

	QPoint p = pos();
	reparent(parentWidget(), flags, p, FALSE);
	move(p);
	show();
}

void MainWin::setUseDock(bool use)
{
	if(use == false || d->tray) {
		if(d->tray) {
			delete d->tray;
			d->tray = 0;
		}

		if (use == false) {
			return;
		}
	}

	if(d->tray) {
		return;
	}

	d->tray = new PsiTrayIcon("Psi", d->trayMenu, d->old_trayicon);
	if (d->old_trayicon) {
		connect(d->tray, SIGNAL(closed()), SLOT(dockActivated()));
		connect(qApp, SIGNAL(trayOwnerDied()), SLOT(dockActivated()));
	}
	connect(d->tray, SIGNAL(clicked(const QPoint &, int)), SLOT(trayClicked(const QPoint &, int)));
	connect(d->tray, SIGNAL(doubleClicked(const QPoint &)), SLOT(trayDoubleClicked()));
	d->tray->setIcon( PsiIconset::instance()->statusPtr( STATUS_OFFLINE ));
	d->tray->setToolTip(ApplicationInfo::name());

	updateReadNext(d->nextAnim, d->nextAmount);

	d->tray->show();
}

void MainWin::buildStatusMenu()
{
	d->statusMenu->clear();
	d->getAction("status_online")->addTo(d->statusMenu);
	if (PsiOptions::instance()->getOption("options.ui.menu.status.chat").toBool()) {
		d->getAction("status_chat")->addTo(d->statusMenu);
	}
	d->statusMenu->insertSeparator();
	d->getAction("status_away")->addTo(d->statusMenu);
	if (PsiOptions::instance()->getOption("options.ui.menu.status.xa").toBool()) {
		d->getAction("status_xa")->addTo(d->statusMenu);
	}
	d->getAction("status_dnd")->addTo(d->statusMenu);
	if (PsiOptions::instance()->getOption("options.ui.menu.status.invisible").toBool()) {
		d->statusMenu->insertSeparator();
		d->getAction("status_invisible")->addTo(d->statusMenu);
	}
	d->statusMenu->insertSeparator();
	d->getAction("status_offline")->addTo(d->statusMenu);
#ifdef USE_PEP
	d->statusMenu->insertSeparator();
	d->getAction("publish_tune")->addTo(d->statusMenu);
#endif
}

void MainWin::activatedStatusAction(int id)
{
	QObjectList l = d->statusGroup->queryList( "IconAction" );
	for (QObjectList::Iterator it = l.begin() ; it != l.end(); ++it) {
		IconAction *action = (IconAction *)(*it);
		action->setChecked ( d->statusActions[action] == id );
	}

	statusChanged(id);
}

QMenuBar* MainWin::mainMenuBar() const
{
#ifdef Q_WS_MAC
	return psiCon()->defaultMenuBar();
#else
	return menuBar();
#endif
}

void MainWin::buildToolbars()
{
	while ( option.toolbars["mainWin"].count() < toolbars.count() && toolbars.count() ) {
		PsiToolBar *tb = toolbars.last();
		toolbars.removeLast();
		delete tb;
	}

	for (int i = 0; i < option.toolbars["mainWin"].count(); i++) {
		PsiToolBar *tb = 0;
		if ( i < toolbars.count() ) {
			tb = toolbars.at(i);
		}

		Options::ToolbarPrefs &tbPref = option.toolbars["mainWin"][i];
		if ( tb && !tbPref.dirty ) {
			continue;
		}

		if ( tb ) {
			delete tb;
		}

		tb = new PsiToolBar(tbPref.name, this, d->psi);
		moveDockWindow ( tb, tbPref.dock, tbPref.nl, tbPref.index, tbPref. extraOffset );

		tb->setGroup( "mainWin", i );
		tb->setType( PsiActionList::Actions_MainWin );
		//connect( tb, SIGNAL( registerAction( IconAction * ) ), SLOT( registerAction( IconAction * ) ) );
		tb->initialize( tbPref, false );

		if ( i < toolbars.count() ) {
			toolbars.removeAt(i);
		}
		toolbars.insert(i, tb);
	}
}

void MainWin::saveToolbarsPositions()
{
	for (int i = 0; i < toolbars.count(); i++) {
		Options::ToolbarPrefs &tbPref = option.toolbars["mainWin"][i];
		getLocation ( toolbars.at(i), tbPref.dock, tbPref.index, tbPref.nl, tbPref.extraOffset );
		tbPref.on = toolbars.at(i)->isVisible();
	}
}

bool MainWin::showDockMenu(const QPoint &)
{
	return false;
}

void MainWin::buildOptionsMenu()
{
	// help menu
	QMenu *helpMenu = new QMenu(d->optionsMenu);

	QStringList actions;
	actions << "help_readme"
	        << "help_tip"
	        << "separator"
	        << "help_online_help"
	        << "help_online_wiki"
	        << "help_online_home"
	        << "help_psi_muc"
	        << "help_report_bug"
	        << "separator"
	        << "help_about"
	        << "help_about_qt";

	d->updateMenu(actions, helpMenu);

	buildGeneralMenu( d->optionsMenu );

	d->optionsMenu->insertSeparator();
	d->optionsMenu->insertItem(IconsetFactory::icon("psi/help").icon(), tr("&Help"), helpMenu);
	d->getAction("menu_quit")->addTo( d->optionsMenu );

}

void MainWin::buildMainMenu()
{
	// main menu
	QStringList actions;
	actions << "menu_add_contact";
	if (PsiOptions::instance()->getOption("options.ui.message.enabled").toBool()) {
		actions << "menu_new_message";
	}
	actions << "menu_disco"
	        << "menu_join_groupchat"
	        << "separator"
	        << "menu_account_setup";
	if (PsiOptions::instance()->getOption("options.ui.menu.main.change-profile").toBool()) {
	        actions << "menu_change_profile";
	}
	actions << "menu_play_sounds";

	d->updateMenu(actions, d->mainMenu);
}

void MainWin::buildToolsMenu()
{
	QStringList actions;
	actions << "menu_file_transfer"
	        << "separator"
	        << "menu_xml_console";

	d->updateMenu(actions, d->toolsMenu);
}

void MainWin::buildGeneralMenu(QMenu *menu)
{
	// options menu
	QStringList actions;
	actions << "menu_add_contact";
	if (PsiOptions::instance()->getOption("options.ui.message.enabled").toBool()) {
		actions << "menu_new_message";
	}
	actions << "menu_disco"
	        << "menu_join_groupchat"
	        << "menu_account_setup"
	        << "menu_options"
	        << "menu_file_transfer";
	if (PsiOptions::instance()->getOption("options.ui.menu.main.change-profile").toBool()) {
	        actions << "menu_change_profile";
	}
	actions << "menu_play_sounds";

	d->updateMenu(actions, menu);
}

void MainWin::actReadmeActivated ()
{
	ShowTextDlg *w = new ShowTextDlg(":/README");
	w->setWindowTitle(CAP(tr("ReadMe")));
	w->show();
}

void MainWin::actOnlineHelpActivated ()
{
	QDesktopServices::openUrl(QUrl("http://psi-im.org/wiki/User_Guide"));
}

void MainWin::actOnlineWikiActivated ()
{
	QDesktopServices::openUrl(QUrl("http://psi-im.org/wiki"));
}

void MainWin::actOnlineHomeActivated ()
{
	QDesktopServices::openUrl(QUrl("http://psi-im.org"));
}

void MainWin::actJoinPsiMUCActivated()
{
	PsiAccount *account = d->psi->contactList()->defaultAccount();
	if(!account) {
		return;
	}

	MUCJoinDlg *w = new MUCJoinDlg(d->psi, account);
	w->le_host->setText("conference.psi-im.org");
	w->le_room->setText("psi");
	w->show();
}

void MainWin::actBugReportActivated ()
{
	QDesktopServices::openUrl(QUrl("http://psi-im.org/forum/forum/2"));
}

void MainWin::actAboutActivated ()
{
	AboutDlg *about = new AboutDlg();
	about->show();
}

void MainWin::actTipActivated ()
{
	TipDlg::show(d->psi);
}

void MainWin::actAboutQtActivated ()
{
	QMessageBox::aboutQt(this);
}

void MainWin::actPlaySoundsActivated (bool state)
{
	useSound = state;
}

void MainWin::actPublishTuneActivated (bool state)
{
	PsiOptions::instance()->setOption("options.extended-presence.tune.publish",state);
}

void MainWin::activatedAccOption(PsiAccount *pa, int x)
{
	if(x == 0) {
		pa->openAddUserDlg();
	}
	else if(x == 2) {
		pa->showXmlConsole();
	}
	else if(x == 3) {
		pa->doDisco();
	}
}

void MainWin::buildTrayMenu()
{
#ifndef Q_WS_MAC
	d->trayMenu->clear();

	if(d->nextAmount > 0) {
		d->trayMenu->insertItem(tr("Receive next event"), this, SLOT(doRecvNextEvent()));
		d->trayMenu->insertSeparator();
	}

	if(isHidden()) {
		d->trayMenu->insertItem(tr("Un&hide"), this, SLOT(trayShow()));
	}
	else {
		d->trayMenu->insertItem(tr("&Hide"), this, SLOT(trayHide()));
	}
	d->optionsButton->addTo(d->trayMenu);
	d->trayMenu->insertItem(tr("Status"), d->statusMenu);

	d->trayMenu->insertSeparator();
	// TODO!
	d->getAction("menu_quit")->addTo(d->trayMenu);
#endif
}

void MainWin::setTrayToolTip(int status)
{
	if (!d->tray) {
		return;
	}
	d->tray->setToolTip(QString("Psi - " + status2txt(status)));
}

void MainWin::decorateButton(int status)
{
	// update the 'change status' buttons
	QObjectList l = d->statusGroup->queryList( "IconAction" );
	for (QObjectList::Iterator it = l.begin() ; it != l.end(); ++it) {
		IconAction *action = (IconAction *)(*it);
		action->setChecked ( d->statusActions[action] == status );
	}

	if(d->lastStatus == status) {
		return;
	}
	d->lastStatus = status;

	if(status == -1) {
		d->statusButton->setText(tr("Connecting"));
		if (option.alertStyle != 0) {
			d->statusButton->setAlert(IconsetFactory::iconPtr("psi/connect"));
			d->statusGroup->setPsiIcon(IconsetFactory::iconPtr("psi/connect"));
		}
		else {
			d->statusButton->setIcon(PsiIconset::instance()->statusPtr(STATUS_OFFLINE));
			d->statusGroup->setPsiIcon(PsiIconset::instance()->statusPtr(STATUS_OFFLINE));
		}

		setWindowIcon(PsiIconset::instance()->status(STATUS_OFFLINE).impix());
	}
	else {
		d->statusButton->setText(status2txt(status));
		d->statusButton->setIcon(PsiIconset::instance()->statusPtr(status));
		d->statusGroup->setPsiIcon(PsiIconset::instance()->statusPtr(status));

		setWindowIcon(PsiIconset::instance()->status(status).impix());
	}

	updateTray();
}

bool MainWin::askQuit()
{
	return TRUE;
}

void MainWin::try2tryCloseProgram()
{
	QTimer::singleShot(0, this, SLOT(tryCloseProgram()));
}

void MainWin::tryCloseProgram()
{
	if(askQuit()) {
		closeProgram();
	}
}

void MainWin::closeEvent(QCloseEvent *e)
{
#ifdef Q_WS_MAC
	trayHide();
	e->accept();
#else
	if(d->tray) {
		trayHide();
		e->accept();
		return;
	}

	if(!askQuit()) {
		return;
	}

	closeProgram();

	e->accept();
#endif
}

void MainWin::keyPressEvent(QKeyEvent *e)
{
#ifdef Q_WS_MAC
	bool allowed = true;
#else
	bool allowed = d->tray ? true: false;
#endif

	bool closekey = false;
	if(e->key() == Qt::Key_Escape) {
		closekey = true;
	}
#ifdef Q_WS_MAC
	else if(e->key() == Qt::Key_W && e->modifiers() & Qt::ControlModifier) {
		closekey = true;
	}
#endif

	if(allowed && closekey) {
		close();
		e->accept();
		return;
	}

	QWidget::keyPressEvent(e);
}

#ifdef Q_WS_WIN
#include <windows.h>
bool MainWin::winEvent(MSG *msg, long *result)
{
	if (d->asTool && msg->message == WM_SYSCOMMAND && msg->wParam == SC_MINIMIZE) {
		hide();	// minimized toolwindows look bad on Windows, so let's just hide it instead
			// plus we cannot do this in changeEvent(), because it's called too late
		*result = 0;
		return true;	// don't let Qt process this event
	}
	return false;
}
#endif

void MainWin::updateCaption()
{
	QString str = "";

	if(d->nextAmount > 0) {
		str += "* ";
	}

	if(d->nickname.isEmpty()) {
		str += ApplicationInfo::name();
	}
	else {
		str += d->nickname;
	}

	if(str == caption()) {
		return;
	}

	setWindowTitle(str);
}

void MainWin::optionsUpdate()
{
	// d->cvlist->optionsUpdate();

	int status = d->lastStatus;
	d->lastStatus = -2;
	decorateButton(status);

#ifndef Q_WS_MAC
	if (option.hideMenubar) {
		mainMenuBar()->hide();
	}
	else {
		mainMenuBar()->show();
	}
#endif

	setWindowOpacity(double(qMax(MINIMUM_OPACITY,PsiOptions::instance()->getOption("options.ui.contactlist.opacity").toInt()))/100);

	buildStatusMenu();

	updateTray();
}

void MainWin::toggleVisible()
{
	if(!isHidden()) {
		trayHide();
	}
	else {
		trayShow();
	}
}

void MainWin::setTrayToolTip(const Status &status, bool)
{
	if (!d->tray) {
		return;
	}
	QString s = "Psi";

 	QString show = status.show();
	if(!show.isEmpty()) {
		show[0] = show[0].upper();
		s += " - "+show;
	}

	QString text = status.status();
	if(!text.isEmpty()) {
		s += ": "+text;
	}

	d->tray->setToolTip(s);
}

void MainWin::trayClicked(const QPoint &, int button)
{
	if(option.dockDCstyle) {
		return;
	}

	if(button == Qt::MidButton) {
		doRecvNextEvent();
		return;
	}

	if(!isHidden()) {
		trayHide();
	}
	else {
		trayShow();
	}
}

void MainWin::trayDoubleClicked()
{
	if(!option.dockDCstyle) {
		return;
	}

	if(d->nextAmount > 0) {
		doRecvNextEvent();
		return;
	}


	if(!isHidden()) {
		trayHide();
	}
	else {
		trayShow();
	}
}

void MainWin::trayShow()
{
	bringToFront(this);
}

void MainWin::trayHide()
{
	hide();
}

void MainWin::updateReadNext(PsiIcon *anim, int amount)
{
	d->nextAnim = anim;
	if(anim == 0) {
		d->nextAmount = 0;
	}
	else {
		d->nextAmount = amount;
	}

	if(d->nextAmount <= 0) {
		d->eventNotifier->hide();
		d->eventNotifier->setText("");
	}
	else {
		d->eventNotifier->setText(QString("<b>") + numEventsString(d->nextAmount) + "</b>");
		d->eventNotifier->show();
		// make sure it shows
		//qApp->processEvents();
	}

	updateTray();
	updateCaption();
}

QString MainWin::numEventsString(int x) const
{
	QString s;
	if(x <= 0) {
		s = "";
	}
	else if(x == 1) {
		s = tr("1 event received");
	}
	else {
		s = tr("%1 events received").arg(x);
	}

	return s;
}

void MainWin::updateTray()
{
	if(!d->tray) {
		return;
	}

	if ( d->nextAmount > 0 ) {
		d->tray->setAlert(d->nextAnim);
	}
	else if ( d->lastStatus == -1 ) {
		d->tray->setAlert(IconsetFactory::iconPtr("psi/connect"));
	}
	else {
		d->tray->setIcon(PsiIconset::instance()->statusPtr(d->lastStatus));
	}

	buildTrayMenu();
	d->tray->setContextMenu(d->trayMenu);
}

void MainWin::doRecvNextEvent()
{
	recvNextEvent();
}

void MainWin::statusClicked(int x)
{
	if(x == Qt::MidButton) {
		recvNextEvent();
	}
}

void MainWin::numAccountsChanged()
{
	d->statusButton->setEnabled(d->psi->contactList()->haveEnabledAccounts());
}

void MainWin::accountFeaturesChanged()
{
	bool have_pep = false;
	foreach(PsiAccount* account, d->psi->contactList()->enabledAccounts()) {
		if (account->serverInfoManager()->hasPEP()) {
			have_pep = true;
			break;
		}
	}

#ifdef USE_PEP
	d->getAction("publish_tune")->setEnabled(have_pep);
#endif
}

void MainWin::dockActivated()
{
	if(isHidden()) {
		show();
	}
}

/**
 * Called when the cancel is clicked or the search becomes empty.
 * Cancels the search.
 */
void MainWin::searchClearClicked()
{
	d->searchWidget->setVisible(false);
	d->searchText->clear();
	// cvlist->clearFilter();
	if (d->filterActive)
	{
		d->getAction("show_offline")->setChecked(d->prefilterShowOffline);
		d->getAction("show_away")->setChecked(d->prefilterShowAway);
	}
	d->filterActive=false;
}

/**
 * Called when the contactview has a keypress.
 * Starts the search/filter process
 */
void MainWin::searchTextStarted(QString const &text)
{
	d->searchWidget->setVisible(true);
	d->searchText->setText(d->searchText->text() + text);
	searchTextEntered(d->searchText->text());
	d->searchText->setFocus();
}

/**
 * Called when the search input is changed.
 * Updates the search.
 */
void MainWin::searchTextEntered(QString const &text)
{
	if (!d->filterActive)
	{
		d->filterActive = true;
		d->prefilterShowOffline = d->getAction("show_offline")->isChecked();
		d->prefilterShowAway = d->getAction("show_away")->isChecked();
		d->getAction("show_offline")->setChecked(true);
		d->getAction("show_away")->setChecked(true);
	}
	if (text.isEmpty()) {
		searchClearClicked();
	} else {

		// cvlist->setFilter(text);
	}
}

#ifdef Q_WS_MAC
void MainWin::setWindowIcon(const QPixmap&)
{
}
#else
void MainWin::setWindowIcon(const QPixmap& p)
{
	Q3MainWindow::setWindowIcon(p);
}
#endif

#if 0
#if defined(Q_WS_WIN)
#include <windows.h>
void MainWin::showNoFocus()
{
	clearWState( WState_ForceHide );

	if ( testWState(WState_Visible) ) {
		SetWindowPos(winId(),HWND_TOPMOST,0,0,0,0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
		if(!d->onTop)
			SetWindowPos(winId(),HWND_NOTOPMOST,0,0,0,0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE);
		return; // nothing to do
	}

	if ( isTopLevel() && !testWState( WState_Resized ) )  {
		// do this before sending the posted resize events. Otherwise
		// the layout would catch the resize event and may expand the
		// minimum size.
		QSize s = sizeHint();
		QSizePolicy::ExpandData exp;
#ifndef QT_NO_LAYOUT
		if ( layout() ) {
			if ( layout()->hasHeightForWidth() )
				s.setHeight( layout()->totalHeightForWidth( s.width() ) );
			exp =  layout()->expanding();
		} else
#endif
		{
			if ( sizePolicy().hasHeightForWidth() )
				s.setHeight( heightForWidth( s.width() ) );
			exp = sizePolicy().expanding();
		}
		if ( exp & QSizePolicy::Horizontally )
			s.setWidth( QMAX( s.width(), 200 ) );
		if ( exp & QSizePolicy::Vertically )
			s.setHeight( QMAX( s.height(), 150 ) );
		QRect screen = QApplication::desktop()->screenGeometry( QApplication::desktop()->screenNumber( pos() ) );
		s.setWidth( QMIN( s.width(), screen.width()*2/3 ) );
		s.setHeight( QMIN( s.height(), screen.height()*2/3 ) );
		if ( !s.isEmpty() )
			resize( s );
	}

	QApplication::sendPostedEvents( this, QEvent::Move );
	QApplication::sendPostedEvents( this, QEvent::Resize );

	setWState( WState_Visible );

	if ( testWFlags(Qt::WStyle_Tool) || isPopup() ) {
		raise();
	} else if ( testWFlags(Qt::WType_TopLevel) ) {
		while ( QApplication::activePopupWidget() )
		QApplication::activePopupWidget()->close();
	}

	if ( !testWState(WState_Polished) )
		polish();

	if ( children() ) {
		QObjectListIt it(*children());
		register QObject *object;
		QWidget *widget;
		while ( it ) {				// show all widget children
			object = it.current();		//   (except popups and other toplevels)
			++it;
			if ( object->isWidgetType() ) {
				widget = (QWidget*)object;
				if ( !widget->isHidden() && !widget->isTopLevel() )
				widget->show();
			}
		}
	}

#if defined(QT_ACCESSIBILITY_SUPPORT)
	QAccessible::updateAccessibility( this, 0, QAccessible::ObjectShow );
#endif

	SetWindowPos(winId(),HWND_TOP,0,0,0,0, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
	UpdateWindow(winId());
}

#else
#endif
#endif

void MainWin::showNoFocus()
{
	bringToFront(this);
}

//#endif
