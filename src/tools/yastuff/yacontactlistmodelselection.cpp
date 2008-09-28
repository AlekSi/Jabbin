/*
 * yacontactlistmodelselection.cpp - stores persistent contact list selections
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

#include "yacontactlistmodelselection.h"

#include <QDomElement>
#include <QStringList>

#include "xmpp_xmlcommon.h"
#include "psiaccount.h"
#include "psicontact.h"
#include "contactlistnestedgroup.h"
#include "contactlistitemproxy.h"

static const QString psiRosterSelectionMimeType = "application/psi-roster-selection";

YaContactListModelSelection::YaContactListModelSelection(QList<ContactListItemProxy*> items)
	: QMimeData()
	, mimeData_(0)
{
	QDomDocument doc;
	QDomElement root = doc.createElement("items");
	root.setAttribute("version", "1.0");
	doc.appendChild(root);

	// TODO: maybe also embed a random instance-specific token to
	// prevent drag'n'drop with other running Psi instances?

	QStringList jids;

	foreach(ContactListItemProxy* itemProxy, items) {
		Q_ASSERT(itemProxy);

		PsiContact* contact = 0;
		ContactListNestedGroup* group = 0;
		if ((contact = dynamic_cast<PsiContact*>(itemProxy->item()))) {
			QDomElement tag = textTag(&doc, "contact", contact->jid().full());
			tag.setAttribute("account", contact->account()->id());
			tag.setAttribute("group", itemProxy->parent() ? itemProxy->parent()->fullName() : "");
			root.appendChild(tag);

			jids << contact->jid().full();
		}
		else if ((group = dynamic_cast<ContactListNestedGroup*>(itemProxy->item()))) {
			QDomElement tag = textTag(&doc, "group", group->fullName());
			root.appendChild(tag);

			jids << group->fullName();
		}
	}

	setText(jids.join(";"));
	setData(psiRosterSelectionMimeType, doc.toByteArray());
}

YaContactListModelSelection::YaContactListModelSelection(const QMimeData* mimeData)
	: QMimeData()
	, mimeData_(mimeData)
{
	const YaContactListModelSelection* other = dynamic_cast<const YaContactListModelSelection*>(mimeData_);
	if (other) {
		mimeData_ = other->mimeData();
	}
}

const QString& YaContactListModelSelection::mimeType()
{
	return psiRosterSelectionMimeType;
}

QDomElement YaContactListModelSelection::rootElementFor(const QMimeData* mimeData) const
{
	QDomDocument doc;
	if (!doc.setContent(mimeData->data(psiRosterSelectionMimeType)))
		return QDomElement();

	QDomElement root = doc.documentElement();
	if (root.tagName() != "items" || root.attribute("version") != "1.0")
		return QDomElement();

	return root;
}

bool YaContactListModelSelection::haveRosterSelectionIn(const QMimeData* mimeData) const
{
	return !rootElementFor(mimeData).isNull();
}

QList<YaContactListModelSelection::Contact> YaContactListModelSelection::contactsFor(const QMimeData* mimeData) const
{
	QList<Contact> result;
	QDomElement root = rootElementFor(mimeData);
	if (root.isNull())
		return result;

	for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;

		if (e.tagName() == "contact") {
			Jid jid = e.text();
			result << Contact(jid.full(),
			                  e.attribute("account"),
			                  e.attribute("group"));
		}
	}

	return result;
}

QList<YaContactListModelSelection::Group> YaContactListModelSelection::groupsFor(const QMimeData* mimeData) const
{
	QList<Group> result;
	QDomElement root = rootElementFor(mimeData);
	if (root.isNull())
		return result;

	for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement e = n.toElement();
		if (e.isNull())
			continue;

		if (e.tagName() == "group") {
			result << Group(e.text());
		}
	}

	return result;
}

const QMimeData* YaContactListModelSelection::mimeData() const
{
	return mimeData_ ? mimeData_ : this;
}

bool YaContactListModelSelection::haveRosterSelection() const
{
	return haveRosterSelectionIn(mimeData());
}

QList<YaContactListModelSelection::Contact> YaContactListModelSelection::contacts() const
{
	return contactsFor(mimeData());
}

QList<YaContactListModelSelection::Group> YaContactListModelSelection::groups() const
{
	return groupsFor(mimeData());
}

bool YaContactListModelSelection::isMultiSelection() const
{
	return (contacts().count() + groups().count()) > 1;
}

void YaContactListModelSelection::debugSelection(QMimeData* data, const QString& name)
{
	qWarning("*** debugSelection %s", qPrintable(name));
	YaContactListModelSelection selection(data);
	foreach(YaContactListModelSelection::Contact c, selection.contacts()) {
		qWarning("\tc: '%s'", qPrintable(c.jid));
	}
	foreach(YaContactListModelSelection::Group g, selection.groups()) {
		qWarning("\tg: '%s'", qPrintable(g.fullName));
	}
}
