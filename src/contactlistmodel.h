/*
 * contactlistmodel.h - model of contact list
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

#ifndef CONTACTLISTMODEL_H
#define CONTACTLISTMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>

class PsiAccount;
class PsiContact;
class PsiContactList;
class ContactListItem;
class ContactListGroup;
class ContactListItemProxy;
class ContactListGroupState;
class ContactListGroupCache;
class ContactListModelUpdater;

class ContactListModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	enum Type {
		InvalidType = 0,
		ContactType = 1,
		GroupType   = 2,
		AccountType = 3
	};

	static ContactListModel::Type indexType(const QModelIndex& index);

	enum {
		// generic
		TypeRole        = Qt::UserRole + 0,
		ActivateRole    = Qt::UserRole + 1,

		// contacts
		JidRole         = Qt::UserRole + 2,
		PictureRole     = Qt::UserRole + 3,
		StatusTextRole  = Qt::UserRole + 4,
		StatusTypeRole  = Qt::UserRole + 5,
                CallableRole    = Qt::UserRole + 6,

		// groups
		ExpandedRole    = Qt::UserRole + 7,
		TotalItemsRole  = Qt::UserRole + 8,
		FullGroupNameRole = Qt::UserRole + 9,

#ifdef YAPSI
		NotifyValueRole = Qt::UserRole + 10,
		GenderRole      = Qt::UserRole + 11,
		LastRole        = Qt::UserRole + 11
#else
		LastRole        = Qt::UserRole + 9
#endif
	};

	enum {
		NameColumn = 0
	};

	ContactListModel(PsiContactList* contactList);
	virtual ~ContactListModel();

	virtual PsiContactList* contactList() const;

	void invalidateLayout();
	QModelIndex itemProxyToModelIndex(ContactListItemProxy* item) const;
	QModelIndex itemProxyToModelIndex(ContactListItemProxy* item, int index) const;

	virtual ContactListModel* clone() const;

	bool showOffline() const;
	bool groupsEnabled() const;
	void setGroupsEnabled(bool enabled);
	void storeGroupState(const QString& id);

	int groupOrder(const QString& groupFullName) const;
	void setGroupOrder(const QString& groupFullName, int order);

	void renameSelectedItem();

	PsiAccount* account(const QModelIndex& index) const;
	ContactListGroupState* groupState() const;
	ContactListGroupCache* groupCache() const;
	void updaterCommit();

	bool updatesEnabled() const;
	void setUpdatesEnabled(bool updatesEnabled);

	virtual void renameGroup(ContactListGroup* group, const QString& newName);

	QModelIndexList indexesFor(PsiContact* contact) const;

	static QVariant contactListItemData(const ContactListItem* item, int role);
	static QVariant contactData(const PsiContact* contact, int role);
	static QVariant contactGroupData(const ContactListGroup* group, int role);

	// reimplemented
	QVariant data(const QModelIndex& index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
	virtual QModelIndex parent(const QModelIndex& index) const;
	virtual int rowCount(const QModelIndex& parent) const;
	virtual int columnCount(const QModelIndex& parent) const;
	Qt::ItemFlags flags(const QModelIndex& index) const;
	virtual bool setData(const QModelIndex&, const QVariant&, int role);
	virtual bool hasChildren(const QModelIndex& index);

public:
	void itemAboutToBeInserted(ContactListGroup* group, int index);
	void insertedItem(ContactListGroup* group, int index);
	void itemAboutToBeRemoved(ContactListGroup* group, int index);
	void removedItem(ContactListGroup* group, int index);
	void updatedItem(ContactListItemProxy* item);
	void updatedGroupVisibility(ContactListGroup* group);

signals:
	void showOfflineChanged();
	void inPlaceRename();

public slots:
	void expanded(const QModelIndex&);
	void collapsed(const QModelIndex&);

protected slots:
	void addContact(PsiContact*);
	void removeContact(PsiContact*);

	void contactUpdated(PsiContact*);
	void contactGroupsChanged(PsiContact*);

	void destroyingContactList();
	void orderChanged();

protected slots:
	void beginBulkUpdate();
	void endBulkUpdate();
	void rosterRequestFinished();

protected:
	virtual QList<PsiContact*> additionalContacts() const;
	ContactListModelUpdater* updater() const;

private:
	bool groupsEnabled_;
	PsiContactList* contactList_;
	ContactListModelUpdater* updater_;
	ContactListGroup* rootGroup_;
	int bulkUpdateCount_;
	bool doResetAfterBulkUpdate_;
	bool doLayoutUpdateAfterBulkUpdate_;
	bool emitDeltaSignals_;
	ContactListGroupState* groupState_;
	ContactListGroupCache* groupCache_;

protected:
	virtual QStringList filterContactGroups(QStringList groups) const;
	virtual ContactListGroup* createRootGroup();
	ContactListItemProxy* itemProxy(const QModelIndex& index) const;

	void addContact(PsiContact* contact, QStringList contactGroups);
	void contactGroupsChanged(PsiContact* contact, QStringList contactGroups);
};

#endif
