/*
 * yacontactlistmodel.cpp - custom ContactListModel
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

#include "yacontactlistmodel.h"

#include <QtAlgorithms>

#include "psioptions.h"
#include "psiaccount.h"
#include "contactlistgroup.h"
#include "contactlistitemproxy.h"
#include "psicontactlist.h"
#include "psicontact.h"
#include "yacommon.h"
#include "common.h"
#include "contactlistnestedgroup.h"
#include "yacontactlistmodelselection.h"
#include "contactlistgroupcache.h"
#include "fakegroupcontact.h"
#include "contactlistmodelupdater.h"
#include "contactlistgroup.h"

//----------------------------------------------------------------------------
// YaContactListModelOperationList
//----------------------------------------------------------------------------

YaContactListModelOperationList::YaContactListModelOperationList(Action action)
	: action_(action)
{
}

YaContactListModelOperationList::YaContactListModelOperationList(Qt::DropAction action)
{
	if (action == Qt::CopyAction)
		action_ = Copy;
	else if (action == Qt::MoveAction)
		action_ = Move;
	else {
		Q_ASSERT_X(false, "YaContactListModelOperationList", "Passed incorrect Qt::DropAction");
	}
}

YaContactListModelOperationList::Action YaContactListModelOperationList::action() const
{
	return action_;
}

void YaContactListModelOperationList::addOperation(PsiContact* contact, const QString& groupFrom, const QString& groupTo)
{
	if (!contact) {
		qWarning("YaContactListModelOperationList::addOperation(): contact is NULL");
		return;
	}

	if (!contact->isEditable()) {
		qWarning("YaContactListModelOperationList::addOperation(): contact is not editable '%s'", qPrintable(contact->jid().full()));
		return;
	}

	if (!contact->groupOperationPermitted(groupFrom, groupTo)) {
		qWarning("YaContactListModelOperationList::addOperation(): contact '%s' refused group operation ('%s' -> '%s')", qPrintable(contact->jid().full()), qPrintable(groupFrom), qPrintable(groupTo));
		return;
	}

	Operation op(groupFrom, groupTo);

	if (!operations_.contains(contact))
		operations_[contact] = QList<Operation>();

	operations_[contact] += op;
}

QList<YaContactListModelOperationList::ContactOperation> YaContactListModelOperationList::operations() const
{
	QList<ContactOperation> result;

	QHash<PsiContact*, QList<Operation> >::const_iterator it;
	for (it = operations_.constBegin(); it != operations_.constEnd(); ++it) {
		ContactOperation op;
		op.contact    = it.key();
		op.operations = it.value();
		result += op;
	}

	return result;
}

void YaContactListModelOperationList::removeAccidentalContactMoveOperations()
{
	if (action_ != Move)
		return;

	QList<PsiContact*> contacts = operations_.keys();
	foreach(PsiContact* psiContact, contacts) {
		bool remove = false;
		foreach(Operation op, operations_[psiContact]) {
			if (psiContact->groups().contains(op.groupTo)) {
				remove = true;
				break;
			}
		}

		if (remove)
			operations_.remove(psiContact);
	}
}

//----------------------------------------------------------------------------
// YaContactListModel
//----------------------------------------------------------------------------

YaContactListModel::YaContactListModel(PsiContactList* contactList)
	: ContactListModel(contactList)
	, singleAccount_(PsiOptions::instance()->getOption("options.ui.account.single").toBool()) // TODO: hook on change notifications
{
	setSupportedDragActions(Qt::MoveAction | Qt::CopyAction);
}

ContactListModel* YaContactListModel::clone() const
{
	return new YaContactListModel(contactList());
}

QStringList YaContactListModel::filterContactGroups(QStringList groups) const
{
	QStringList grs = ContactListModel::filterContactGroups(groups);

	foreach (QString grp, Ya::BOTS_GROUP_NAMES) {
		if (grs.contains(grp)) {
			grs.remove(grp);
		}
	}
	return grs;
}

bool YaContactListModel::singleAccount() const
{
	return singleAccount_;
}

QVariant YaContactListModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	ContactListItemProxy* item = static_cast<ContactListItemProxy*>(index.internalPointer());
	PsiContact* contact = 0;

	if (role == Qt::ForegroundRole) {
		if (item && (contact = dynamic_cast<PsiContact*>(item->item()))) {
			return qVariantFromValue(Ya::statusColor(contact->status().type()));
		}
	}

	return ContactListModel::data(index, role);
}

Qt::DropActions YaContactListModel::supportedDropActions() const
{
	return Qt::MoveAction | Qt::CopyAction;
}

Qt::ItemFlags YaContactListModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = ContactListModel::flags(index);

	ContactListItemProxy* item = static_cast<ContactListItemProxy*>(index.internalPointer());

	if (item && item->item()) {
		// TODO: disable dragging of groups if account is offline!
		return f | Qt::ItemIsDropEnabled | (item->item()->isEditable() ? Qt::ItemIsDragEnabled : f);
	}

	return f;
}

QStringList YaContactListModel::mimeTypes() const
{
	return QStringList()
	       << YaContactListModelSelection::mimeType()
	       << "text/plain";
}

QMimeData* YaContactListModel::mimeData(const QModelIndexList& indexes) const
{
	QList<ContactListItemProxy*> items;

	foreach(QModelIndex index, indexes) {
		if (index.isValid()) {
			ContactListItemProxy* itemProxy = static_cast<ContactListItemProxy*>(index.internalPointer());
			if (!itemProxy)
				continue;

			items << itemProxy;
		}
	}

	return new YaContactListModelSelection(items);
}

QModelIndexList YaContactListModel::indexesFor(const QMimeData* data) const
{
	QModelIndexList result;
	YaContactListModelSelection selection(data);
	if (!selection.haveRosterSelection() || !contactList())
		return result;

	foreach(YaContactListModelSelection::Contact contact, selection.contacts()) {
		PsiAccount* account = contactList()->getAccount(contact.account);
		if (!account)
			continue;
		PsiContact* psiContact = account->findContact(contact.jid);
		if (!psiContact)
			continue;

		QList<ContactListGroup*> groups = groupCache()->groupsFor(psiContact);
		if (!contact.group.isEmpty()) {
			QList<ContactListGroup*> matched;
			foreach(ContactListGroup* group, groups) {
				if (group->fullName() == contact.group) {
					matched << group;
					break;
				}
			}

			if (!matched.isEmpty())
				groups = matched;
		}

		foreach(ContactListGroup* group, groups) {
			int contactIndex = group->indexOf(psiContact);
			if (contactIndex >= 0) {
				ContactListItemProxy* itemProxy = group->item(contactIndex);
				if (itemProxy) {
					QModelIndex index = itemProxyToModelIndex(itemProxy);
					if (index.isValid() && !result.contains(index))
						result << index;
				}
			}
		}
	}

	// TODO: Add account support
	foreach(YaContactListModelSelection::Group group, selection.groups()) {
		ContactListGroup* contactGroup = groupCache()->findGroup(group.fullName);
		QModelIndex index = groupToIndex(contactGroup);
		if (!result.contains(index))
			result << index;
	}

	return result;
}

QModelIndex YaContactListModel::groupToIndex(ContactListGroup* group) const
{
	if (group && group->parent()) {
		int groupIndex = group->parent()->indexOf(group);
		if (groupIndex >= 0) {
			ContactListItemProxy* itemProxy = group->parent()->item(groupIndex);
			if (itemProxy) {
				QModelIndex index = itemProxyToModelIndex(itemProxy);
				return index;
			}
		}
	}

	return QModelIndex();
}

bool YaContactListModel::supportsMimeDataOnIndex(const QMimeData* data, const QModelIndex& parent) const
{
	if (!groupsEnabled() || !YaContactListModelSelection(data).haveRosterSelection()) {
		return false;
	}

#ifdef USE_GENERAL_CONTACT_GROUP
	if (!parent.isValid()) {
		return false;
	}
#endif

	foreach(QModelIndex index, indexesFor(data)) {
		if (index == parent) {
			return false;
		}

		// protection against dragging parent group inside its child
		ContactListItemProxy* item = itemProxy(index);
		Q_ASSERT(item);
		ContactListGroup* group = dynamic_cast<ContactListGroup*>(item->item());
		if (group) {
			ContactListItemProxy* item2 = itemProxy(parent);
			ContactListGroup* group2 = item2 ? item2->parent() : 0;
			if (group2 && group2->fullName().startsWith(group->fullName() + ContactListGroup::groupDelimiter())) {
				return false;
			}
		}
	}

	return true;
}

void YaContactListModel::addOperationsForGroupRename(const QString& currentGroupName, const QString& newGroupName, YaContactListModelOperationList* operations) const
{
	ContactListGroup* group = groupCache()->findGroup(currentGroupName);
	if (group) {
		for (int i = 0; i < group->itemsCount(); ++i) {
			ContactListItemProxy* itemProxy = group->item(i);
			PsiContact* contact = 0;
			ContactListGroup* childGroup = 0;
			if ((contact = dynamic_cast<PsiContact*>(itemProxy->item()))) {
				operations->addOperation(contact, currentGroupName, newGroupName);
			}
			else if ((childGroup = dynamic_cast<ContactListGroup*>(itemProxy->item()))) {
				QString theName = childGroup->fullName().split(ContactListGroup::groupDelimiter()).last();
				QString newName = (newGroupName.isEmpty() ? "" : newGroupName + ContactListGroup::groupDelimiter()) + theName;
				addOperationsForGroupRename(childGroup->fullName(), newName, operations);
			}
		}
	}
}

bool YaContactListModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
	Q_UNUSED(row);
	Q_UNUSED(column);

	YaContactListModelSelection selection(data);
	if (!selection.haveRosterSelection() || !contactList())
		return false;

	YaContactListModelOperationList operations(action);

	foreach(YaContactListModelSelection::Contact contact, selection.contacts()) {
		PsiAccount* account = contactList()->getAccount(contact.account);
		PsiContact* psiContact = account ? account->findContact(contact.jid) : 0;
		operations.addOperation(psiContact,
		                        contact.group,
		                        getDropGroupName(parent));
	}

	foreach(YaContactListModelSelection::Group group, selection.groups()) {
		QString parentGroupName = getDropGroupName(parent);
		if (parentGroupName.startsWith(group.fullName + ContactListGroup::groupDelimiter())) {
			qWarning("Dropping group to its descendant is not supported ('%s' -> '%s')", qPrintable(group.fullName), qPrintable(parentGroupName));
			continue;
		}

		if (parentGroupName == group.fullName)
			continue;

		// TODO: unify these two lines with the ones in operationsForGroupRename
		QString theName = group.fullName.split(ContactListGroup::groupDelimiter()).last();
		QString newName = (parentGroupName.isEmpty() ? "" : parentGroupName + ContactListGroup::groupDelimiter()) + theName;
		if (newName == group.fullName)
			continue;

		addOperationsForGroupRename(group.fullName, newName, &operations);
	}

	if (selection.isMultiSelection())
		operations.removeAccidentalContactMoveOperations();

	// qWarning("*** New operation list: ***");
	// foreach(YaContactListModelOperationList::ContactOperation contactOperation, operations.operations()) {
	// 	QStringList groups;
	// 	foreach(YaContactListModelOperationList::Operation op, contactOperation.operations)
	// 		groups += QString("%1 -> %2").arg(op.groupFrom, op.groupTo);
	// 	qWarning("\t'%s' (%s)", qPrintable(contactOperation.contact->jid()), qPrintable(groups.join(";")));
	// }

	performContactOperations(operations, Operation_DragNDrop);

	return true;
}

void YaContactListModel::renameGroup(ContactListGroup* group, const QString& newName)
{
	Q_ASSERT(group);
	YaContactListModelOperationList operations(YaContactListModelOperationList::Move);

	QStringList name = group->fullName().split(ContactListGroup::groupDelimiter());
	if (name.isEmpty())
		return;
	name.takeLast();
	if (!newName.isEmpty())
		name << newName;
	addOperationsForGroupRename(group->fullName(), name.join(ContactListGroup::groupDelimiter()), &operations);

	performContactOperations(operations, Operation_GroupRename);
}

void YaContactListModel::performContactOperations(const YaContactListModelOperationList& operations, OperationType operationType)
{
	QHash<ContactListGroup*, int> groupContactCount;

	foreach(YaContactListModelOperationList::ContactOperation contactOperation, operations.operations()) {
		PsiContact* psiContact = contactOperation.contact;
		if (!psiContact) {
			qWarning("Only contact moving via drag'n'drop is supported for now.");
			continue;
		}

		// PsiAccount* dropAccount = getDropAccount(account, parent);
		// if (!dropAccount || dropAccount != account) {
		// 	qWarning("Dropping to different accounts is not supported for now.");
		// 	return;
		// }

		QStringList groups = psiContact->groups();

		foreach(YaContactListModelOperationList::Operation op, contactOperation.operations) {
			if (operations.action() == YaContactListModelOperationList::Move) {
				groups.remove(op.groupFrom);

				ContactListGroup* group = groupCache()->findGroup(op.groupFrom);
				if (group && groupContactCount.contains(group)) {
					groupContactCount[group] -= 1;
				}
				else if (group) {
					groupContactCount[group] = group->contactsCount() - 1;
				}
			}

			if (!groups.contains(op.groupTo)) {
				groups << op.groupTo;
			}
		}

		psiContact->setGroups(groups);
	}

	QHashIterator<ContactListGroup*, int> it(groupContactCount);
	while (it.hasNext()) {
		it.next();

		if (!it.value() && operationType == Operation_DragNDrop) {
			// qWarning("group = %s; %d", qPrintable(it.key()->fullName()), it.value());
			FakeGroupContact* contact = addFakeGroupContact(QStringList(), it.key()->fullName());
		}
	}
}

// TODO: think of a way to merge this with performContactOperations
QList<PsiContact*> YaContactListModel::removeIndexesHelper(const QMimeData* data, bool performRemove)
{
	QList<PsiContact*> result;
	QMap<PsiContact*, QStringList> toRemove;
	YaContactListModelOperationList operations = removeOperationsFor(data);

	foreach(YaContactListModelOperationList::ContactOperation contactOperation, operations.operations()) {
		PsiContact* psiContact = contactOperation.contact;
		if (!psiContact)
			continue;

		QStringList groups = psiContact->groups();

		foreach(YaContactListModelOperationList::Operation op, contactOperation.operations) {
			groups.remove(op.groupFrom);
		}

		if (!groupsEnabled()) {
			groups.clear();
		}

#ifndef YAPSI
		if (groups.isEmpty() && !dynamic_cast<FakeGroupContact*>(psiContact) && psiContact->inList()) {
#else
		if (psiContact->inList() || dynamic_cast<FakeGroupContact*>(psiContact)) {
#endif
			if (!result.contains(psiContact)) {
				result << psiContact;
			}
		}

		toRemove[psiContact] = groups;
	}

	if (performRemove && !toRemove.isEmpty()) {
		beginBulkUpdate();

		QMapIterator<PsiContact*, QStringList> it(toRemove);
		while (it.hasNext()) {
			it.next();
			PsiContact* psiContact = it.key();
			QStringList groups = it.value();

			if (!groups.isEmpty())
				psiContact->setGroups(groups);
			else
				psiContact->remove();
		}

		endBulkUpdate();
	}

	return result;
}

PsiAccount* YaContactListModel::getDropAccount(PsiAccount* account, const QModelIndex& parent) const
{
	Q_UNUSED(account);

	if (!parent.isValid())
		return 0;

	ContactListItemProxy* item = static_cast<ContactListItemProxy*>(parent.internalPointer());
	PsiContact* contact = 0;

	if (item && (contact = dynamic_cast<PsiContact*>(item->item()))) {
		return contact->account();
	}
	// else if ((group = dynamic_cast<ContactListGroup*>(item))) {
	// 	return group->account();
	// }

	return 0;
}

QString YaContactListModel::getDropGroupName(const QModelIndex& parent) const
{
	if (!parent.isValid())
		return 0;

	ContactListItemProxy* item = static_cast<ContactListItemProxy*>(parent.internalPointer());
	ContactListGroup* group = 0;

	if (item) {
		if ((group = dynamic_cast<ContactListGroup*>(item->item()))) {
			return group->fullName();
		}
		else {
			Q_ASSERT(item->parent());
			return item->parent()->fullName();
		}
	}

	return QString();
}

YaContactListModelOperationList YaContactListModel::removeOperationsFor(const QMimeData* data) const
{
	YaContactListModelOperationList operations(YaContactListModelOperationList::Remove);
	YaContactListModelSelection selection(data);

	if (!contactList())
		return operations;

	foreach(YaContactListModelSelection::Contact contact, selection.contacts()) {
		PsiAccount* account = contactList()->getAccount(contact.account);
		PsiContact* psiContact = account ? account->findContact(contact.jid) : 0;
		operations.addOperation(psiContact,
		                        contact.group,
		                        QString());
	}

	foreach(YaContactListModelSelection::Group group, selection.groups()) {
		addOperationsForGroupRename(group.fullName, QString(), &operations);
	}

	return operations;
}

/**
 * Returns list of contacts that will be removed from roster by removeIndexes().
 * Contacts that will not be reported by this function will only lose some groups
 * they belong to.
 */
