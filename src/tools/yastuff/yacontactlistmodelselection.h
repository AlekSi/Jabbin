/*
 * yacontactlistmodelselection.h - stores persistent contact list selections
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

#ifndef YACONTACTLISTMODELSELECTION_H
#define YACONTACTLISTMODELSELECTION_H

#include <QMimeData>

class ContactListItemProxy;
class QDomElement;

class YaContactListModelSelection : public QMimeData
{
	Q_OBJECT
public:
	YaContactListModelSelection(QList<ContactListItemProxy*> items);
	YaContactListModelSelection(const QMimeData* mimeData);

	static const QString& mimeType();

	struct Contact {
		Contact(QString _jid, QString _account, QString _group)
			: jid(_jid)
			, account(_account)
			, group(_group)
		{}
		QString jid;
		QString account;
		QString group;
	};

	struct Group {
		Group(QString _fullName)
			: fullName(_fullName)
		{}
		QString fullName;
	};

	bool haveRosterSelection() const;

	QList<Contact> contacts() const;
	QList<Group> groups() const;

	bool isMultiSelection() const;

	static void debugSelection(QMimeData* data, const QString& name);

private:
	const QMimeData* mimeData_;

	const QMimeData* mimeData() const;

	QDomElement rootElementFor(const QMimeData* mimeData) const;
	bool haveRosterSelectionIn(const QMimeData* mimeData) const;
	QList<Contact> contactsFor(const QMimeData* mimeData) const;
	QList<Group> groupsFor(const QMimeData* mimeData) const;
};

#endif
