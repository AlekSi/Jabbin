/*
 * common.h - contains all the common variables and functions for Psi
 * Copyright (C) 2001-2003  Justin Karneges
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

#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QMap>
#include <QSize>
#include <QStringList>
#include <QList>
#include <QColor>

#include "statuspreset.h"
#include "phononsoundplayer.h"

class QSound;
class QTimer;
class QProcess;

// -----------------------------------------------------------------------------
// Options
// -----------------------------------------------------------------------------

enum {
	cOnline,
	cOffline,
	cAway,
	cDND,
	cProfileFore,
	cProfileBack,
	cGroupFore,
	cGroupBack,
	cListBack,
	cAnimFront,
	cAnimBack,
	cStatus,
	cNumColors // A guard to store the number of colors
};
enum { fRoster, fMessage, fChat, fPopup };
enum { eMessage, eChat1, eChat2, eHeadline, eSystem, eOnline, eOffline, eSend, eIncomingFT, eFTComplete };
enum { dcClose, dcHour, dcDay, dcNever };

struct Options
{
	QColor color[cNumColors];
	QString font[4];
	int smallFontSize; // do not modify or save/load this value! it is calculated at run time!
	int alertStyle;

	QString systemIconset;
	QStringList emoticons;
	QString defaultRosterIconset;
	QMap<QString, QString> serviceRosterIconset;
	QMap<QString, QString> customRosterIconset;

	bool useleft, singleclick, hideMenubar, askOnline, askOffline, popupMsgs, popupChats, popupHeadlines, popupFiles, raise;
	bool alwaysOnTop, noAwaySound, noAwayPopup, noUnlistedPopup, rosterAnim, autoVCardOnLogin, xmlConsoleOnLogin;
	bool useDock, dockDCstyle, dockHideMW, dockToolMW;
	bool smallChats, chatLineEdit, useTabs, putTabsAtBottom, usePerTabCloseButton, autoRosterSize, autoRosterSizeGrowTop, autoResolveNicksOnAdd, brushedMetal;

	bool autoCopy; // although this setting is ignored under linux,
	               // it is preserved in case user uses the same config file on different platforms
	bool useCaps;

	bool oldSmallChats; //Filthy hack, see chat code.
	int delChats, browser;

	bool useRC;

	int defaultAction;
	int incomingAs;
	QStringList recentStatus; //recent status messages
	QMap<QString,StatusPreset> sp; // Status message presets.

	int asAway, asXa, asOffline;
	bool use_asAway, use_asXa, use_asOffline;
	QString asMessage;
	QString onevent[10];

	// Added by Kiko 020621: points to the directory where the trusted
	// certificates used in validating the server's certificate are kept
	QString trustCertStoreDir;

	QString player;

	bool ignoreHeadline, ignoreNonRoster, excludeGroupChatsFromIgnore, scrollTo, keepSizes, useEmoticons, alertOpenChats;
	bool raiseChatWindow, showSubjects, showCounter, chatSays, showGroupCounts;

	QSize sizeEventDlg, sizeChatDlg, sizeTabDlg;
	bool jidComplete, grabUrls, noGCSound;

	struct ToolbarPrefs {
		bool dirty;

		QString name;
		bool on;
		bool locked;
		bool stretchable;
		QStringList keys;

		Qt::Dock dock;
		int index;
		bool nl;
		int extraOffset;
	};
	QMap< QString, QList<ToolbarPrefs> > toolbars;

	// groupchat highlighting/nick colouring
	bool gcHighlighting, gcNickColoring;
	QStringList gcHighlights, gcNickColors;

	bool clNewHeadings;
	bool outlineHeadings;

	// passive popups
	bool ppIsOn;
	bool ppMessage, ppHeadline, ppChat, ppCall, ppOnline, ppStatus, ppOffline, ppFile;
	int  ppJidClip, ppStatusClip, ppTextClip, ppHideTime;
	QColor ppBorderColor;

	// Bouncing of the dock (Mac OS X)
	typedef enum { NoBounce, BounceOnce, BounceForever } BounceDockSetting;
	BounceDockSetting bounceDock;

	struct {
		bool roster, services;
	} lockdown;

	bool useTransportIconsForContacts;

	// roster sorting styles
	typedef enum Roster_ContactSortStyle {
		ContactSortStyle_Status = 0,
		ContactSortStyle_Alpha
	};
	Roster_ContactSortStyle rosterContactSortStyle;

	typedef enum Roster_GroupSortStyle {
		GroupSortStyle_Alpha = 0,
		GroupSortStyle_Rank
	};
	Roster_GroupSortStyle rosterGroupSortStyle;

	typedef enum Roster_AccountSortStyle {
		AccountSortStyle_Alpha = 0,
		AccountSortStyle_Rank
	};
	Roster_AccountSortStyle rosterAccountSortStyle;

	bool discoItems, discoInfo;

	bool autoAuth, notifyAuth;

	// event priority
	enum { EventPriorityDontCare = -1 };
	int eventPriorityHeadline;
	int eventPriorityChat;
	int eventPriorityMessage;
	int eventPriorityAuth;
	int eventPriorityFile;
	int eventPriorityRosterExchange;

	// Message events
	bool messageEvents;
	bool inactiveEvents;

	int dtPort;
	QString dtExternal;

	// Last used path remembering
	QString lastPath;
	QString lastSavePath;

	//background images
	QString chatBgImage, rosterBgImage;
};

extern
Options option;

// -----------------------------------------------------------------------------
// Status
// -----------------------------------------------------------------------------

#include "xmpp_status.h"

#define STATUS_OFFLINE   XMPP::Status::Offline
#define STATUS_ONLINE    XMPP::Status::Online
#define STATUS_AWAY      XMPP::Status::Away
#define STATUS_XA        XMPP::Status::XA
#define STATUS_DND       XMPP::Status::DND
#define STATUS_INVISIBLE XMPP::Status::Invisible
#define STATUS_CHAT      XMPP::Status::FFC

#define STATUS_ASK	 100
#define STATUS_NOAUTH	 101
#define STATUS_ERROR	 102

void soundPlay(const QString &);

QString status2txt(int status);
XMPP::Status makeStatus(int, const QString &);
XMPP::Status makeStatus(int, const QString &, int);
XMPP::Status::Type makeSTATUS(const XMPP::Status &);
QString clipStatus(const QString &str, int width, int height);


// -----------------------------------------------------------------------------
// Widget tools
// -----------------------------------------------------------------------------

void bringToFront(QWidget *w, bool grabFocus = true);
void replaceWidget(QWidget *, QWidget *);
void closeDialogs(QWidget *);
#ifdef Q_WS_X11
#include <QWidget>
void x11wmClass(Display *dsp, WId wid, QString resName);
#define X11WM_CLASS(x)	x11wmClass(x11Display(), winId(), (x));
#else
#define X11WM_CLASS(x)	/* dummy */
#endif

// -----------------------------------------------------------------------------
// History utilities
// -----------------------------------------------------------------------------

QString logencode(QString);
QString logdecode(const QString &);


// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

QString CAP(const QString &str);

QString encodePassword(const QString &, const QString &);
QString decodePassword(const QString &, const QString &);

bool operator!=(const QMap<QString, QString> &, const QMap<QString, QString> &);

bool fileCopy(const QString &src, const QString &dest);

void soundPlay(const QString &);

extern Qt::WFlags psi_dialog_flags;
extern bool useSound;

/*
class SoundPlayer: public QObject {
Q_OBJECT
public:
    SoundPlayer(QObject * parent);
	virtual ~SoundPlayer();
    static SoundPlayer * instance();
    void playContinuosSound(QString file, int msec);
    void stop();

protected slots:
    void playNext();
    void play(QString file);

private:
    int msec;
    int interval;
    QString file;
    bool played;
    QTimer * timer;
    QSound * sound;
    QProcess * process;
};
*/

#endif
