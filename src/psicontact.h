/*
 * psicontact.h - PsiContact
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

#ifndef PSICONTACT_H
#define PSICONTACT_H

#include "contactlistitem.h"
#include "psicontactlist.h"
#include "xmpp_vcard.h"

class GroupMenu;
class PsiIcon;
class PsiAccount;
class ResourceMenu;
class UserListItem;
class YaProfile;

class PsiContact : public ContactListItem
{
	Q_OBJECT
private:
	PsiContact(const PsiContact&);
	PsiContact& operator=(const PsiContact&);

protected:
	PsiContact();

public:
	PsiContact(const UserListItem& u, PsiAccount* account);
	~PsiContact();

	PsiAccount* account() const;
	const UserListItem& userListItem() const;
	virtual void update(const UserListItem& u);

	bool isBlocked() const;
	virtual bool inList() const;
	virtual bool authorizesToSeeStatus() const;
	virtual bool isOnline() const;
	virtual bool isHidden() const;
	virtual bool isCallable() const;

	void activate();

	virtual void setEditing(bool editing);

#ifdef YAPSI
	bool isYaInformer() const;
	bool isYaJid();
	bool addAvailable() const;
	bool removeAvailable() const;
	bool authAvailable() const;
	bool blockAvailable() const;

	void showOnlineTemporarily();
	void setReconnectingState(bool reconnecting);

	void startDelayedMoodUpdate(int timeoutInSecs);
#endif

	virtual bool isFake() const;

	// reimplemented
	virtual ContactListModel::Type type() const;
	virtual const QString& name() const;
	virtual void setName(const QString& name);
	virtual XMPP::Jid jid() const;
	virtual XMPP::Status status() const;
	virtual QString toolTip() const;
	virtual QIcon picture() const;
	virtual QIcon statusIcon() const;
	virtual ContactListItemMenu* contextMenu(ContactListModel* model);
	virtual bool isEditable() const;
	virtual bool compare(const ContactListItem* other) const;
	virtual bool isRemovable() const;

	XMPP::VCard::Gender gender() const;

	bool groupOperationPermitted(const QString& oldGroupName, const QString& newGroupName) const;
	virtual QStringList groups() const;
	virtual void setGroups(QStringList);
	bool alerting() const;
	void setAlert(const PsiIcon* icon);
	bool find(const Jid& jid) const;
	// PsiContactList* contactList() const;
	ResourceMenu* createResourceMenu(QWidget* parent) const;
	ResourceMenu* createActiveChatsMenu(QWidget* parent) const;
	GroupMenu* createGroupMenu(QWidget* parent) const;

protected:
	virtual bool shouldBeVisible() const;
	// virtual ContactListGroupItem* desiredParent() const;

public slots:
	virtual void receiveIncomingEvent();
	virtual void sendMessage();
	virtual void sendMessageTo(QString resource);
	virtual void openChat();
	virtual void openCall();
	virtual void openChatTo(QString resource);
#ifdef WHITEBOARDING
	virtual void openWhiteboard();
	virtual void openWhiteboardTo(QString resource);
#endif
	virtual void executeCommand(QString resource);
	virtual void openActiveChat(QString resource);
	virtual void sendFile();
	virtual void inviteToGroupchat(QString groupChat);
	virtual void logon();
	virtual void logoff();
	virtual void toggleBlockedState();
	virtual void rerequestAuthorizationFrom();
	virtual void removeAuthorizationFrom();
	virtual void remove();
	virtual void assignCustomPicture();
	virtual void clearCustomPicture();
	virtual void userInfo();
	virtual void history();
#ifdef YAPSI
	virtual void yaProfile();
	virtual void yaPhotos();
	virtual void yaEmail();
#endif

private slots:
	void avatarChanged(const Jid&);
#ifdef YAPSI
	void moodUpdate();
#endif

signals:
	void updated();
	void groupsChanged();
#ifdef YAPSI
	void moodChanged(const QString&);
#endif

	/**
	 * This signal is emitted when PsiContact has entered its final
	 * destruction stage.
	 */
	void destroyed(PsiContact*);

private:
	class Private;
	Private *d;

#ifdef YAPSI
	YaProfile getYaProfile() const;
	void addRemoveAuthBlockAvailable(bool* add, bool* remove, bool* auth, bool* block) const;
#endif
};

#endif
