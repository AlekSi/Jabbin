/*
 * yapsiserver.h - COM server
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

#ifndef YAPSISERVER_H
#define YAPSISERVER_H

#include <QObject>
#include <QPointer>
#include <QAxObject>

class PsiMain;
class YaOnline;
struct IDispatch;

class YaPsiServer : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("ClassID",     "{9701704f-7b96-49a7-9e7c-179c1b0bcd72}")
	Q_CLASSINFO("InterfaceID", "{b02d7f7c-d475-4f19-a1d4-e4df4e276186}")
	Q_CLASSINFO("EventsID",    "{6396e824-d832-4b90-9ff7-d7d2e6d645a6}")

public:
	YaPsiServer(QObject* parent = 0);
	~YaPsiServer();

	QVariant dynamicCall(const char* function, const QVariant& var1 = QVariant(), const QVariant& var2 = QVariant(), const QVariant& var3 = QVariant(), const QVariant& var4 = QVariant(), const QVariant& var5 = QVariant(), const QVariant& var6 = QVariant(), const QVariant& var7 = QVariant(), const QVariant& var8 = QVariant());

	// static YaPsiServer* instance();

signals:
	void onlineObjectChanged();
	void doShowRoster(bool visible);
	void disconnectRereadSettingsAndReconnect();
	void doSetOfflineMode();
	void doSetDND(bool isDND);
	void doSoundsChanged();
	void doPingServer();
	void doToasterClicked(const QString& id);
	void doToasterScreenLocked(const QString& id);
	void doToasterIgnored(const QString& id);
	void doToasterSkipped(const QString& id);
	void doShowIgnoredToasters();
	void doScreenUnlocked();
	void doPlaySound(const QString& type);
	void clearMoods();
	void doAuthAccept(const QString& id);
	void doAuthDecline(const QString& id);
	void doOnlineConnected();
	void showPreferences();

// Note: the following slots are intended for use by the COM/OLE clients **only**
public slots:
	void start(bool offlineMode);
	void shutdown();
	void showRoster();
	void hideRoster();
	void hideAllWindows();
	QString getBuildNumber();
	QString getUninstallPath();
	void setOfflineMode();
	void proxyChanged();
	void userChanged();
	void setDND(bool isDND);
	void soundsChanged();
	void onlineConnected();
	void onlineDisconnected();
	// void playSound(const QString& type);

	void showText(const QString& text);
	void setOnlineObject(IDispatch* obj);
	void toasterClicked(const QString& text);
	void toasterScreenLocked(const QString& id);
	void toasterIgnored(const QString& id);
	void toasterSkipped(const QString& id);
	void showIgnoredToasters();
	void screenUnlocked();
	void showProperties();

	void authAccept(const QString& id);
	void authDecline(const QString& id);

private:
	PsiMain* main_;
	YaOnline* yaOnline_;
	QPointer<QAxObject> onlineObject_;

	static YaPsiServer* instance_;

	void deinitProfile();
	void initProfile();
};

#endif
