/*
 * advwidget.cpp - AdvancedWidget template class
 * Copyright (C) 2005-2007  Michail Pishchagin
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <QMetaType>
#include <qglobal.h>
#ifdef Q_OS_WIN
#if __GNUC__ >= 3
#	define WINVER 0x0500
#endif
#include <windows.h>
#include <winuser.h>
#endif

#include "advwidget.h"

#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QDesktopWidget>
#include <QVariant>

#define USE_PSIOPTIONS
#define USE_YAVISUALUTIL

#ifdef USE_PSIOPTIONS
#include "psioptions.h"
#endif

#ifdef USE_YAVISUALUTIL
#include "yavisualutil.h"
#endif

#ifdef Q_WS_X11
#include<X11/Xutil.h>
#include <QX11Info>
#endif

// TODO: Make use of KDE taskbar flashing support

//----------------------------------------------------------------------------
// AdvancedWidgetShared
//----------------------------------------------------------------------------

class AdvancedWidgetShared : public QObject
{
	Q_OBJECT
public:
	AdvancedWidgetShared();
	~AdvancedWidgetShared();
};

AdvancedWidgetShared::AdvancedWidgetShared()
	: QObject(qApp)
{
}

AdvancedWidgetShared::~AdvancedWidgetShared()
{
}

static AdvancedWidgetShared *advancedWidgetShared = 0;

//----------------------------------------------------------------------------
// GAdvancedWidget::Private
//----------------------------------------------------------------------------

class GAdvancedWidget::Private : public QObject
{
	Q_OBJECT
public:
	Private(GAdvancedWidget* gAdv, QWidget* parent);

	static int  stickAt;
	static bool stickToWindows;
	static bool stickEnabled;

	GAdvancedWidget* gAdvancedWidget_;
	QWidget* parentWidget_;
	bool flashing_;
	QString geometryOptionPath_;

	bool flashing() const;
	void doFlash(bool on);
	void posChanging(int *x, int *y, int *width, int *height);
	void moveEvent(QMoveEvent *e);

	void restoreGeometry(QRect savedGeometry);

protected:
	// reimplemented
	bool eventFilter(QObject* obj, QEvent* e);

private:
	QTimer* saveGeometryTimer_;
	QRect newGeometry_;

public slots:
	void saveGeometry();
	void restoreGeometry();

private slots:
	void updateGeometry();
};

int  GAdvancedWidget::Private::stickAt        = 5;
bool GAdvancedWidget::Private::stickToWindows = true;
bool GAdvancedWidget::Private::stickEnabled   = true;

GAdvancedWidget::Private::Private(GAdvancedWidget* gAdv, QWidget* parent)
	: QObject(parent)
	, gAdvancedWidget_(gAdv)
	, parentWidget_(parent)
	, flashing_(false)
{
	if (!advancedWidgetShared)
		advancedWidgetShared = new AdvancedWidgetShared();

	parentWidget_ = parent;

	saveGeometryTimer_ = new QTimer(this);
	saveGeometryTimer_->setInterval(100);
	saveGeometryTimer_->setSingleShot(true);
	connect(saveGeometryTimer_, SIGNAL(timeout()), SLOT(saveGeometry()));
}

void GAdvancedWidget::Private::posChanging(int *x, int *y, int *width, int *height)
{
	if ( stickAt <= 0                     ||
	    !stickEnabled                     ||
	    !parentWidget_->isTopLevel()      ||
	     parentWidget_->isMaximized()     ||
	    !parentWidget_->updatesEnabled() )
	{
		return;
	}

	QWidget *p = parentWidget_;
	if ( p->pos() == QPoint(*x, *y) &&
	     p->frameSize() == QSize(*width, *height) )
		return;

	bool resizing = p->frameSize() != QSize(*width, *height);

	QDesktopWidget *desktop = qApp->desktop();
	QWidgetList list;

	if ( stickToWindows )
		list = QApplication::topLevelWidgets();

	foreach(QWidget *w, list) {
		QRect rect;
		bool dockWidget = false;

		if ( w->windowType() == Qt::Desktop )
			rect = ((QDesktopWidget *)w)->availableGeometry((QWidget *)parent());
		else {
			if ( w == p ||
			     desktop->screenNumber(p) != desktop->screenNumber(w) )
				continue;

			if ( !w->isVisible() )
				continue;

			// we want for widget to stick to outer edges of another widget, so
			// we'll change the rect to what it'll snap
			rect = QRect(w->frameGeometry().bottomRight(), w->frameGeometry().topLeft());
			dockWidget = true;
		}

		if ( *x != p->x() )
		if ( *x <= rect.left() + stickAt &&
		     *x >  rect.left() - stickAt ) {
			if ( !dockWidget ||
			     (p->frameGeometry().bottom() >= rect.bottom() &&
			      p->frameGeometry().top() <= rect.top()) ) {
				*x = rect.left();
				if ( resizing )
					*width = p->frameSize().width() + p->x() - *x;
			}
		}

		if ( *x + *width >= rect.right() - stickAt &&
		     *x + *width <= rect.right() + stickAt ) {
			if ( !dockWidget ||
			     (p->frameGeometry().bottom() >= rect.bottom() &&
			      p->frameGeometry().top() <= rect.top()) ) {
				if ( resizing )
					*width = rect.right() - *x + 1;
				else
					*x = rect.right() - *width + 1;
			}
		}

		if ( *y != p->y() )
		if ( *y <= rect.top() + stickAt &&
		     *y >  rect.top() - stickAt ) {
			if ( !dockWidget ||
			     (p->frameGeometry().right() >= rect.right() &&
			      p->frameGeometry().left() <= rect.left()) ) {
				*y = rect.top();
				if ( resizing )
					*height = p->frameSize().height() + p->y() - *y;
			}
		}

		if ( *y + *height >= rect.bottom() - stickAt &&
		     *y + *height <= rect.bottom() + stickAt ) {
			if ( !dockWidget ||
			     (p->frameGeometry().right() >= rect.right() &&
			      p->frameGeometry().left() <= rect.left()) ) {
				if ( resizing )
					*height = rect.bottom() - *y + 1;
				else
					*y = rect.bottom() - *height + 1;
			}
		}
	}
}

bool GAdvancedWidget::Private::flashing() const
{
	return flashing_;
}

void GAdvancedWidget::Private::doFlash(bool yes)
{
	flashing_ = yes;
	if (parentWidget_->window() != parentWidget_)
		return;

#ifdef Q_WS_WIN
	FLASHWINFO fwi;
	fwi.cbSize = sizeof(fwi);
	fwi.hwnd = parentWidget_->winId();
	if (yes) {
		fwi.dwFlags = FLASHW_ALL | FLASHW_TIMER;
		fwi.dwTimeout = 0;
		fwi.uCount = 5;
	}
	else {
		fwi.dwFlags = FLASHW_STOP;
		fwi.uCount = 0;
	}
	FlashWindowEx(&fwi);

#elif defined( Q_WS_X11 )
	static Atom demandsAttention = None;
	static Atom wmState = None;


    /* Xlib-based solution */
	// adopted from http://www.qtforum.org/article/12334/Taskbar-flashing.html
	// public domain by Marcin Jakubowski
    Display *xdisplay = QX11Info::display();
    Window rootwin = QX11Info::appRootWindow();

	if (demandsAttention == None)
    	demandsAttention = XInternAtom(xdisplay, "_NET_WM_STATE_DEMANDS_ATTENTION", true);
	if (wmState == None)
		wmState = XInternAtom(xdisplay, "_NET_WM_STATE", true);

    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = wmState;
    e.xclient.display = xdisplay;
    e.xclient.window = parentWidget_->winId();
    e.xclient.format = 32;
    e.xclient.data.l[1] = demandsAttention;
    e.xclient.data.l[2] = 0l;
    e.xclient.data.l[3] = 0l;
    e.xclient.data.l[4] = 0l;

    if (yes) {
        e.xclient.data.l[0] = 1;
    }
    else {
        e.xclient.data.l[0] = 0;
    }
    XSendEvent(xdisplay, rootwin, False, (SubstructureRedirectMask | SubstructureNotifyMask), &e);

