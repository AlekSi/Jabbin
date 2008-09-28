/*
 * groupmenu.cpp - a menu with a list of groups
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

#include "groupmenu.h"

#include <QAction>

GroupMenu::GroupMenu(QWidget* parent)
	: QMenu(parent)
{
}

void GroupMenu::actionActivated()
{
	QAction* action = static_cast<QAction*>(sender());
	emit groupActivated(action->property("groupName").toString());
}

/**
 * \param text will be shown on screen, and \param groupName is the 
 * actual group name. Specify true as \param current when group is
 * currently selected for a contact.
 */
void GroupMenu::addGroup(QString text, QString groupName, bool current)
{
	QAction* action = new QAction(text, this);
	addAction(action);
	action->setCheckable(true);
	action->setChecked(current);
	action->setProperty("groupName", QVariant(groupName));
	connect(action, SIGNAL(activated()), SLOT(actionActivated()));
}
