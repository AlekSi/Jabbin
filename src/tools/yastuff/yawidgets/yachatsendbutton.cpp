/*
 * yachatsendbutton.cpp - custom send button for chat dialog
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

#include "yachatsendbutton.h"

#include <QPainter>
#include <QAction>

#include "tabbablewidget.h"
#include "yawindowbackground.h"

static QString pixmapPath(QString path, bool officeTheme)
{
	QString dir = officeTheme ? "office" : "funny";
	dir = ":images/chat/send_button/" + dir;

	return path.arg(dir);
}

static int pixmapIndex(bool officeTheme)
{
	return officeTheme ? 1 : 0;
}

static QPixmap buttonBackground(bool officeTheme, bool pressed)
{
	QPixmap pix[2];
	QPixmap pix_pressed[2];

	if (pressed) {
		if (pix_pressed[pixmapIndex(officeTheme)].isNull()) {
			pix_pressed[pixmapIndex(officeTheme)] = QPixmap(pixmapPath("%1/send_background_pressed.png", officeTheme));
		}
		Q_ASSERT(!pix_pressed[pixmapIndex(officeTheme)].isNull());
		return pix_pressed[pixmapIndex(officeTheme)];
	}

	if (pix[pixmapIndex(officeTheme)].isNull()) {
		pix[pixmapIndex(officeTheme)] = QPixmap(pixmapPath("%1/send_background.png", officeTheme));
	}
	Q_ASSERT(!pix[pixmapIndex(officeTheme)].isNull());
	return pix[pixmapIndex(officeTheme)];
}

static QPixmap buttonGlyph(bool officeTheme, bool hover)
{
	QPixmap pix[2];
	QPixmap pix_hover[2];
	if (hover) {
		if (pix_hover[pixmapIndex(officeTheme)].isNull()) {
			pix_hover[pixmapIndex(officeTheme)] = QPixmap(pixmapPath("%1/send_glyph_hover.png", officeTheme));
		}
		Q_ASSERT(!pix_hover[pixmapIndex(officeTheme)].isNull());
		return pix_hover[pixmapIndex(officeTheme)];
	}

	if (pix[pixmapIndex(officeTheme)].isNull()) {
		pix[pixmapIndex(officeTheme)] = QPixmap(pixmapPath("%1/send_glyph.png", officeTheme));
	}
	Q_ASSERT(!pix[pixmapIndex(officeTheme)].isNull());
	return pix[pixmapIndex(officeTheme)];
}

//----------------------------------------------------------------------------
// YaChatSendButtonExtra
//----------------------------------------------------------------------------

YaChatSendButtonExtra::YaChatSendButtonExtra(QWidget* parent)
	: QToolButton(parent)
{
}

YaChatSendButtonExtra::~YaChatSendButtonExtra()
{
}

void YaChatSendButtonExtra::paintEvent(QPaintEvent*)
{
	QPainter p(this);

	bool officeTheme = false;
	TabbableWidget* dlg = dynamic_cast<TabbableWidget*>(parentWidget()->parentWidget());
	if (dlg) {
		officeTheme = dlg->background().name() == "office.png";
	}

	p.drawPixmap(rect(), buttonBackground(officeTheme, isDown()));
	QPixmap pix = buttonGlyph(officeTheme, /*underMouse() && isEnabled()*/ true);
	p.drawPixmap((width() - pix.width()) / 2, (isDown() ? 2 : 0) + (height() - pix.height()) / 2, pix);
}

//----------------------------------------------------------------------------
// YaChatSendButton
//----------------------------------------------------------------------------

YaChatSendButton::YaChatSendButton(QWidget* parent)
	: OverlayWidget<QWidget, YaChatSendButtonExtra>(parent,
	        new YaChatSendButtonExtra(parent->parentWidget()))
{
	connect(extra(), SIGNAL(clicked()), SIGNAL(clicked()));
}

YaChatSendButton::~YaChatSendButton()
{
}

QRect YaChatSendButton::extraGeometry() const
{
	static const int shadowHeight = 2;
	return QRect(globalRect().x(),
	             globalRect().y() - shadowHeight,
	             width(),
	             height() + shadowHeight);
}

void YaChatSendButton::setAction(QAction* action)
{
	extra()->setDefaultAction(action);
}

void YaChatSendButton::updatePosition()
{
	gOverlayWidget()->delayedInvalidateExtraPosition();
}