#else
	Q_UNUSED(yes)
#endif
}

void GAdvancedWidget::Private::moveEvent(QMoveEvent *)
{
	if (!parentWidget_->isTopLevel())
		return;
#ifdef Q_WS_MAC
	// because we need to work correctly when dragging window between two adjacent
	// vertical displays, we can't use desktop()->availableGeometry(parentWidget),
	// we need to use the combined geometry of all screens, otherwise there will be
	// heavy flicker when dragging between vertical screens
	QRegion allScreens;
	for (int i = 0; i < qApp->desktop()->numScreens(); ++i)
		allScreens += qApp->desktop()->availableGeometry(i);

	QRect r = allScreens.boundingRect();
	// QRect r = qApp->desktop()->availableGeometry(parentWidget);

	QRect g = parentWidget_->frameGeometry();

	int margin = 5;

	if (g.top() < r.top())
		g.moveTo(g.x(), r.top());

	if (g.right() < r.left() + margin)
		g.moveTo(r.left() + margin - g.width(), g.y());

	if (g.left() > r.right() - margin)
		g.moveTo(r.right() - margin, g.y());

	if (g.top() > r.bottom() - margin)
		g.moveTo(g.x(), r.bottom() - margin);

	if (parentWidget_->frameGeometry() != g) {
		newGeometry_ = g;
		if (!parentWidget_->property("disable-geometry-saving").toBool()) {
			QTimer::singleShot(0, this, SLOT(updateGeometry()));
		}
	}
#endif
}

