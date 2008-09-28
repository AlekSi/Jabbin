/*
 * yacontactlistfriendsmodel.cpp
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

#include "yacontactlistfriendsmodel.h"

#include "psiaccount.h"
#include "psicontactlist.h"
#include "yacommon.h"
#include "contactlistgroup.h"

YaContactListFriendsModel::YaContactListFriendsModel(PsiContactList* contactList)
	: YaContactListModel(contactList)
{
}

ContactListModel* YaContactListFriendsModel::clone() const
{
	return new YaContactListFriendsModel(contactList());
}

QStringList YaContactListFriendsModel::filterContactGroups(QStringList groups) const
{
	QStringList result;
	foreach(QString group, groups)
		if (Ya::isFriendsGroup(group))
			result << group; // TODO: strip the first group name, to make nested groups possible
	return result;
}

Qt::ItemFlags YaContactListFriendsModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = YaContactListModel::flags(index);

	if (!index.isValid())
		return f | Qt::ItemIsDropEnabled;

	return f;
}

PsiAccount* YaContactListFriendsModel::getDropAccount(PsiAccount* account, const QModelIndex& parent) const
{
	if (parent.isValid())
		return YaContactListModel::getDropAccount(account, parent);

	// FIXME: Return only ya.ru account
	// if (!account->isYaAccount())
	// 	return 0;
	return account;
}

QString YaContactListFriendsModel::getDropGroupName(const QModelIndex& parent) const
{
	if (parent.isValid())
		return YaContactListModel::getDropGroupName(parent);

	return Ya::defaultFriendsGroup();
}
