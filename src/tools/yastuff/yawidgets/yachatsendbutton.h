/*
 * yachatsendbutton.h - custom send button for chat dialog
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

#ifndef YACHATSENDBUTTON_H
#define YACHATSENDBUTTON_H

#include <QToolButton>

#include "overlaywidget.h"

class YaChatSendButtonExtra : public QToolButton
{
	Q_OBJECT
public:
	YaChatSendButtonExtra(QWidget* parent);
	~YaChatSendButtonExtra();

protected:
	// reimplemented
	void paintEvent(QPaintEvent*);
};

class YaChatSendButton : public OverlayWidget<QWidget, YaChatSendButtonExtra>
{
	Q_OBJECT
public:
	YaChatSendButton(QWidget* parent);
	~YaChatSendButton();

	void setAction(QAction* action);
	void updatePosition();

signals:
	void clicked();

private:
	// reimplemented
	QRect extraGeometry() const;
};

#endif
