/*
 * yapsiserver.cpp - COM server
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

#include "yapsiserver.h"

#include <QApplication>
#include <QAxFactory>
#include <QPointer>
#include <QSettings>
#include <QDir>

QAXFACTORY_BEGIN("{6801583f-e09f-49be-a2bf-ed06fa3aad76}",
                 "{30265966-7950-4438-b601-af5d9eb29f9e}")
	QAXCLASS(YaPsiServer)
QAXFACTORY_END()

#include <QMessageBox>

void log(QString message)
{
	QMessageBox::information(0, "YaPsiServer",
	                         message);
}

#include "psicon.h"
#include "main.h"
#include "yaonline.h"
#include "jabbinpsi_revision.h" // auto-generated file, see src.pro for details
#include "psilogger.h"

YaPsiServer* YaPsiServer::instance_ = 0;

YaPsiServer::YaPsiServer(QObject* parent)
	: QObject(parent)
	, main_(0)
	, yaOnline_(0)
{
	LOG_TRACE;
	if (instance_) {
		LOG_TRACE;
		// log("YaPsiServer already instantiated!");
		delete instance_;
		instance_ = 0;
	}

	instance_ = this;
	LOG_TRACE;
}

YaPsiServer::~YaPsiServer()
{
	delete yaOnline_;
	delete main_;
	instance_ = 0;
}

void YaPsiServer::start(bool offlineMode)
{
	LOG_TRACE;
	if (main_) {
		return;
	}

	LOG_TRACE;
	if (!yaOnline_) {
		yaOnline_ = new YaOnline(this);
	}

	LOG_TRACE;
	initProfile();
	LOG_TRACE;

	if (offlineMode) {
		setOfflineMode();
	}
	LOG_TRACE;
}

void YaPsiServer::deinitProfile()
{
	LOG_TRACE;
	if (main_) {
		Q_ASSERT(yaOnline_);
		yaOnline_->setController(0);
		delete main_;
		main_ = 0;
	}
	LOG_TRACE;
}

void YaPsiServer::initProfile()
{
	LOG_TRACE;
	if (main_ || !yaOnline_)
		return;

	LOG_TRACE;
	main_ = new PsiMain();
	LOG_TRACE;
	qApp->processEvents();
	LOG_TRACE;
	yaOnline_->setController(main_->controller());
	LOG_TRACE;

	emit disconnectRereadSettingsAndReconnect();
	LOG_TRACE;
}

void YaPsiServer::showText(const QString& text)
{
	log(text);
}

void YaPsiServer::toasterClicked(const QString& id)
{
	emit doToasterClicked(id);
}

void YaPsiServer::toasterScreenLocked(const QString& id)
{
	emit doToasterScreenLocked(id);
}

void YaPsiServer::toasterIgnored(const QString& id)
{
	emit doToasterIgnored(id);
}

void YaPsiServer::toasterSkipped(const QString& id)
{
	emit doToasterSkipped(id);
}

void YaPsiServer::showIgnoredToasters()
{
	emit doShowIgnoredToasters();
}

void YaPsiServer::screenUnlocked()
{
	emit doScreenUnlocked();
}

void YaPsiServer::showProperties()
{
	emit showPreferences();
}

void YaPsiServer::setOnlineObject(IDispatch* obj)
{
	if (!onlineObject_.isNull()) {
		onlineObject_->deleteLater();
		onlineObject_ = 0;
	}

	if (obj) {
		onlineObject_ = new QAxObject(reinterpret_cast<IUnknown*>(obj), this);
	}

	emit onlineObjectChanged();
}

QVariant YaPsiServer::dynamicCall(const char* function, const QVariant& var1, const QVariant& var2, const QVariant& var3, const QVariant& var4, const QVariant& var5, const QVariant& var6, const QVariant& var7, const QVariant& var8)
{
	if (!onlineObject_.isNull()) {
		return onlineObject_->dynamicCall(function, var1, var2, var3, var4, var5, var6, var7, var8);
	}
	return QVariant();
}

// YaPsiServer* YaPsiServer::instance()
// {
// 	return instance_;
// }

void YaPsiServer::shutdown()
{
	PsiLogger::instance()->log("YaPsiServer::shutdown()");
	QPointer<QApplication> app(qApp);
	deinitProfile();

	if (!app.isNull()) {
		qApp->quit();
	}
}

void YaPsiServer::showRoster()
{
	emit doShowRoster(true);
}

void YaPsiServer::hideRoster()
{
	emit doShowRoster(false);
}

void YaPsiServer::hideAllWindows()
{
	foreach(QWidget* w, qApp->topLevelWidgets()) {
		w->hide();
	}
}

QString YaPsiServer::getBuildNumber()
{
	return QString::number(JABBINPSI_REVISION);
}

QString YaPsiServer::getUninstallPath()
{
	QSettings sUser(QSettings::UserScope, "Jabberout", "Jabbin");
	QDir programDir(sUser.value("ProgramDir").toString());
	QStringList filters;
	filters << "unins*.exe";
	QStringList unins = programDir.entryList(filters, QDir::NoFilter, QDir::Time);
	if (unins.isEmpty())
		return QString();
	return programDir.absoluteFilePath(unins.first());
}

void YaPsiServer::setOfflineMode()
{
	emit doSetOfflineMode();
}

void YaPsiServer::proxyChanged()
{
	emit disconnectRereadSettingsAndReconnect();
}

void YaPsiServer::userChanged()
{
	LOG_TRACE;
	bool doShowRoster = main_ && main_->controller() && main_->controller()->mainWinVisible();

	deinitProfile();
	if (yaOnline_)
		yaOnline_->updateActiveProfile();
	initProfile();

	if (doShowRoster)
		showRoster();
}

void YaPsiServer::setDND(bool isDND)
{
	emit doSetDND(isDND);
}

void YaPsiServer::soundsChanged()
{
	emit doSoundsChanged();
}

void YaPsiServer::onlineConnected()
{
	if (!yaOnline_)
		return;

	// we need to ensure that chat is / should be connected to server now
	emit doOnlineConnected();

	emit doPingServer();
}

void YaPsiServer::onlineDisconnected()
{
	emit doPingServer();
}

// void YaPsiServer::playSound(const QString& type)
// {
// 	emit doPlaySound(type);
// }

void YaPsiServer::authAccept(const QString& id)
{
	emit doAuthAccept(id);
}

void YaPsiServer::authDecline(const QString& id)
{
	emit doAuthDecline(id);
}