QList<PsiContact*> YaContactListModel::contactsLostByRemove(const QMimeData* data) const
{
	return ((YaContactListModel*)this)->removeIndexesHelper(data, false);
}

/**
 * Removes selected roster items from roster by either removing some of the
 * contacts' groups, or by removing them from roster altogether.
 */
void YaContactListModel::removeIndexes(const QMimeData* data)
{
	removeIndexesHelper(data, true);
}

FakeGroupContact* YaContactListModel::addFakeGroupContact(QStringList groups, QString specificGroupName)
{
	FakeGroupContact* contact = 0;
	if (specificGroupName.isEmpty())
		contact = new FakeGroupContact(this, groups);
	else
		contact = new FakeGroupContact(this, specificGroupName);

	connect(contact, SIGNAL(destroyed(PsiContact*)), SLOT(fakeContactDestroyed(PsiContact*)));
	fakeGroupContacts_ << contact;

	updater()->addContact(contact);

	return contact;
}

QModelIndex YaContactListModel::addGroup()
{
	if (groupsEnabled()) {
		FakeGroupContact* contact = addFakeGroupContact(groupCache()->groups(), QString());
		updater()->commit();

		QList<ContactListGroup*> groups = groupCache()->groupsFor(contact);
		if (!groups.isEmpty()) {
			ContactListGroup* group = groups.first();
			group->updateOnlineContactsFlag();
			QModelIndex result = groupToIndex(group);
			Q_ASSERT(result.isValid());
			return result;
		}
	}
	return QModelIndex();
}

QList<PsiContact*> YaContactListModel::additionalContacts() const
{
	QList<PsiContact*> result = ContactListModel::additionalContacts();
	result += fakeGroupContacts_;
	return result;
}

void YaContactListModel::fakeContactDestroyed(PsiContact* fakeContact)
{
	fakeGroupContacts_.removeAll(fakeContact);
}

QModelIndexList YaContactListModel::indexesFor(PsiContact* contact, QMimeData* contactSelection) const
{
	QModelIndexList indexes;
	if (contactSelection) {
		indexes += indexesFor(contactSelection);
	}
	if (indexes.isEmpty() && contact) {
		indexes += ContactListModel::indexesFor(contact);
	}
	return indexes;
}
