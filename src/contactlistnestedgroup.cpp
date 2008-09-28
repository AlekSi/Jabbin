/*
 * contactlistnestedgroup.cpp - class to handle nested contact list groups
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

#include "contactlistnestedgroup.h"

#include <QStringList>

#include "psicontact.h"
#include "contactlistitemproxy.h"

#define CONTACTLIST_NESTED_GROUPS

ContactListNestedGroup::ContactListNestedGroup(ContactListModel* model, ContactListGroup* parent, QString name)
	: ContactListGroup(model, parent)
{
	internalSetName(name);
}

ContactListNestedGroup::~ContactListNestedGroup()
{
	internalSetName(QString());
	foreach(ContactListGroup* group, groups_) {
// qWarning("~ContactListNextedGroup(%x): %s", this, qPrintable(group->fullName()));
		removeItem(ContactListGroup::findGroup(group));
	}
	qDeleteAll(groups_);
	groups_.clear();
}

void ContactListNestedGroup::addContact(PsiContact* contact, QStringList contactGroups)
{
	foreach(QString groupName, contactGroups) {
		if (groupName == name()) {
			ContactListGroup::addContact(contact, contactGroups);
		}
		else {
			QStringList nestedGroups;
#ifdef CONTACTLIST_NESTED_GROUPS
			nestedGroups = groupName.split(groupDelimiter());
#else
			nestedGroups += groupName;
#endif
			if (!name().isEmpty()) {
				QString firstPart = nestedGroups.takeFirst();
				Q_ASSERT(firstPart == name());
			}

			Q_ASSERT(!nestedGroups.isEmpty());

			ContactListGroup* group = findGroup(nestedGroups.first());
			if (!group) {
				group = new ContactListNestedGroup(model(), this, nestedGroups.first());
				groups_.append(group);
// qWarning("ContactListNextedGroup(%x)::addContact: %s", this, qPrintable(group->fullName()));
				addItem(new ContactListItemProxy(this, group));
			}

			QStringList moreGroups;
			moreGroups << nestedGroups.join(groupDelimiter());
			group->addContact(contact, moreGroups);
		}
	}
}

void ContactListNestedGroup::contactUpdated(PsiContact* contact)
{
	// since now we're using groupCache() passing contact to
	// all nested groups is not necessary anymore
	// foreach(ContactListGroup* group, groups_)
	// 	group->contactUpdated(contact);
	ContactListGroup::contactUpdated(contact);
}

#define CONTACTLISTNESTEDGROUP_OLD_CONTACTGROUPSCHANGED

void ContactListNestedGroup::contactGroupsChanged(PsiContact* contact, QStringList contactGroups)
{
#ifdef CONTACTLISTNESTEDGROUP_OLD_CONTACTGROUPSCHANGED
	bool addToSelf = false;
	QList<QStringList> splitGroupNames;
	foreach(QString group, contactGroups) {
		if (group == name()) {
			addToSelf = true;
			continue;
		}

		QStringList split;
#ifdef CONTACTLIST_NESTED_GROUPS
		split = group.split(groupDelimiter());
#else
		split += group;
#endif
		if (!name().isEmpty()) {
			QString firstPart = split.takeFirst();
			// hmm, probably should continue as the data should be invalid
		}
		splitGroupNames << split;
	}

	// here we process splitGroupNames and update nested groups which are already present
	QList<ContactListGroup*> emptyGroups;
	QVector<ContactListGroup*> unnotifiedGroups = groups_;
	QList<QStringList> newGroups;
	while (!splitGroupNames.isEmpty()) {
		const QStringList& split = splitGroupNames.first();
		ContactListGroup* group = 0;
		if (!split.isEmpty())
			group = findGroup(split.first());

		if (!group) {
			newGroups << splitGroupNames.takeFirst();
			continue;
		}

		QStringList mergedGroupNames;
		foreach(QStringList i, splitGroupNames)
			if (!i.isEmpty() && i.first() == split.first())
				mergedGroupNames += i.join(groupDelimiter());

		foreach(QString i, mergedGroupNames) {
#ifdef CONTACTLIST_NESTED_GROUPS
			splitGroupNames.remove(i.split(groupDelimiter()));
#else
			splitGroupNames.remove(QStringList() << i);
#endif
		}

		group->contactGroupsChanged(contact, mergedGroupNames);
		if (!group->itemsCount()) {
			emptyGroups << group;
		}
		unnotifiedGroups.remove(unnotifiedGroups.indexOf(group));
	}

	// remove the contact from the unnotified groups
	foreach(ContactListGroup* group, unnotifiedGroups) {
		group->contactGroupsChanged(contact, QStringList());
		if (!group->itemsCount()) {
			emptyGroups << group;
		}
	}

	// remove empty groups afterwards
	foreach(ContactListGroup* group, emptyGroups) {
// qWarning("ContactListNextedGroup(%x)::contactGroupsChanged: removing empty group: %s", this, qPrintable(group->fullName()));
		removeItem(ContactListGroup::findGroup(group));
		groups_.remove(groups_.indexOf(group));
		delete group;
	}

	// create new groups, if required
	foreach(QStringList split, newGroups) {
		QStringList fullGroupName;
		if (!name().isEmpty())
			fullGroupName << name();
		fullGroupName += split;
		QStringList tmp;
		tmp << fullGroupName.join(groupDelimiter());
		addContact(contact, tmp);
	}

	if (addToSelf) {
		// contact should be located here
		if (!findContact(contact)) {
			Q_ASSERT(!contactGroups.isEmpty());
			ContactListGroup::contactGroupsChanged(contact, contactGroups);
		}
	}
	else {
		// remove self
		if (findContact(contact)) {
			ContactListGroup::contactGroupsChanged(contact, QStringList());
		}
	}
#else
	QStringList newNestedGroups = fullName().isEmpty()
	                              ? contactGroups
	                              : contactGroups.filter(QRegExp(QString("^%1($|%2)").arg(fullName(), groupDelimiter())));

	QStringList directChildren;
	foreach(QString nnGroup, newNestedGroups) {
		QString unqualifiedName = nnGroup.mid(QString(fullName().isEmpty() ? "" : fullName() + groupDelimiter()).length());
		if (!unqualifiedName.isEmpty()) {
			// direct children!
			directChildren << QString(name().isEmpty() ? "" : name() + groupDelimiter()) + unqualifiedName;
		}
	}

	// let's add contacts to the children
	addContact(contact, directChildren);

	// we're running into recursion
	foreach(ContactListGroup* child, groups_) {
		child->contactGroupsChanged(contact, newNestedGroups);
		if (!child->itemsCount()) {
// qWarning("ContactListNextedGroup(%x)::contactGroupsChanged: removing empty group2: %s", this, qPrintable(child->fullName()));
			removeItem(ContactListGroup::findGroup(child));
			groups_.remove(groups_.indexOf(child));
			delete group;
		}
	}

	// let's add/remove the contact from itself, if necessary
	ContactListGroup::contactGroupsChanged(contact, newNestedGroups.filter(QRegExp(QString("^%1$").arg(fullName()))));
#endif
}

ContactListGroup* ContactListNestedGroup::findGroup(const QString& groupName) const
{
	foreach(ContactListGroup* group, groups_)
		if (group->name() == groupName)
			return group;

	return 0;
}

bool ContactListNestedGroup::expanded() const
{
	return ContactListGroup::expanded();
}

void ContactListNestedGroup::setExpanded(bool expanded)
{
	ContactListGroup::setExpanded(expanded);
}
