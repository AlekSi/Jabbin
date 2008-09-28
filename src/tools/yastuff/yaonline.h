/*
 * yaonline.h - communication with running instance of Online
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

#ifndef YAONLINE_H
#define YAONLINE_H

#include <QObject>

class PsiAccount;
class PsiCon;
class PsiEvent;
class YaPsiServer;
class YcuApiWrapper;
class PsiContact;

#include "xmpp_message.h"
#include "xmpp_status.h"

class YaOnline;
class YaOnlineHelper
{
public:
	// it's better to use PsiCon::yaOnline() to get this, though
	static YaOnline* instance();
	friend class PsiCon;
	friend class PsiMain;
};

class YaOnline : public QObject
{
	Q_OBJECT
public:
	YaOnline(YaPsiServer* parent);
	~YaOnline();

	void notify(int id, PsiEvent* event);

	void openUrl(const QString& url, bool isYandex);
	void clearedMessageHistory();

	void updateActiveProfile();
	QString activeProfile() const;
	void setController(PsiCon* controller);

	static bool onlineIsRunning();
	XMPP::Status::Type lastStatus() const;

signals:
	void doShowRoster(bool visible);
	void forceStatus(XMPP::Status::Type statusType);
	void clearMoods();
	void showYapsiPreferences();

public slots:
	void doQuit();
	void showSidebar();
	void showPreferences();
	void setDND(bool isDND);
	void doShowIgnoredToasters();
	void setErrorMessage(const QString& error);
	void clearErrorMessage();
	void notifyAllUnshownEvents();

private slots:
	void onlineObjectChanged();
	void accountCountChanged();
	void lastMailNotify(const XMPP::Message&);
	void disconnectRereadSettingsAndReconnect();
	void doSetOfflineMode();
	void doSetDND(bool isDND);
	void doSoundsChanged();
	void doPingServer();
	void doToasterClicked(const QString& id);
	void doToasterScreenLocked(const QString& id);
	void doToasterIgnored(const QString& id);
	void doToasterSkipped(const QString& id);
	void doScreenUnlocked();
	// void doPlaySound(const QString& type);
	void doAuthAccept(const QString& id);
	void doAuthDecline(const QString& id);
	void doOnlineConnected();

	void ipcMessage(const QString& message);
	void checkForAliveOnlineProcess();

	void eventQueueChanged();

private:
	static YaOnline* instance_;
	YaPsiServer* server_;
	PsiCon* controller_;
	YcuApiWrapper* ycuApi_;
	XMPP::Status::Type lastStatus_;
	QString activeProfile_;

	PsiAccount* onlineAccount() const;
	void rereadSettings();
	void updateOnlineAccount();
	void updateProxySettings();
	void updateMiscSettings();
	void showToaster(const QString& type, PsiAccount* account, const XMPP::Jid& jid, const QString& message, const QDateTime& timeStamp, const QString& callbackId);
	QString avatarPath(PsiAccount* account, const XMPP::Jid& jid);
	QString contactName(PsiAccount* account, const XMPP::Jid& jid);
	QString contactGender(PsiAccount* account, const XMPP::Jid& jid);
	bool doToasterIgnored(PsiAccount* account, const XMPP::Jid& jid);

	friend class YaOnlineHelper;
};

#endif