void GAdvancedWidget::Private::updateGeometry()
{
	QWidget *w = (QWidget *)parent();
	w->move(newGeometry_.topLeft());
}

void GAdvancedWidget::Private::saveGeometry()
{
#ifdef USE_PSIOPTIONS
	PsiOptions::instance()->setOption(geometryOptionPath_, parentWidget_->saveGeometry());
#endif
}

void GAdvancedWidget::Private::restoreGeometry()
{
#ifdef USE_PSIOPTIONS
	QRect savedGeometry = PsiOptions::instance()->getOption(geometryOptionPath_).toRect();
	QByteArray byteArray = PsiOptions::instance()->getOption(geometryOptionPath_).toByteArray();

	if (!savedGeometry.isEmpty()) {
		restoreGeometry(savedGeometry);
	}
	else {
		parentWidget_->restoreGeometry(byteArray);
	}
#endif
}

// FIXME: should use frameGeometry
void GAdvancedWidget::Private::restoreGeometry(QRect savedGeometry)
{
	QRect geom = savedGeometry;
	QDesktopWidget *pdesktop = QApplication::desktop();
	int nscreen = pdesktop->screenNumber(geom.topLeft());
	QRect r = pdesktop->screenGeometry(nscreen);

	// if the coordinates are out of the desktop bounds, reset to the top left
	int pad = 10;
	if (geom.left() < r.left())
		geom.moveLeft(r.left());
	if (geom.right() >= r.right())
		geom.moveRight(r.right() - 1);
	if (geom.top() < r.top())
		geom.moveTop(r.top());
	if (geom.bottom() >= r.bottom())
		geom.moveBottom(r.bottom() - 1);
	if ((geom.width() + pad * 2) > r.width())
		geom.setWidth(r.width() - pad * 2);
	if ((geom.height() + pad * 2) > r.height())
		geom.setHeight(r.height() - pad * 2);

	parentWidget_->move(geom.topLeft());
	parentWidget_->resize(geom.size());
}

bool GAdvancedWidget::Private::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == parentWidget_) {
		if (e->type() == QEvent::Move || e->type() == QEvent::Resize) {
			if (!parentWidget_->property("disable-geometry-saving").toBool()) {
				saveGeometryTimer_->start();
			}
		}

		return false;
	}

	return QObject::eventFilter(obj, e);
}

//----------------------------------------------------------------------------
// GAdvancedWidget
//----------------------------------------------------------------------------

GAdvancedWidget::GAdvancedWidget(QWidget *parent)
	: QObject(parent)
{
	d = new Private(this, parent);
}

#ifdef Q_OS_WIN
bool GAdvancedWidget::winEvent(MSG* msg, long* result)
{
	if ( msg->message == WM_WINDOWPOSCHANGING ) {
		WINDOWPOS *wpos = (WINDOWPOS *)msg->lParam;

		d->posChanging(&wpos->x, &wpos->y, &wpos->cx, &wpos->cy);

		*result = 0;
		return true;
	}

	return false;
}
#endif

QString GAdvancedWidget::geometryOptionPath() const
{
	return d->geometryOptionPath_;
}

void GAdvancedWidget::setGeometryOptionPath(const QString& optionPath)
{
	Q_ASSERT(d->geometryOptionPath_.isEmpty());
	Q_ASSERT(!optionPath.isEmpty());
	d->geometryOptionPath_ = optionPath;
	d->restoreGeometry();
	d->parentWidget_->installEventFilter(d);
}

bool GAdvancedWidget::flashing() const
{
	return d->flashing();
}

void GAdvancedWidget::dockTo(QRect window)
{
	QDesktopWidget *pdesktop = QApplication::desktop();
	int nscreen = pdesktop->screenNumber(window.topLeft());
	QRect r = pdesktop->screenGeometry(nscreen);

	QRect geom = window;
	int width = 400;
	int margin = 1;

	if (window.left() - r.left() >= r.right() - window.right()) {
		geom.setRight(window.left() - 1 - margin);
		geom.setLeft(geom.right() - width);
	}
	else {
		geom.setLeft(window.right() + 1 + margin);
		geom.setRight(geom.left() + width);
	}

	d->restoreGeometry(geom);
}

void GAdvancedWidget::doFlash(bool on)
{
	d->doFlash( on );
}

void GAdvancedWidget::moveToCenterOfScreen()
{
	QRect r = QApplication::desktop()->availableGeometry(d->parentWidget_);
	d->parentWidget_->move((r.width() - d->parentWidget_->width()) / 2,
	                       (r.height() - d->parentWidget_->height()) / 2);
}

