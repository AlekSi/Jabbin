/*
 * yaprivacymanager.cpp - an iq:privacy manager with simple API
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

#include "psiaccount.h"
#include "yaprivacymanager.h"
#include "privacylist.h"
#include "xmpp_client.h"
#include "xmpp_status.h"
#include "xmpp_tasks.h"
#include "userlist.h"

static const QString BLOCKED_LIST_NAME = "blocked";

static XMPP::Jid processJid(const XMPP::Jid& jid)
{
	return jid.bare();
}

static bool privacyListItemForJid(const PrivacyListItem& item, const Jid& jid)
{
	return item.type() == PrivacyListItem::JidType &&
	       processJid(item.value()) == processJid(jid);
}

YaPrivacyManager::YaPrivacyManager(PsiAccount* account)
	: PsiPrivacyManager(account->client()->rootTask())
	, account_(account)
	, accountAvailable_(false)
	, isAvailable_(false)
{
	connect(account, SIGNAL(updatedActivity()), SLOT(accountStateChanged()));

	connect(this, SIGNAL(listReceived(const PrivacyList&)), SLOT(listReceived(const PrivacyList&)));
	connect(this, SIGNAL(listsReceived(const QString&, const QString&, const QStringList&)), SLOT(listsReceived(const QString&, const QString&, const QStringList&)));
	connect(this, SIGNAL(listsError()), SLOT(listsError()));

	connect(this, SIGNAL(changeDefaultList_success()), SLOT(changeDefaultList_success()));
	connect(this, SIGNAL(changeDefaultList_error()), SLOT(changeDefaultList_error()));
	connect(this, SIGNAL(changeActiveList_success()), SLOT(changeActiveList_success()));
	connect(this, SIGNAL(changeActiveList_error()), SLOT(changeActiveList_error()));
}

YaPrivacyManager::~YaPrivacyManager()
{
	qDeleteAll(lists_);
}

bool YaPrivacyManager::isAvailable() const
{
	return isAvailable_;
}

void YaPrivacyManager::setIsAvailable(bool available)
{
	if (available != isAvailable_) {
		isAvailable_ = available;
		emit availabilityChanged();
	}
}

void YaPrivacyManager::accountStateChanged()
{
	if (!account_->isAvailable()) {
		setIsAvailable(false);
	}

	if (account_->isAvailable() && !accountAvailable_) {
		requestListNames();
	}

	accountAvailable_ = account_->isAvailable();
}

static QStringList findDifferences(QStringList previous, QStringList current)
{
	QStringList result;
	foreach(QString i, previous) {
		if (!current.contains(i))
			result += i;
	}
	return result;
}

void YaPrivacyManager::listReceived(const PrivacyList& list)
{
	QStringList previouslyBlockedContacts = blockedContacts();

	if (lists_.contains(list.name()))
		*lists_[list.name()] = list;
	else
		lists_[list.name()] = new PrivacyList(list);

	if (list.name() == BLOCKED_LIST_NAME)
		invalidateBlockedListCache();

	QStringList currentlyBlockedContacts = blockedContacts();
	QStringList updatedContacts;
	updatedContacts += findDifferences(previouslyBlockedContacts, currentlyBlockedContacts);
	updatedContacts += findDifferences(currentlyBlockedContacts, previouslyBlockedContacts);

	foreach(QString contact, updatedContacts) {
		emit simulateContactOffline(contact);

		if (!isContactBlocked(contact)) {
			if (isAuthorized(contact)) {
				JT_Presence* p = new JT_Presence(account_->client()->rootTask());
				p->pres(processJid(contact), account_->status());
				p->go(true);
			}

			{
				JT_Presence* p = new JT_Presence(account_->client()->rootTask());
				p->probe(processJid(contact));
				p->go(true);
			}
		}
	}

	emit listChanged(updatedContacts);
}

void YaPrivacyManager::listsReceived(const QString& defaultList, const QString& activeList, const QStringList& lists)
{
	if (!lists.contains(BLOCKED_LIST_NAME)) {
		createBlockedList();
	}
	else {
		requestList(BLOCKED_LIST_NAME);
	}

	if (defaultList.isEmpty()) {
		changeDefaultList(BLOCKED_LIST_NAME);
	}

	changeActiveList(BLOCKED_LIST_NAME);

	setIsAvailable(true);
}

void YaPrivacyManager::listsError()
{
	setIsAvailable(false);
}

void YaPrivacyManager::changeDefaultList_success()
{
}

void YaPrivacyManager::changeDefaultList_error()
{
	qWarning("YaPrivacyManager::changeDefaultList_error()");
}

void YaPrivacyManager::changeActiveList_success()
{
}

void YaPrivacyManager::changeActiveList_error()
{
	qWarning("YaPrivacyManager::changeActiveList_error()");
}

void YaPrivacyManager::createBlockedList()
{
	PrivacyList list(BLOCKED_LIST_NAME);
	PrivacyListItem allowAll;
	allowAll.setType(PrivacyListItem::FallthroughType);
	allowAll.setAction(PrivacyListItem::Allow);
	allowAll.setAll();

	list.insertItem(0, allowAll);
	changeList(list);
}

PrivacyList* YaPrivacyManager::blockedList() const
{
	return lists_[BLOCKED_LIST_NAME];
}

QStringList YaPrivacyManager::blockedContacts() const
{
	QStringList result;
	if (blockedList()) {
		foreach(PrivacyListItem item, blockedList()->items()) {
			if (item.type() == PrivacyListItem::JidType &&
			    item.action() == PrivacyListItem::Deny) {
				result << processJid(item.value()).full();
			}
		}
	}
	return result;
}

void YaPrivacyManager::invalidateBlockedListCache()
{
	isBlocked_.clear();

	if (!blockedList())
		return;

	foreach(PrivacyListItem item, blockedList()->items()) {
		if (item.type() == PrivacyListItem::JidType &&
		    item.action() == PrivacyListItem::Deny) {
			isBlocked_[processJid(item.value()).full()] = true;
		}
	}
}

bool YaPrivacyManager::isContactBlocked(const XMPP::Jid& jid) const
{
	return isBlocked_.contains(processJid(jid).full());
}

PrivacyListItem YaPrivacyManager::blockItemFor(const XMPP::Jid& jid) const
{
	PrivacyListItem item;
	item.setType(PrivacyListItem::JidType);
	item.setValue(processJid(jid).full());
	item.setAction(PrivacyListItem::Deny);
	item.setAll();
	return item;
}

void YaPrivacyManager::setContactBlocked(const XMPP::Jid& jid, bool blocked)
{
	if (!blockedList() || isContactBlocked(jid) == blocked)
		return;

	if (blocked && isAuthorized(jid)) {
		JT_Presence* p = new JT_Presence(account_->client()->rootTask());
		p->pres(processJid(jid), account_->loggedOutStatus());
		p->go(true);
	}

	PrivacyList newList(*blockedList());
	newList.clear();

	foreach(PrivacyListItem item, blockedList()->items()) {
		if (privacyListItemForJid(item, jid))
			continue;

		newList.appendItem(item);
	}

	if (blocked)
		newList.insertItem(0, blockItemFor(jid));

	changeList(newList);
}

bool YaPrivacyManager::isAuthorized(const XMPP::Jid& jid) const
{
	UserListItem* u = account_->findFirstRelevant(processJid(jid));
	return u && (u->subscription().type() == Subscription::Both ||
	             u->subscription().type() == Subscription::From);
}
