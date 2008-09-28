/*
 * contactlistgroup.h - flat contact list group class
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

#ifndef CONTACTLISTGROUP_H
#define CONTACTLISTGROUP_H

#include "contactlistitem.h"

#include <QVector>
#include <QModelIndex>

class QTimer;

class PsiContact;
class ContactListModel;
class ContactListItemMenu;
class ContactListItemProxy;

class ContactListGroup : public ContactListItem
{
	Q_OBJECT
public:
	ContactListGroup(ContactListModel* model, ContactListGroup* parent);
	~ContactListGroup();

	ContactListItemProxy* item(int index) const;
	int itemsCount() const;
	int indexOf(const ContactListItem* item) const;

	ContactListModel* model() const { return model_; }

	ContactListGroup* parent() const;
	QModelIndex toModelIndex() const;

	bool isFake() const;
	bool haveOnlineContacts() const;
	int contactsCount() const;

	// QStringList:
	// 1. Пустой, значит добавлять ни в коем случае не надо
	// 2. Элемент — пустая строка — это значит что надо добавлять в General-группу
	// 3. Элементы — непустые строки — названия групп, включая сепараторы (сплитить отдельно)
	virtual void addContact(PsiContact* contact, QStringList contactGroups);
	virtual void contactUpdated(PsiContact* contact);
	// модель фильтрует группы перед запуском contactGroupsChanged
	virtual void contactGroupsChanged(PsiContact* contact, QStringList contactGroups);

	QString fullName() const;

	// reimplemented
	virtual ContactListModel::Type type() const;
	virtual const QString& name() const;
	virtual void setName(const QString& name);
	virtual bool isEditable() const;
	virtual bool isRemovable() const;
	virtual bool isExpandable() const;
	virtual bool expanded() const;
	virtual void setExpanded(bool expanded);
	virtual ContactListItemMenu* contextMenu(ContactListModel* model);
	virtual bool compare(const ContactListItem* other) const;

	static const QString& groupDelimiter();
	static void setGroupDelimiter(const QString&);
	QString sanitizeGroupName(const QString&) const;
	QStringList sanitizeGroupNames(const QStringList& names) const;

#ifdef UNIT_TEST
	void dumpTree() const;
#endif

protected:
	void addItem(ContactListItemProxy* item);
	void removeItem(ContactListItemProxy* item);
	ContactListItemProxy* findContact(PsiContact* contact) const;
	ContactListItemProxy* findGroup(ContactListGroup* group) const;
	const QVector<ContactListItemProxy*>& items() const;
	void internalSetName(const QString& name);

public slots:
	void updateOnlineContactsFlag();

private:
	ContactListModel* model_;
	ContactListGroup* parent_;
	QTimer* updateOnlineContactsTimer_;
	QString name_;
	QVector<PsiContact*> contacts_;
	QVector<ContactListItemProxy*> items_;
	bool haveOnlineContacts_;

	void removeContact(PsiContact* contact);
#ifdef UNIT_TEST
	void dumpInfo(const ContactListItem* item, int indent) const;
	void dumpTree(int indent) const;
#endif
};

#endif
