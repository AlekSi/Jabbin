/*
 * yaclosebutton.cpp - close button
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

#include "yaclosebutton.h"

YaCloseButton::YaCloseButton(QWidget* parent)
	: QToolButton(parent)
	, chatButton_(false)
{
	setFocusPolicy(Qt::NoFocus);

	normal_  = QPixmap(":/images/closetab.png");
	setChatButton(chatButton_);
}

QSize YaCloseButton::sizeHint() const
{
	return normal_.size();
}

bool YaCloseButton::chatButton() const
{
	return chatButton_;
}

void YaCloseButton::setChatButton(bool chatButton)
{
	QString normal = "closetab.png";
	if (chatButton) {
		normal = "closetab_chat.png";
	}

#ifndef YAPSI_NO_STYLESHEETS
	setStyleSheet(QString(
	"QToolButton {"
	"	border-image: none;"
	"	border: 0px;"
	"	background: url(:/images/%1) center center no-repeat;"
	"}"
	"QToolButton:pressed, QToolButton:hover, QToolButton:focus {"
	"	background: url(:/images/closetab_pressed.png) center center no-repeat;"
	"}"
	).arg(normal)
	);
#endif
}
