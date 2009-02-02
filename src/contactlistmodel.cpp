/*
 * contactlistmodel.cpp - model of contact list
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

#include "contactlistmodel.h"

#include <QModelIndex>
#include <QVariant>
#include <QColor>
#include <QMessageBox>

#include "psiaccount.h"
#include "psicontact.h"
#include "psicontactlist.h"
#include "contactlistitem.h"
#include "contactlistgroup.h"
#include "contactlistnestedgroup.h"
#include "contactlistitemproxy.h"
#include "contactlistgroupstate.h"
#include "contactlistgroupcache.h"
#include "contactlistmodelupdater.h"

ContactListModel::ContactListModel(PsiContactList* contactList)
	: QAbstractItemModel(contactList)
	, groupsEnabled_(false)
	, contactList_(contactList)
	, updater_(0)
	, rootGroup_(0)
	, bulkUpdateCount_(0)
	, emitDeltaSignals_(true)
	, groupState_(0)
	, groupCache_(0)
{
	groupState_ = new ContactListGroupState(this);
	updater_ = new ContactListModelUpdater(contactList, this);
	groupCache_ = new ContactListGroupCache(this);

	connect(groupState_, SIGNAL(orderChanged()), SLOT(orderChanged()));
	connect(updater_, SIGNAL(addedContact(PsiContact*)), SLOT(addContact(PsiContact*)));
	connect(updater_, SIGNAL(removedContact(PsiContact*)), SLOT(removeContact(PsiContact*)));
	connect(updater_, SIGNAL(contactUpdated(PsiContact*)), SLOT(contactUpdated(PsiContact*)));
	connect(updater_, SIGNAL(contactGroupsChanged(PsiContact*)), SLOT(contactGroupsChanged(PsiContact*)));
	connect(updater_, SIGNAL(beginBulkContactUpdate()), SLOT(beginBulkUpdate()));
	connect(updater_, SIGNAL(endBulkContactUpdate()), SLOT(endBulkUpdate()));
	connect(contactList, SIGNAL(destroying()), SLOT(destroyingContactList()));
	connect(contactList, SIGNAL(showOfflineChanged(bool)), SIGNAL(showOfflineChanged()));
	connect(contactList, SIGNAL(rosterRequestFinished()), SLOT(rosterRequestFinished()));
}

ContactListModel::~ContactListModel()
{
	emitDeltaSignals_ = false;
	delete rootGroup_;
}

void ContactListModel::storeGroupState(const QString& id)
{
	groupState_->load(id);
}

void ContactListModel::updaterCommit()
{
	if (!updater()) {
		Q_ASSERT(false);
		return;
	}
	updater()->commit();
}

void ContactListModel::destroyingContactList()
{
	updater_->commit();
	delete updater_;

	groupState_->updateGroupList(this);
	groupState_->save();

	contactList_ = 0;
	invalidateLayout();
}

ContactListGroup* ContactListModel::createRootGroup()
{
	if (!groupsEnabled_)
		return new ContactListGroup(this, 0);

	return new ContactListNestedGroup(this, 0, QString());
}

ContactListModel* ContactListModel::clone() const
{
	return new ContactListModel(contactList());
}

bool ContactListModel::groupsEnabled() const
{
	return groupsEnabled_;
}

void ContactListModel::setGroupsEnabled(bool enabled)
{
	if (groupsEnabled_ != enabled) {
		groupsEnabled_ = enabled;
		invalidateLayout();
	}
}

void ContactListModel::beginBulkUpdate()
{
	Q_ASSERT(bulkUpdateCount_ >= 0);
	if (!bulkUpdateCount_) {
		emitDeltaSignals_ = false;
		doResetAfterBulkUpdate_ = false;
		doLayoutUpdateAfterBulkUpdate_ = false;

		emit layoutAboutToBeChanged();
	}

	++bulkUpdateCount_;
}

void ContactListModel::endBulkUpdate()
{
	--bulkUpdateCount_;
	Q_ASSERT(bulkUpdateCount_ >= 0);

	if (!bulkUpdateCount_) {
		emitDeltaSignals_ = true;
		if (doResetAfterBulkUpdate_) {
			reset();

			// in Qt 4.3.4 emitting modelReset() leads to QSortFilterProxyModel
			// calling invalidate() on itself first, then immediately it will emit
			// modelReset() too.
			//
			// It's very easy to reproduce on yapsi r2193 and Qt 4.3.4: enable groups in roster,
			// then disable them. And all you could see now is blank contacts roster.
			emit layoutAboutToBeChanged();
			emit layoutChanged();
		}
		else if (doLayoutUpdateAfterBulkUpdate_) {
			emit layoutChanged();
		}
	}
}

void ContactListModel::rosterRequestFinished()
{
	if (rowCount(QModelIndex()) == 0) {
		reset();

		emit layoutAboutToBeChanged();
		emit layoutChanged();
	}
}

void ContactListModel::orderChanged()
{
	emit layoutAboutToBeChanged();
	emit layoutChanged();
}

void ContactListModel::invalidateLayout()
{
	beginBulkUpdate();

	if (rootGroup_)
		delete rootGroup_;

	rootGroup_ = createRootGroup();

	if (contactList_)
		foreach(PsiContact* contact, contactList_->contacts())
			addContact(contact);

	foreach(PsiContact* contact, additionalContacts())
		addContact(contact);

	endBulkUpdate();
}

QStringList ContactListModel::filterContactGroups(QStringList groups) const
{
	return groups;
}

void ContactListModel::addContact(PsiContact* contact)
{
	addContact(contact, contact->groups());
}

void ContactListModel::removeContact(PsiContact* contact)
{
	contactGroupsChanged(contact, QStringList());
}

void ContactListModel::contactGroupsChanged(PsiContact* contact)
{
	Q_ASSERT(rootGroup_);
	Q_ASSERT(contact);
	contactGroupsChanged(contact, contact->groups());
}

void ContactListModel::addContact(PsiContact* contact, QStringList contactGroups)
{
	Q_ASSERT(rootGroup_);
	rootGroup_->addContact(contact, rootGroup_->sanitizeGroupNames(filterContactGroups(contactGroups)));
}

QModelIndexList ContactListModel::indexesFor(PsiContact* contact) const
{
	Q_ASSERT(contact);
	QModelIndexList result;
	foreach(ContactListGroup* group, groupCache()->groupsFor(contact)) {
		result += index(group->indexOf(contact), 0, group->toModelIndex());
	}
	return result;
}

void ContactListModel::contactUpdated(PsiContact* contact)
{
	Q_ASSERT(rootGroup_);
	// rootGroup_->contactUpdated(contact);
	foreach(ContactListGroup* group, groupCache()->groupsFor(contact)) {
		group->contactUpdated(contact);
	}
}

void ContactListModel::contactGroupsChanged(PsiContact* contact, QStringList contactGroups)
{
	Q_ASSERT(rootGroup_);
	rootGroup_->contactGroupsChanged(contact, rootGroup_->sanitizeGroupNames(filterContactGroups(contactGroups)));
}

void ContactListModel::itemAboutToBeInserted(ContactListGroup* group, int index)
{
	doResetAfterBulkUpdate_ = true;

	if (!emitDeltaSignals_)
		return;

	beginInsertRows(group->toModelIndex(), index, index);
}

void ContactListModel::insertedItem(ContactListGroup* group, int index)
{
	Q_ASSERT(doResetAfterBulkUpdate_);
	Q_UNUSED(group);
	Q_UNUSED(index);

	if (!emitDeltaSignals_)
		return;

	endInsertRows();
}

void ContactListModel::itemAboutToBeRemoved(ContactListGroup* group, int index)
{
	doResetAfterBulkUpdate_ = true;

	if (!emitDeltaSignals_)
		return;

	beginRemoveRows(group->toModelIndex(), index, index);
}

void ContactListModel::removedItem(ContactListGroup* group, int index)
{
	Q_ASSERT(doResetAfterBulkUpdate_);
	Q_UNUSED(group);
	Q_UNUSED(index);

	if (!emitDeltaSignals_)
		return;

	endRemoveRows();
}

void ContactListModel::updatedItem(ContactListItemProxy* item)
{
	doLayoutUpdateAfterBulkUpdate_ = true;

	if (!emitDeltaSignals_)
		return;

	QModelIndex index = itemProxyToModelIndex(item);
	emit dataChanged(index, index);
}

void ContactListModel::updatedGroupVisibility(ContactListGroup* group)
{
	doLayoutUpdateAfterBulkUpdate_ = true;

	if (!emitDeltaSignals_)
		return;

	Q_UNUSED(group);
	// FIXME: Ideally this should work, but in current Qt it doesn't
	// updatedItem(group->parent()->findGroup(group));
	emit layoutAboutToBeChanged();
	emit layoutChanged();
}

QVariant ContactListModel::contactListItemData(const ContactListItem* item, int role)
{
	Q_ASSERT(item);
	if (role == TypeRole) {
		return QVariant(item->type());
	}
	else if ((role == Qt::DisplayRole || role == Qt::EditRole) /*&& index.column() == NameColumn*/) {
		return QVariant(item->name());
	}
	else if (role == ExpandedRole) {
		return QVariant(item->expanded());
	}

	return QVariant();
}

