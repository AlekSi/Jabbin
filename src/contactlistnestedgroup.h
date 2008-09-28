/*
 * contactlistnestedgroup.h - class to handle nested contact list groups
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

#ifndef CONTACTLISTNESTEDGROUP_H
#define CONTACTLISTNESTEDGROUP_H

#include "contactlistgroup.h"

class ContactListNestedGroup : public ContactListGroup
{
public:
	ContactListNestedGroup(ContactListModel* model, ContactListGroup* parent, QString name);
	~ContactListNestedGroup();

	// reimplemented
	virtual void addContact(PsiContact* contact, QStringList contactGroups);
	virtual void contactUpdated(PsiContact* contact);
	virtual void contactGroupsChanged(PsiContact* contact, QStringList contactGroups);

	// reimplemented
	virtual bool expanded() const;
	virtual void setExpanded(bool expanded);

protected:
	ContactListGroup* findGroup(const QString& groupName) const;

private:
	QVector<ContactListGroup*> groups_;
};

#endif
