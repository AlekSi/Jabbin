/*
 * contactlistgroupmenu.cpp - context menu for contact list groups
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

#include "contactlistgroupmenu.h"

#include <QPointer>

#include "iconaction.h"
#include "iconset.h"
#include "contactlistgroup.h"
#include "psioptions.h"
#include "shortcutmanager.h"
#include "psicontactlist.h"

class ContactListGroupMenu::Private : public QObject
{
	Q_OBJECT

	QPointer<ContactListGroup> group;
	IconAction* act_rename;
	QAction* act_removeGroupAndContacts;
	QAction* act_addGroup;

public:
	Private(ContactListGroupMenu* menu, ContactListGroup* _group)
		: QObject(0)
		, group(_group)
		, menu_(menu)
	{
		connect(menu, SIGNAL(aboutToShow()), SLOT(updateActions()));

		act_rename = new IconAction("", tr("Re&name"), menu->shortcuts("contactlist.rename"), this, "act_rename");
		connect(act_rename, SIGNAL(activated()), this, SLOT(rename()));

		act_removeGroupAndContacts = new QAction(tr("&Remove"), this);
		act_removeGroupAndContacts->setShortcuts(ShortcutManager::instance()->shortcuts("contactlist.delete"));
		connect(act_removeGroupAndContacts, SIGNAL(activated()), SLOT(removeGroupAndContacts()));

		act_addGroup = new QAction(tr("&Add group..."), this);
		connect(act_addGroup, SIGNAL(activated()), SLOT(addGroup()));

		updateActions();

		// menu->addAction(act_sendMessage);
		menu->addAction(act_rename);
		// menu->addSeparator();
		// menu->addAction(act_removeGroup); // disassociates all contacts with this group
		menu->addAction(act_removeGroupAndContacts); // removes all contacts within this group
		menu->addAction(act_addGroup);
	}

private slots:
	void updateActions()
	{
		if (!group)
			return;

		act_rename->setEnabled(group->isEditable());
		act_removeGroupAndContacts->setEnabled(group->isRemovable());
		act_addGroup->setEnabled(group->model()->contactList()->haveAvailableAccounts());
	}

	void rename()
	{
		if (group) {
			// group->contactList()->emitRename();
			menu_->model()->renameSelectedItem();
		}
	}

	void removeGroupAndContacts()
	{
		if (group) {
			emit menu_->removeSelection();
		}
	}

	void addGroup()
	{
		if (group) {
			emit menu_->addGroup();
		}
	}

private:
	ContactListGroupMenu* menu_;
};

ContactListGroupMenu::ContactListGroupMenu(ContactListGroup* group, ContactListModel* model)
	: ContactListItemMenu(group, model)
{
	d = new Private(this, group);
}

ContactListGroupMenu::~ContactListGroupMenu()
{
	delete d;
}

#include "contactlistgroupmenu.moc"