QVariant ContactListModel::contactGroupData(const ContactListGroup* group, int role)
{
	Q_ASSERT(group);

	if (role == TotalItemsRole) {
		return QVariant(group->itemsCount());
	}
	else if (role == FullGroupNameRole) {
		return QVariant(group->fullName());
	}

	return contactListItemData(group, role);
}

QVariant ContactListModel::contactData(const PsiContact* contact, int role)
{
	Q_ASSERT(contact);

	if (role == JidRole /*&& index.column() == NameColumn*/) {
		return QVariant(contact->jid().full());
	}
	else if (role == PictureRole /*&& index.column() == NameColumn*/) {
		return QVariant(contact->picture());
	}
	else if (role == StatusTextRole) {
		return QVariant(contact->status().status().simplified());
	}
	else if (role == StatusTypeRole) {
		return QVariant(contact->status().type());
	}
	else if (role == CallableRole) {
		return QVariant(contact->isCallable());
	}
#ifdef YAPSI
	else if (role == NotifyValueRole) {
		return QVariant(contact->status().notifyValue());
	}
	else if (role == GenderRole) {
		return QVariant(contact->gender());
	}
#endif

	return contactListItemData(contact, role);
}

QVariant ContactListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	ContactListItemProxy* item = itemProxy(index);
	Q_ASSERT(item);
	Q_ASSERT(item->item());
	if (PsiContact* contact = dynamic_cast<PsiContact*>(item->item())) {
		return contactData(contact, role);
	}
	else if (ContactListGroup* group = dynamic_cast<ContactListGroup*>(item->item())) {
		return contactGroupData(group, role);
	}

	return contactListItemData(item->item(), role);
}

