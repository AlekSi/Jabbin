/*
 * psiapplication.h - subclass of QApplication to do some workarounds
 * Copyright (C) 2003  Michail Pishchagin
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

#ifndef PSIAPPLICATION_H
#define PSIAPPLICATION_H

#include <QApplication>
#ifdef Q_WS_MAC
#include <Carbon/Carbon.h>
#endif

class QEvent;

#ifdef Q_WS_WIN
#include <qt_windows.h>
#endif

class PsiApplication : public QApplication
{
	Q_OBJECT
public:
	PsiApplication(int &argc, char **argv, bool GUIenabled = true);
	~PsiApplication();

	// reimplemented
	bool event(QEvent* event);
	bool notify(QObject *receiver, QEvent *event);
#ifdef Q_WS_X11
	bool x11EventFilter(XEvent *event);
#endif
#ifdef Q_WS_MAC
	bool macEventFilter(EventHandlerCallRef, EventRef);
#endif
#ifdef Q_WS_WIN
	bool winEventFilter(MSG* msg, long* result);
#endif

	// reimplemented
	void commitData(QSessionManager& manager);

private slots:
	void appAboutToQuit();
	void appExiting(int);

signals:
	void forceSavePreferences();
	void dockActivated();
	void newTrayOwner();
	void trayOwnerDied();

private:
	void init(bool GUIenabled);
};

#endif
