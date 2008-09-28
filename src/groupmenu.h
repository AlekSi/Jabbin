/*
 * groupmenu.h - a menu with a list of groups
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

#ifndef GROUPMENU_H
#define GROUPMENU_H

#include <QMenu>

class GroupMenu : public QMenu
{
	Q_OBJECT
public:
	GroupMenu(QWidget* parent);

	void addGroup(QString text, QString groupName, bool selected);

signals:
	void groupActivated(QString groupName);

private slots:
	void actionActivated();
};

#endif