bool ContactListModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
	if (!index.isValid())
		return false;

	ContactListItemProxy* item = itemProxy(index);
	if (!item)
		return false;
	ContactListGroup*     group   = 0;
	PsiContact*           contact = 0;

	if (role == ActivateRole) {
		PsiContact* contact = dynamic_cast<PsiContact*>(item->item());
		if (!contact)
			return false;

		contact->activate();
		return true;
	}
	else if (role == Qt::EditRole) {
		QString name = data.toString();
		if ((contact = dynamic_cast<PsiContact*>(item->item()))) {
			if (name.isEmpty()) {
				// QMessageBox::information(0, tr("Error"), tr("You can't set a blank name."));
				return false;
			}
			else {
				contact->setName(name);
			}
		}
		else if ((group = dynamic_cast<ContactListGroup*>(item->item()))) {
			// if (name.isEmpty()) {
			// 	QMessageBox::information(0, tr("Error"), tr("You can't set a blank group name."));
			// 	return false;
			// }
			// else {
				// // make sure we don't have it already
				// if (group->account()->groupList().contains(name)) {
				// 	QMessageBox::information(0, tr("Error"), tr("You already have a group with that name."));
				// 	return false;
				// }
				group->setName(name);
			// }
		}
		emit dataChanged(index, index);
		return true;
	}
	else if (role == ExpandedRole) {
		if ((group = dynamic_cast<ContactListGroup*>(item->item()))) {
			group->setExpanded(data.toBool());
		}
	}

	return true;
}

QVariant ContactListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(section);
	Q_UNUSED(orientation);
	Q_UNUSED(role);
	return QVariant();
}

/**
 * Returns the model item index for the item in the \param parent
 * with the given \param row and \param column.
 */
QModelIndex ContactListModel::index(int row, int column, const QModelIndex &parent) const
{
	if (column < 0 || column >= 1 || row < 0 || parent.column() > 0)
		return QModelIndex();

	ContactListGroup* group = 0;
	if (!parent.isValid()) {
		group = rootGroup_;
	}
	else {
		ContactListItemProxy* item = itemProxy(parent);
		group = dynamic_cast<ContactListGroup*>(item->item());
	}

	if (group && row < group->itemsCount())
		return itemProxyToModelIndex(group->item(row), row);

	return QModelIndex();
}

/**
 * Return the parent of the given \param index model item.
 */
QModelIndex ContactListModel::parent(const QModelIndex &index) const
{
	if (!index.isValid())
		return QModelIndex();

	ContactListItemProxy* item = itemProxy(index);
	if (item && item->parent())
		return item->parent()->toModelIndex();

	return QModelIndex();
}

