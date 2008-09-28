/*
 * desktoputil.cpp - url-opening routines
 * Copyright (C) 2007  Maciej Niedzielski, Michail Pishchagin
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

#include "desktoputil.h"

#include <QDesktopServices>
#include <QUrl>
#include <QSettings>
#include <QFileInfo>
#include <QProcess>
#include <QSysInfo>

#ifdef YAPSI_ACTIVEX_SERVER
#include "yaonline.h"
#endif

#ifdef Q_WS_WIN
#include <windows.h>

QString defaultBrowser()
{
	QSettings settings("HKEY_CLASSES_ROOT\\HTTP\\shell\\open\\command", QSettings::NativeFormat);
	QString command = settings.value(".").toString();
	QRegExp rx("\"(.+)\"");
	if (rx.indexIn(command) != -1)
		return rx.capturedTexts()[1];
	return command;
}
#endif

static bool doOpenUrl(const QUrl& url)
{
#ifdef YAPSI_ACTIVEX_SERVER
	YaOnlineHelper::instance()->openUrl(QString(url.toEncoded()), false);
	return true;
#endif

#ifdef Q_WS_WIN
	// on Vista it always returns iexplore.exe as default browser
	bool oldStyleDefaultBrowserInfo = QSysInfo::WindowsVersion < QSysInfo::WV_VISTA;

	QFileInfo browserFileInfo(defaultBrowser());
	if (oldStyleDefaultBrowserInfo && browserFileInfo.fileName() == "iexplore.exe") {
		return QProcess::startDetached(browserFileInfo.absoluteFilePath(),
		                               QStringList() << "-new" << url.toEncoded());
	}
	else {
		// FIXME: This is necessary for Qt 4.3.3 to handle all URLs correctly
		ShellExecute(0, 0, (TCHAR *)QString(url.toEncoded()).utf16(), 0, 0, SW_SHOWNORMAL);
		return true;
	}
#endif
	return QDesktopServices::openUrl(url);
}

/**
 *	\brief Opens URL using OS default handler
 *	\param url the url to open
 *
 *	\a url may be either percent encoded or not.
 *	If it contains only ASCII characters, it is decoded,
 *	else it is converted to QUrl in QUrl::TolerantMode mode.
 *	Resulting QUrl object is passed to QDesktopServices::openUrl().
 *
 *	\sa QDesktopServices::openUrl()
 */
bool DesktopUtil::openUrl(const QString &url)
{
#ifdef YAPSI_ACTIVEX_SERVER
	// FIXME: IM-473 Workaround for Qt 4.3.5/Win
	// This appears to be fixed in Qt 4.4.2+
	YaOnlineHelper::instance()->openUrl(url, false);
	return true;
#endif

	QByteArray ascii = url.toAscii();
	if (ascii == url)
		return doOpenUrl(QUrl::fromEncoded(ascii));
	else
		return doOpenUrl(QUrl(url, QUrl::TolerantMode));
}

#ifdef YAPSI
bool DesktopUtil::openYaUrl(const QString& url)
{
#ifdef YAPSI_ACTIVEX_SERVER
	YaOnlineHelper::instance()->openUrl(url, true);
	return true;
#else
	return DesktopUtil::openUrl(url);
#endif
}
#endif