#ifdef Q_WS_WIN
// http://groups.google.ru/group/borland.public.cppbuilder.winapi/msg/6eb6f1832d68686d?hl=ru&
bool ForceForegroundWindow(HWND hwnd)
{
	// Code from Thomas Stutz @ delphi3000.com
	// Converted to Borland C++ Builder Code by Wolfgang Frisch
	bool Result;
	// #define SPI_GETFOREGROUNDLOCKTIMEOUT (0x2000);
	// #define SPI_SETFOREGROUNDLOCKTIMEOUT (0x2001);
	DWORD nullvalue = 0;

	DWORD ForegroundThreadID;
	DWORD ThisThreadID;
	DWORD timeout;

	if (IsIconic(hwnd)) {
		ShowWindow(hwnd, SW_RESTORE);
	}

	if (GetForegroundWindow() == hwnd) {
		return true;
	}
	else {
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);

		// Windows 98/2000 doesn't want to foreground a window when some other
		// window has keyboard focus
		if (((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) && (osvi.dwMajorVersion > 4))
		    || ((osvi.dwPlatformId  == VER_PLATFORM_WIN32_WINDOWS)
		        && ((osvi.dwMajorVersion  > 4) || ((osvi.dwMajorVersion == 4) &&
		                                           (osvi.dwMinorVersion > 0))))) {
			// Code from Karl E. Peterson, www.mvps.org/vb/sample.htm
			// Converted to Delphi by Ray Lischner
			// Published in The Delphi Magazine 55, page 16

			Result = false;
			ForegroundThreadID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
			ThisThreadID = GetWindowThreadProcessId(hwnd, NULL);
			if (AttachThreadInput(ThisThreadID, ForegroundThreadID, true)) {
				BringWindowToTop(hwnd); // IE 5.5 related hack
				SetForegroundWindow(hwnd);
				AttachThreadInput(ThisThreadID, ForegroundThreadID, false);
				Result = (GetForegroundWindow() == hwnd);
			}

			if (!Result) {
				// Code by Daniel P. Stasinski
				SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &timeout, 0);
				SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &nullvalue, SPIF_SENDCHANGE);
				BringWindowToTop(hwnd); // IE 5.5 related hack
				SetForegroundWindow(hwnd);
				SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &timeout, SPIF_SENDCHANGE);
			}
		}
		else {
			BringWindowToTop(hwnd); // IE 5.5 related hack
			SetForegroundWindow(hwnd);
		}
		Result = (GetForegroundWindow() == hwnd);
	}
	return Result;
}
#endif

/**
 * http://trolltech.com/customer/task-tracker/index_html?id=202971&method=entry
 * There's a bug in Qt that prevents us to show a window on Windows
 * without it being activated in the process. This is not good in some cases.
 * We try our best to work-around this on Windows.
 */
void GAdvancedWidget::showWithoutActivation()
{
	if (d->parentWidget_->isVisible())
		return;

#ifdef Q_WS_WIN
	HWND foregroundWindow = GetForegroundWindow();
#endif
	d->parentWidget_->show();
#ifdef Q_WS_WIN
	if (foregroundWindow) {
		// the first step is to make sure we're the topmost window
		// otherwise step two doesn't seem to have any effect at all
		ForceForegroundWindow(d->parentWidget_->winId());
		ForceForegroundWindow(foregroundWindow);
	}
#endif
}

void GAdvancedWidget::windowActivationChange(bool oldstate)
{
	Q_UNUSED(oldstate);
	if ( d->parentWidget_->isActiveWindow() ) {
		doFlash(false);
	}
}

void GAdvancedWidget::preShowWidgetOffscreen()
{
	d->parentWidget_->setProperty("disable-geometry-saving", true);

#ifdef USE_YAVISUALUTIL
	d->parentWidget_->move(0, Ya::VisualUtil::belowScreenGeometry() + 100);
#endif
}

void GAdvancedWidget::postShowWidgetOffscreen()
{
	qApp->processEvents();
	// QThread::currentThread()->sleep(1);

	d->parentWidget_->setProperty("disable-geometry-saving", false);
}

int GAdvancedWidget::stickAt()
{
	return Private::stickAt;
}

void GAdvancedWidget::setStickAt(int val)
{
	Private::stickAt = val;
}

bool GAdvancedWidget::stickToWindows()
{
	return Private::stickToWindows;
}

void GAdvancedWidget::setStickToWindows(bool val)
{
	Private::stickToWindows = val;
}

bool GAdvancedWidget::stickEnabled()
{
	return Private::stickEnabled;
}

void GAdvancedWidget::setStickEnabled(bool val)
{
	Private::stickEnabled = val;
}

void GAdvancedWidget::moveEvent(QMoveEvent *e)
{
	d->moveEvent(e);
}

#include "advwidget.moc"
