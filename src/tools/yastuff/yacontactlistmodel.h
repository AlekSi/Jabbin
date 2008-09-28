/*
 * yacontactlistmodel.h - custom ContactListModel
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

#ifndef YACONTACTLISTMODEL_H
#define YACONTACTLISTMODEL_H

#include "contactlistmodel.h"

class ContactListItem;
class ContactListGroupItem;
class PsiContactGroup;
class PsiAccount;
class YaContactListModelSelection;
class PsiContact;
class FakeGroupContact;

#include "xmpp_jid.h"

#include <QHash>

class YaContactListModelOperationList
{
public:
	enum Action {
		Copy = 0,
		Move,
		Remove
	};

	struct Operation {
		Operation()
		{}
		Operation(const QString& _groupFrom, const QString& _groupTo)
			: groupFrom(_groupFrom)
			, groupTo(_groupTo)
		{}
		QString groupFrom;
		QString groupTo;
	};

	struct ContactOperation {
		PsiContact* contact;
		QList<Operation> operations;
	};

	YaContactListModelOperationList(Action action);
	YaContactListModelOperationList(Qt::DropAction action);

	Action action() const;

	void addOperation(PsiContact* contact, const QString& groupFrom, const QString& groupTo);
	QList<ContactOperation> operations() const;

	void removeAccidentalContactMoveOperations();

private:
	Action action_;
	QHash<PsiContact*, QList<Operation> > operations_;
};

class YaContactListModel : public ContactListModel
{
	Q_OBJECT
public:
	YaContactListModel(PsiContactList* contactList);

	QModelIndex addGroup();

	// reimplemented
	QVariant data(const QModelIndex& index, int role) const;
	Qt::DropActions supportedDropActions() const;
	Qt::ItemFlags flags(const QModelIndex& index) const;
	QStringList mimeTypes() const;
	QMimeData* mimeData(const QModelIndexList& indexes) const;
	bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
	virtual ContactListModel* clone() const;
	void renameGroup(ContactListGroup* group, const QString& newName);

	QModelIndexList indexesFor(const QMimeData* data) const;
	QModelIndexList indexesFor(PsiContact* contact, QMimeData* contactSelection) const;

	QList<PsiContact*> contactsLostByRemove(const QMimeData* data) const;
	void removeIndexes(const QMimeData* data);

	bool supportsMimeDataOnIndex(const QMimeData* data, const QModelIndex& parent) const;

protected:
	// reimplemented
	virtual QStringList filterContactGroups(QStringList groups) const;

	bool singleAccount() const;

	virtual PsiAccount* getDropAccount(PsiAccount* account, const QModelIndex& parent) const;
	virtual QString getDropGroupName(const QModelIndex& parent) const;

	// reimplemented
	QList<PsiContact*> additionalContacts() const;

private slots:
	void fakeContactDestroyed(PsiContact* fakeContact);

private:
	bool singleAccount_;
	QList<PsiContact*> fakeGroupContacts_;

	enum OperationType {
		Operation_DragNDrop = 0,
		Operation_GroupRename
	};

	FakeGroupContact* addFakeGroupContact(QStringList groups, QString specificGroupName);
	QModelIndex groupToIndex(ContactListGroup* group) const;
	QList<PsiContact*> removeIndexesHelper(const QMimeData* data, bool performRemove);
	YaContactListModelOperationList removeOperationsFor(const QMimeData* data) const;
	void addOperationsForGroupRename(const QString& currentGroupName, const QString& newGroupName, YaContactListModelOperationList* operations) const;
	void performContactOperations(const YaContactListModelOperationList& operations, OperationType operationType);
};

#endif
