/*
 * yacontactlistcontactsmodel.cpp
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

#include "yacontactlistcontactsmodel.h"

#include "psiaccount.h"
#include "psicontactlist.h"
#include "yacommon.h"

YaContactListContactsModel::YaContactListContactsModel(PsiContactList* contactList)
	: YaContactListModel(contactList)
{
}

ContactListModel* YaContactListContactsModel::clone() const
{
	return new YaContactListContactsModel(contactList());
}

QStringList YaContactListContactsModel::filterContactGroups(QStringList groups) const
{
	QStringList result;
	foreach(QString group, YaContactListModel::filterContactGroups(groups)) {
		if (!Ya::isFriendsGroup(group)) {
			result << group;
		}
	}
	return result;
}

Qt::ItemFlags YaContactListContactsModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = YaContactListModel::flags(index);

	if (!index.isValid())
		return f | Qt::ItemIsDropEnabled;

	return f;
}

PsiAccount* YaContactListContactsModel::getDropAccount(PsiAccount* account, const QModelIndex& parent) const
{
	if (parent.isValid())
		return YaContactListModel::getDropAccount(account, parent);

	return account;
}

QString YaContactListContactsModel::getDropGroupName(const QModelIndex& parent) const
{
	if (parent.isValid())
		return YaContactListModel::getDropGroupName(parent);

	return QString();
}