/**
 * Returns the number of rows in the \param parent model index.
 */
int ContactListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;

	ContactListGroup* group = 0;
	if (parent.isValid()) {
		ContactListItemProxy* item = itemProxy(parent);
		group = dynamic_cast<ContactListGroup*>(item->item());
	}
	else {
		group = rootGroup_;
	}

	return group ? group->itemsCount() : 0;
}

/**
 * Returns the number of columns in the \param parent model item.
 */
int ContactListModel::columnCount(const QModelIndex& parent) const
{
	if (parent.column() > 0)
		return 0;
	return 1;
}

/**
 * Returns the item flags for the given \param index in the model.
 */
Qt::ItemFlags ContactListModel::flags(const QModelIndex& index) const
{
	if (!index.isValid())
		return Qt::ItemIsDropEnabled;

	Qt::ItemFlags f = QAbstractItemModel::flags(index);
	f |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;

	ContactListItemProxy* item = itemProxy(index);
	if ((index.column() == NameColumn) && item && item->item()->isEditable())
		f |= Qt::ItemIsEditable;
	return f;
}

/**
 * Returns true if the \param parent model item has children; otherwise
 * returns false.
 */
bool ContactListModel::hasChildren(const QModelIndex& parent)
{
	return rowCount(parent) > 0;
}

/**
 * Call this slot to notify \param index that it's now displayed in the 'expanded' state.
 */
void ContactListModel::expanded(const QModelIndex& index)
{
	setData(index, QVariant(true), ExpandedRole);
}

/**
* Call this slot to notify \param index that it's now displayed in the 'collapsed' state.
 */
void ContactListModel::collapsed(const QModelIndex& index)
{
	setData(index, QVariant(false), ExpandedRole);
}

PsiContactList* ContactListModel::contactList() const
{
	return contactList_;
}

QModelIndex ContactListModel::itemProxyToModelIndex(ContactListItemProxy* item) const
{
	Q_ASSERT(item);
	int row = item->parent() ? item->parent()->indexOf(item->item()) : 0;
	return itemProxyToModelIndex(item, row);
}

QModelIndex ContactListModel::itemProxyToModelIndex(ContactListItemProxy* item, int index) const
{
	Q_ASSERT(item);
	return createIndex(index, 0, item);
}

void ContactListModel::renameSelectedItem()
{
	emit inPlaceRename();
}

ContactListModel::Type ContactListModel::indexType(const QModelIndex& index)
{
	QVariant type = index.data(ContactListModel::TypeRole);
	if (type.isValid())
		return static_cast<ContactListModel::Type>(type.toInt());

	return ContactListModel::InvalidType;
}

ContactListItemProxy* ContactListModel::itemProxy(const QModelIndex& index) const
{
	if ((index.row() < 0) || (index.column() < 0) || (index.model() != this))
		return 0;
	return static_cast<ContactListItemProxy*>(index.internalPointer());
}

PsiAccount* ContactListModel::account(const QModelIndex& index) const
{
	ContactListItemProxy* item = itemProxy(index);
	if (item) {
		PsiContact* contact = dynamic_cast<PsiContact*>(item->item());
		if (contact)
			return contact->account();
	}
	return 0;
}

ContactListGroupState* ContactListModel::groupState() const
{
	return groupState_;
}

ContactListModelUpdater* ContactListModel::updater() const
{
	return updater_;
}

ContactListGroupCache* ContactListModel::groupCache() const
{
	return groupCache_;
}

QList<PsiContact*> ContactListModel::additionalContacts() const
{
	return QList<PsiContact*>();
}

void ContactListModel::renameGroup(ContactListGroup* group, const QString& newName)
{
	qWarning("ContactListGroup::renameGroup(): Not implemented.");
}

int ContactListModel::groupOrder(const QString& groupFullName) const
{
	ContactListGroup* group = groupCache()->findGroup(groupFullName);
	if (group) {
		return groupState()->groupOrder(group);
	}
	return 0;
}

void ContactListModel::setGroupOrder(const QString& groupFullName, int order)
{
	ContactListGroup* group = groupCache()->findGroup(groupFullName);
	if (group) {
		groupState()->setGroupOrder(group, order);
	}
}

bool ContactListModel::showOffline() const
{
	if (!contactList_)
		return false;
	return contactList_->showOffline();
}

bool ContactListModel::updatesEnabled() const
{
	return updater()->updatesEnabled();
}

void ContactListModel::setUpdatesEnabled(bool updatesEnabled)
{
	updater()->setUpdatesEnabled(updatesEnabled);
}
