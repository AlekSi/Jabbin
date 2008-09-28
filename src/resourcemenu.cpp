/*
 * resourcemenu.cpp - helper class for displaying contact's resources
 * Copyright (C) 2006  Michail Pishchagin
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

#include "resourcemenu.h"
#include "psiiconset.h"
#include "userlist.h"
#include "xmpp_status.h"

/**
 * \class ResourceMenu
 * Helper class that displays available resources using QMenu.
 */

ResourceMenu::ResourceMenu(QWidget *parent)
	: QMenu(parent)
{
	// nothing here
}

/**
 * Helper function to add resource to the menu.
 */
void ResourceMenu::addResource(const UserResource &r)
{
	addResource(r.status().type(), r.name());
}

/**
 * Helper function to add resource to the menu.
 */
void ResourceMenu::addResource(int status, QString name)
{
	QString rname = name;
	if(rname.isEmpty())
		rname = tr("[blank]");

	//rname += " (" + status2txt(status) + ")";

	QAction* action = new QAction(PsiIconset::instance()->status(status).icon(), rname, this);
	addAction(action);
	action->setProperty("resource", QVariant(name));
	connect(action, SIGNAL(activated()), SLOT(actionActivated()));
}

void ResourceMenu::actionActivated()
{
	QAction* action = static_cast<QAction*>(sender());
	emit resourceActivated(action->property("resource").toString());
}
