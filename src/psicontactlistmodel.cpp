/*
 * psicontactlistmodel.cpp - a ContactListModel subclass that does Psi-specific things
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

#include "psicontactlistmodel.h"

#include <QMessageBox>

#include "psicontactlist.h"

#include "contactlistitem.h"
#include "psiaccount.h"
#include "psicontact.h"
#include "psicontactgroup.h"
#include "iconset.h"
#include "common.h"

/**
 * TODO
 */
PsiContactListModel::PsiContactListModel(PsiContactList* contactList)
	: ContactListModel(contactList)
{
}

/**
 * TODO
 */
QVariant PsiContactListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	ContactListItem* item = static_cast<ContactListItem*>(index.internalPointer());
	PsiAccount*      account = 0;
	PsiContactGroup* group   = 0;
	// PsiContact*      contact = 0;

	if (role == Qt::DisplayRole && index.column() == NameColumn) {
		if ((account = dynamic_cast<PsiAccount*>(item))) {
			return QVariant(account->name());
		}
	}
	else if (role == Qt::DecorationRole && (index.column() == NameColumn)) {
		if ((group = dynamic_cast<PsiContactGroup*>(item))) {
#ifndef YAPSI
			if (index.column() == NameColumn) {
				// TODO: Probably could use QIcon::State::On for open groups
				// and QIcon::State::Off for closed groups
				if ( group->items() == 0 )
					return QVariant(IconsetFactory::icon("psi/groupEmpty").icon());
				else if ( group->expanded() )
					return QVariant(IconsetFactory::icon("psi/groupOpen").icon());
				else
					return QVariant(IconsetFactory::icon("psi/groupClosed").icon());
			}
#endif
		}
		else if ((account = dynamic_cast<PsiAccount*>(item))) {
			if (index.column() == NameColumn) {
				return QVariant(account->statusIcon());
			}
		}
	}
	else if (role == Qt::BackgroundRole) {
		if ((group = dynamic_cast<PsiContactGroup*>(item))) {
			return qVariantFromValue(option.color[cGroupBack]);
		}
		else if ((account = dynamic_cast<PsiAccount*>(item))) {
			return qVariantFromValue(option.color[cProfileBack]);
		}
	}
	else if (role == Qt::ForegroundRole) {
		// if ((contact = dynamic_cast<PsiContact*>(item))) {
		// 	return qVariantFromValue(option.color[cGroupBack]);
		// }
		if ((group = dynamic_cast<PsiContactGroup*>(item))) {
			return qVariantFromValue(option.color[cGroupFore]);
		}
		else if ((account = dynamic_cast<PsiAccount*>(item))) {
			return qVariantFromValue(option.color[cProfileFore]);
		}
	}

	return ContactListModel::data(index, role);
}

/**
 * TODO
 */
bool PsiContactListModel::setData(const QModelIndex& index, const QVariant& data, int role)
{
	if (role == ActivateRole) {
		if (!index.isValid())
			return false;

		ContactListItem* item = static_cast<ContactListItem*>(index.internalPointer());
		PsiContact* contact = dynamic_cast<PsiContact*>(item);
		if (!contact)
			return false;

		contact->activate();
		return true;
	}
	else if (role == Qt::EditRole) {
		ContactListItem* item = static_cast<ContactListItem*>(index.internalPointer());
		PsiContactGroup* group   = 0;
		PsiContact*      contact = 0;
		QString name = data.toString();
		if ((contact = dynamic_cast<PsiContact*>(item))) {
			if (name.isEmpty()) {
				QMessageBox::information(0, tr("Error"), tr("You can't set a blank name."));
				return false;
			}
			else {
				contact->setName(name);
			}
		}
		else if ((group = dynamic_cast<PsiContactGroup*>(item))) {
			if (name.isEmpty()) {
				QMessageBox::information(0, tr("Error"), tr("You can't set a blank group name."));
				return false;
			}
			else {
				// make sure we don't have it already
				if (group->account()->groupList().contains(name)) {
					QMessageBox::information(0, tr("Error"), tr("You already have a group with that name."));
					return false;
				}

				group->setName(name);
			}
		}
		emit dataChanged(index, index);
		return true;
	}

	return ContactListModel::setData(index, data, role);
}

PsiContactList* PsiContactListModel::psiContactList() const
{
	return static_cast<PsiContactList*>(contactList());
}

static int childrenCount(const PsiContactListModel* model, const QModelIndex& root)
{
	int result = 0;
	for (int i = 0; i < model->rowCount(root); ++i) {
		QModelIndex index = model->index(i, 0, root);
		if (model->data(index, ContactListModel::ExpandedRole).toBool()) {
			result += childrenCount(model, index);
		} else {
			result++;
		}
	}
	return result;
}

int PsiContactListModel::visibleItemsCount() const
{
	return childrenCount(this, QModelIndex());
}
