/*
 * contactlistproxymodel.cpp - contact list model sorting and filtering
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

#include "contactlistproxymodel.h"

#include "psicontactlist.h"
#include "psicontact.h"
#include "contactlistitem.h"
#include "contactlistgroup.h"
#include "contactlistitemproxy.h"

ContactListProxyModel::ContactListProxyModel(QObject* parent, PsiContactList* contactList)
	: QSortFilterProxyModel(parent)
{
	sort(0, Qt::AscendingOrder);
	setDynamicSortFilter(true);
}

void ContactListProxyModel::setSourceModel(QAbstractItemModel* model)
{
	Q_ASSERT(dynamic_cast<ContactListModel*>(model));
	QSortFilterProxyModel::setSourceModel(model);
	connect(model, SIGNAL(showOfflineChanged()), SLOT(filterParametersChanged()));
}

bool ContactListProxyModel::showOffline() const
{
	return static_cast<ContactListModel*>(sourceModel())->showOffline();
}

bool ContactListProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
	if (!index.isValid())
		return false;

	ContactListItemProxy* itemProxy = static_cast<ContactListItemProxy*>(index.internalPointer());
	ContactListItem* item = itemProxy ? itemProxy->item() : 0;
	if (!item) {
		Q_ASSERT(false);
		return false;
	}

	if (item->editing()) {
		return true;
	}

	switch (ContactListModel::indexType(index)) {
	case ContactListModel::ContactType: {
		PsiContact* psiContact = dynamic_cast<PsiContact*>(item);

		if (psiContact->isHidden()) {
			return false;
		}

		if (!showOffline()) {
			return psiContact->isOnline();
		}
		else {
			return true;
		}
	}
	case ContactListModel::GroupType:
		if (!showOffline()) {
			ContactListGroup* group = dynamic_cast<ContactListGroup*>(item);
			return group->haveOnlineContacts();
		}
		else {
			return true;
		}
	default:
		Q_ASSERT(false);
	}

	return true;
}

bool ContactListProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
	ContactListItemProxy* item1 = static_cast<ContactListItemProxy*>(left.internalPointer());
	ContactListItemProxy* item2 = static_cast<ContactListItemProxy*>(right.internalPointer());
	if (!item1 || !item2)
		return false;
	return item1->item()->compare(item2->item());
}

void ContactListProxyModel::filterParametersChanged()
{
	invalidateFilter();
}

void ContactListProxyModel::updateSorting()
{
	invalidate();
}
