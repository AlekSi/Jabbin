/*
 * psicontact.cpp - PsiContact
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

#include "psicontact.h"

#include <QFileDialog>

#include "avatars.h"
#include "common.h"
#include "contactview.h"
#include "groupmenu.h"
#include "iconset.h"
#include "jidutil.h"
#include "profiles.h"
#include "psiaccount.h"
#include "psicontactmenu.h"
#include "psiiconset.h"
#include "psioptions.h"
#include "resourcemenu.h"
#include "userlist.h"
#include "userlist.h"
#include "alertable.h"
#include "avatars.h"
#ifdef YAPSI
#include "yaprivacymanager.h"
#include "yacommon.h"
#include "yaprofile.h"
#endif
#include "contactlistgroup.h"
#include "desktoputil.h"
#include "yaremoveconfirmationmessagebox.h"
#include "vcardfactory.h"
#include "psicon.h"
#include "psicontactlist.h"
#include "capsmanager.h"

static const int statusTimerInterval = 5000;

class PsiContact::Private : public Alertable
{
	Q_OBJECT
public:
	Private(PsiContact* contact)
		: account_(0)
		, statusTimer_(0)
		, contact_(contact)
	{
		oldStatus_ = XMPP::Status(XMPP::Status::Offline);
#ifdef YAPSI
		showOnlineTemporarily_ = false;
		reconnecting_ = false;

#endif

		statusTimer_ = new QTimer(this);
		statusTimer_->setInterval(statusTimerInterval);
		statusTimer_->setSingleShot(true);
		connect(statusTimer_, SIGNAL(timeout()), SLOT(updateStatus()));
	}

	~Private()
	{
	}

	PsiAccount* account_;
	QTimer* statusTimer_;
	UserListItem u_;
	QString name_;
	Status status_;
	Status oldStatus_;
#ifdef YAPSI
	bool showOnlineTemporarily_;
	bool reconnecting_;
#endif

	/**
	 * Returns bare of the base jid combined with \param resource.
	 */
	Jid jidForResource(const QString& resource)
	{
		QString s = u_.jid().bare();
		if (!resource.isEmpty()) {
			s += '/';
			s += resource;
		}
		return Jid(s);
	}

	void setStatus(XMPP::Status status)
	{
		status_ = status;
		reconnecting_ = false;
		if (account_ && !account_->notifyOnline())
			oldStatus_ = status_;
		else
			statusTimer_->start();
	}

private slots:
	void updateStatus()
	{
		showOnlineTemporarily_ = false;
		oldStatus_ = status_;
		emit contact_->updated();
	}

private:
	PsiContact* contact_;

	void alertFrameUpdated()
	{
#ifndef YAPSI
		emit contact_->updated();
#endif
	}
};

/**
 * Creates new PsiContact.
 */
PsiContact::PsiContact(const UserListItem& u, PsiAccount* account)
	: ContactListItem(account)
{
	d = new Private(this);
	d->account_ = account;
	if (d->account_)
		connect(d->account_->avatarFactory(), SIGNAL(avatarChanged(const Jid&)), SLOT(avatarChanged(const Jid&)));
	update(u);

	// updateParent();
}

PsiContact::PsiContact()
	: ContactListItem(0)
{
	d = new Private(this);
	d->account_ = 0;
}

/**
 * Destructor.
 */
PsiContact::~PsiContact()
{
	emit destroyed(this);
	delete d;
}

/**
 * Returns account to which a contact belongs.
 */
PsiAccount* PsiContact::account() const
{
	return d->account_;
}

/**
 * TODO: Think of ways to remove this function.
 */
const UserListItem& PsiContact::userListItem() const
{
	return d->u_;
}

/**
 * This function should be called only by PsiAccount to update
 * PsiContact's current state.
 */
void PsiContact::update(const UserListItem& u)
{
	d->u_ = u;
	Status status = Status(Status::Offline);
	if (d->u_.priority() != d->u_.userResourceList().end())
		status = (*d->u_.priority()).status();

	d->setStatus(status);

	emit updated();
	emit groupsChanged();
}

/**
 * Triggers default action.
 */
void PsiContact::activate()
{
	if (!account())
		return;
	// FIXME: probably should obsolete this option
	// if(option.singleclick)
	// 	return;
	account()->actionDefault(jid());
}

ContactListModel::Type PsiContact::type() const
{
	return ContactListModel::ContactType;
}

/**
 * Returns contact's display name.
 */
const QString& PsiContact::name() const
{
	d->name_ = JIDUtil::nickOrJid(d->u_.name(), jid().full());
	return d->name_;
}

/**
 * Returns contact's Jabber ID.
 */
XMPP::Jid PsiContact::jid() const
{
	return d->u_.jid();
}

/**
 * Returns contact's status.
 */
Status PsiContact::status() const
{
	return d->status_;
}

/**
 * Returns tool tip text for contact in HTML format.
 */
QString PsiContact::toolTip() const
{
	return d->u_.makeTip(true, false);
}

/**
 * Returns contact's avatar picture.
 */
QIcon PsiContact::picture() const
{
	if (!account())
		return QIcon();
	return account()->avatarFactory()->getAvatar(jid().bare());
}

/**
 * Returns contact's status picture, or alert frame, if alert is present.
 */
QIcon PsiContact::statusIcon() const
{
	if (d->alerting())
		return d->currentAlertFrame();

	return PsiIconset::instance()->status(jid(), status()).icon();
}

/**
 * TODO
 */
ContactListItemMenu* PsiContact::contextMenu(ContactListModel* model)
{
	if (!account())
		return 0;
	return new PsiContactMenu(this, model);
}

bool PsiContact::isFake() const
{
	return false;
}

/**
 * Returns true if user could modify (i.e. rename/change group/remove from
 * contact list) this contact on server.
 */
bool PsiContact::isEditable() const
{
	if (!account())
		return false;
#ifdef YAPSI_ACTIVEX_SERVER
	if (account()->psi() &&
	    account()->psi()->contactList() &&
	    account()->psi()->contactList()->onlineAccount() &&
	    !account()->psi()->contactList()->onlineAccount()->isAvailable())
	{
		return false;
	}
#endif
	return account()->isAvailable();
}

/**
 * This function should be invoked when contact is being renamed by
 * user. \param name specifies new name. PsiContact is responsible
 * for the actual roster update.
 */
void PsiContact::setName(const QString& name)
{
	if (account())
		account()->actionRename(jid(), name);
}

static const QString globalGroupDelimiter = "::";
static const QString accountGroupDelimiter = "::";

/**
 * Processes the list of groups to the result ready to be fed into
 * ContactListModel. All account-specific group delimiters are replaced
 * in favor of the global Psi one.
 */
QStringList PsiContact::groups() const
{
	QStringList result;
	if (!account())
		return result;

	if (d->u_.groups().isEmpty()) {
		// empty group name means that the contact should be added
		// to the 'General' group or no group at all
#ifdef USE_GENERAL_CONTACT_GROUP
		result << tr("General");
#else
		result << QString();
#endif
	}
	else {
		foreach(QString group, d->u_.groups()) {
			QString groupName = group.split(accountGroupDelimiter).join(globalGroupDelimiter);
#ifdef USE_GENERAL_CONTACT_GROUP
			if (groupName.isEmpty()) {
				groupName = tr("General");
			}
#endif
			result << groupName;
		}
	}

	return result;
}

/**
 * Sets contact's groups to be \param newGroups. All associations to all groups
 * it belonged to prior to calling this function are lost. \param newGroups
 * becomes the only groups of the contact. Note: \param newGroups should be passed
 * with global Psi group delimiters.
 */
void PsiContact::setGroups(QStringList newGroups)
{
	if (!account())
		return;
	QStringList newAccountGroups;
	foreach(QString group, newGroups) {
		QString groupName = group.split(globalGroupDelimiter).join(accountGroupDelimiter);
#ifdef USE_GENERAL_CONTACT_GROUP
		if (groupName == tr("General")) {
			groupName = QString();
		}
#endif
		newAccountGroups << groupName;
	}

	if (newAccountGroups.count() == 1 && newAccountGroups.first().isEmpty())
		newAccountGroups.clear();

	account()->actionGroupsSet(jid(), newAccountGroups);
}

bool PsiContact::groupOperationPermitted(const QString& oldGroupName, const QString& newGroupName) const
{
#ifdef YAPSI
	// FIXME: make sure YaRoster::addGroup() and YaRoster::removeContact() operate
	// on the same model. And that that model is YaRoster::contactsModel_.
	if (account() && /*account()->isYaAccount() &&*/ Ya::isFriendsGroup(oldGroupName))
		return false;
#endif
	return true;
}

bool PsiContact::isRemovable() const
{
	foreach(QString group, groups()) {
		if (!groupOperationPermitted(group, QString()))
			return false;
	}
#ifdef YAPSI_ACTIVEX_SERVER
	if (account()->psi() &&
	    account()->psi()->contactList() &&
	    account()->psi()->contactList()->onlineAccount() &&
	    !account()->psi()->contactList()->onlineAccount()->isAvailable())
	{
		return false;
	}
#endif
	return true;
}

/**
 * Returns true if contact currently have an alert set.
 */
bool PsiContact::alerting() const
{
	return d->alerting();
}

/**
 * Sets alert icon for contact. Pass null pointer in order to clear alert.
 */
void PsiContact::setAlert(const PsiIcon* icon)
{
	d->setAlert(icon);
	// updateParent();
}

/**
 * Contact should always be visible if it's alerting.
 */
bool PsiContact::shouldBeVisible() const
{
#ifndef YAPSI
	if (d->alerting())
		return true;
#endif
	return false;
	// return ContactListItem::shouldBeVisible();
}

/**
 * Standard desired parent.
 */
// ContactListGroupItem* PsiContact::desiredParent() const
// {
// 	return ContactListContact::desiredParent();
// }

/**
 * Returns true if this contact is the one for the \param jid. In case
 * if reimplemented in metacontact-enabled subclass, it could match
 * several different jids.
 */
bool PsiContact::find(const Jid& jid) const
{
	return this->jid().compare(jid);
}

/**
 * This behaves just like ContactListContact::contactList(), but statically
 * casts returned value to PsiContactList.
 */
// PsiContactList* PsiContact::contactList() const
// {
// 	return static_cast<PsiContactList*>(ContactListContact::contactList());
// }

/**
 * Creates ResourceMenu to select from all online resources for a contact.
 */
ResourceMenu* PsiContact::createResourceMenu(QWidget* parent) const
{
	if (!account())
		return 0;

	ResourceMenu* menu = new ResourceMenu(parent);
	foreach(UserResource resource, d->u_.userResourceList())
		menu->addResource(resource);
	return menu;
}

/**
 * Creates ResourceMenu to select a resource from the list of currently
 * active resources (i.e. with which conversations do currently exist).
 */
ResourceMenu* PsiContact::createActiveChatsMenu(QWidget* parent) const
{
	if (!account())
		return 0;

	ResourceMenu* menu = new ResourceMenu(parent);
	foreach(QString resourceName, account()->hiddenChats(jid())) {
		// determine status
		int status;
		const UserResourceList &rl = d->u_.userResourceList();
		UserResourceList::ConstIterator uit = rl.find(resourceName);
		if (uit != rl.end() || (uit = rl.priority()) != rl.end())
			status = makeSTATUS((*uit).status());
		else
			status = STATUS_OFFLINE;
		menu->addResource(status, resourceName);
	}

	return menu;
}

/**
 * Creates GroupMenu to select new group for a contact.
 */
GroupMenu* PsiContact::createGroupMenu(QWidget* parent) const
{
	if (!account())
		return 0;

	GroupMenu* menu = new GroupMenu(parent);
	menu->addGroup(tr("&None"), "", d->u_.groups().isEmpty());
	menu->addSeparator();

	int n = 0;
	QStringList groupList = account()->groupList();
	groupList.removeAll(tr("Hidden"));
	foreach(QString groupName, groupList) {
		QString accelerator;
		if (n++ < 9)
			accelerator = "&";
		QString text = QString("%1%2. %3").arg(accelerator).arg(n).arg(groupName);
		menu->addGroup(text, groupName, d->u_.groups().contains(groupName));
	}

	// TODO: provide facility to hide unwanted contacts in roster

	return menu;
}

void PsiContact::receiveIncomingEvent()
{
	if (account())
		account()->actionRecvEvent(jid());
}

void PsiContact::sendMessage()
{
	if (account())
		account()->actionSendMessage(jid());
}

void PsiContact::sendMessageTo(QString resource)
{
	if (account())
		account()->actionSendMessage(d->jidForResource(resource));
}

void PsiContact::openCall()
{
	qDebug() << "PsiContact::openCall() : " << account();
	if (account())
		account()->actionMakeCall(jid());
}

void PsiContact::openChat()
{
	if (account())
		account()->actionOpenChat(jid());
}

void PsiContact::openChatTo(QString resource)
{
	if (account())
		account()->actionOpenChatSpecific(d->jidForResource(resource));
}

#ifdef WHITEBOARDING
void PsiContact::openWhiteboard()
{
	if (account())
		account()->actionOpenWhiteboard(jid());
}

void PsiContact::openWhiteboardTo(QString resource)
{
	if (account())
		account()->actionOpenWhiteboardSpecific(d->jidForResource(resource));
}
#endif

void PsiContact::executeCommand(QString resource)
{
	if (account())
		account()->actionExecuteCommandSpecific(d->jidForResource(resource), "");
}

void PsiContact::openActiveChat(QString resource)
{
	if (account())
		account()->actionOpenChatSpecific(d->jidForResource(resource));
}

void PsiContact::sendFile()
{
	if (account())
		account()->actionSendFile(jid());
}

void PsiContact::inviteToGroupchat(QString groupChat)
{
	if (account())
		account()->actionInvite(jid(), groupChat);
}

// TODO: implement PsiAgent class and move these functions there
void PsiContact::logon()
{
	qWarning("PsiContact::logon()");
}

void PsiContact::logoff()
{
	qWarning("PsiContact::logoff()");
}


void PsiContact::toggleBlockedState()
{
	if (!account())
		return;

	YaPrivacyManager* privacyManager = dynamic_cast<YaPrivacyManager*>(account()->privacyManager());
	Q_ASSERT(privacyManager);

	bool blocked = privacyManager->isContactBlocked(jid());
	if (!blocked) {
		YaRemoveConfirmationMessageBox msgBox(
		    tr("Blocking contact"),
		    tr("This will block<br>"
		       "<b>%1</b>"
		       "<br>from seeing your status and sending you messages.").arg(jid().full()),
		    0);
		msgBox.setDestructiveActionName(tr("Block"));
		if (!msgBox.confirmRemoval()) {
			return;
		}
	}

	privacyManager->setContactBlocked(jid(), !blocked);
}

/*!
 * The only way to get dual authorization with XMPP1.0-compliant servers
 * is to request auth first and make a contact accept it, and request it
 * on its own.
 */
void PsiContact::rerequestAuthorizationFrom()
{
	if (account())
		account()->dj_authReq(jid());
}

void PsiContact::removeAuthorizationFrom()
{
	qWarning("PsiContact::removeAuthorizationFrom()");
}

void PsiContact::remove()
{
	if (account())
		account()->actionRemove(jid());
}

void PsiContact::assignCustomPicture()
{
	if (!account())
		return;

	// FIXME: Should check the supported filetypes dynamically
	// FIXME: parent of QFileDialog is NULL, probably should make it psi->contactList()
	QString file = QFileDialog::getOpenFileName(0, tr("Choose an image"), "", tr("All files (*.png *.jpg *.gif)"));
	if (!file.isNull()) {
		account()->avatarFactory()->importManualAvatar(jid(), file);
	}
}

void PsiContact::clearCustomPicture()
{
	if (account())
		account()->avatarFactory()->removeManualAvatar(jid());
}

void PsiContact::userInfo()
{
	qDebug() << "PsiContact::userInfo() 1";
	if (account()) {
		qDebug() << "PsiContact::userInfo() 2";
		account()->actionInfo(jid());
	}
	qDebug() << "PsiContact::userInfo() 3";
}

void PsiContact::history()
{
#ifdef YAPSI
	qDebug() << "History requested... YAPSI";
	if (account())
		Ya::showHistory(account(), jid());
#else
	qDebug() << "History requested... PSI";
	if (account())
		account()->actionHistory(jid());
#endif
}

#ifdef YAPSI
bool PsiContact::isYaInformer() const
{
	return d->u_.groups().contains(Ya::INFORMERS_GROUP_NAME);
}

bool PsiContact::isYaJid()
{
	return Ya::isYaJid(jid().full());
}

YaProfile PsiContact::getYaProfile() const
{
	return YaProfile(account(), jid());
}

void PsiContact::yaProfile()
{
	if (!account() || !isYaJid())
		return;
	getYaProfile().browse();
}

void PsiContact::yaPhotos()
{
	if (!account() || !isYaJid())
		return;
	getYaProfile().browsePhotos();
}

void PsiContact::yaEmail()
{
	// FIXME: use vcard.email()?
	DesktopUtil::openUrl(
	    QString("mailto:%1").arg(jid().bare()));
}

void PsiContact::addRemoveAuthBlockAvailable(bool* addButton, bool* deleteButton, bool* authButton, bool* blockButton) const
{
	*addButton    = false;
	*deleteButton = false;
	*authButton   = false;
	*blockButton  = false;

	if (account() && account()->isAvailable() && !userListItem().isSelf()) {
		UserListItem* u = account()->findFirstRelevant(jid());

		if (!u || !u->inList()) {
			*addButton   = isEditable();
			*blockButton = isEditable();
			*deleteButton = isEditable();
		}
		else {
			if (u->subscription().type() == Subscription::Both) {
				*deleteButton = isEditable();
				*blockButton  = isEditable();
			}
			else {
				*deleteButton = isEditable();
				*authButton   = isEditable();
				*blockButton  = isEditable();
			}
		}

		*deleteButton = *deleteButton && isEditable() && isRemovable();

		YaPrivacyManager* privacyManager = dynamic_cast<YaPrivacyManager*>(account()->privacyManager());
		Q_ASSERT(privacyManager);
		*blockButton = *blockButton && privacyManager->isAvailable();
	}
}

bool PsiContact::addAvailable() const
{
	bool addButton, deleteButton, authButton, blockButton;
	addRemoveAuthBlockAvailable(&addButton, &deleteButton, &authButton, &blockButton);
	return addButton;
}

bool PsiContact::removeAvailable() const
{
	bool addButton, deleteButton, authButton, blockButton;
	addRemoveAuthBlockAvailable(&addButton, &deleteButton, &authButton, &blockButton);
	return deleteButton;
}

bool PsiContact::authAvailable() const
{
	bool addButton, deleteButton, authButton, blockButton;
	addRemoveAuthBlockAvailable(&addButton, &deleteButton, &authButton, &blockButton);
	return authButton;
}

bool PsiContact::blockAvailable() const
{
	bool addButton, deleteButton, authButton, blockButton;
	addRemoveAuthBlockAvailable(&addButton, &deleteButton, &authButton, &blockButton);
	return blockButton;
}
#endif

void PsiContact::avatarChanged(const Jid& j)
{
	if (!j.compare(jid(), false))
		return;
	emit updated();
}

static inline int rankStatus(int status)
{
	switch (status) {
		case XMPP::Status::FFC:       return 0; // YAPSI original: 0;
		case XMPP::Status::Online:    return 0; // YAPSI original: 1;
		case XMPP::Status::Away:      return 1; // YAPSI original: 2;
		case XMPP::Status::XA:        return 1; // YAPSI original: 3;
		case XMPP::Status::DND:       return 0; // YAPSI original: 4;
		case XMPP::Status::Invisible: return 5; // YAPSI original: 5;
		default:
			return 6;
	}
	return 0;
}

bool PsiContact::compare(const ContactListItem* other) const
{
	const ContactListGroup* group = dynamic_cast<const ContactListGroup*>(other);
	if (group) {
		return !group->compare(this);
	}

	const PsiContact* contact = dynamic_cast<const PsiContact*>(other);
	if (contact) {
		int rank = rankStatus(d->oldStatus_.type()) - rankStatus(contact->d->oldStatus_.type());
		if (rank == 0)
			rank = QString::localeAwareCompare(name().lower(), contact->name().lower());
		return rank < 0;
	}

	return ContactListItem::compare(other);
}

static YaPrivacyManager* privacyManager(PsiAccount* account)
{
	return dynamic_cast<YaPrivacyManager*>(account->privacyManager());
}

bool PsiContact::isBlocked() const
{
	return account() && privacyManager(account()) &&
	       privacyManager(account())->isContactBlocked(jid());
}

bool PsiContact::inList() const
{
	return userListItem().inList();
}

bool PsiContact::authorizesToSeeStatus() const
{
	return userListItem().subscription().type() == Subscription::Both ||
	       userListItem().subscription().type() == Subscription::To;
}

bool PsiContact::isOnline() const
{
	return d->status_.type()    != XMPP::Status::Offline ||
	       d->oldStatus_.type() != XMPP::Status::Offline
#ifdef YAPSI
	       || d->showOnlineTemporarily_
	       || d->reconnecting_;
#endif
	;
}

bool PsiContact::isCallable() const
{
    const UserResourceList & list = d->u_.userResourceList();
    foreach (UserResource res, list) {
        if (account()->capsManager()->features(
                    d->u_.jid().withResource(res.name())).canVoice()) return true;

    }
    return false;
}

void PsiContact::updateCaps()
{
    if (!isOnline()) return;
    if (!isCallable()) return;

    const UserResourceList & list = d->u_.userResourceList();
    foreach (const UserResource & res, list) {
        qDebug() << "PsiContact::updateCaps:" << jid().bare();
        account()->capsManager()->updateCaps(
            jid(),
            res.status().capsNode(),
            res.status().capsVersion(),
            res.status().capsExt()
        );
    }
}

bool PsiContact::isHidden() const
{
	return false;
}

#ifdef YAPSI
void PsiContact::showOnlineTemporarily()
{
	d->showOnlineTemporarily_ = true;
	d->statusTimer_->start();
	emit updated();
}

void PsiContact::setReconnectingState(bool reconnecting)
{
	d->reconnecting_ = reconnecting;
	emit updated();
}
#endif

void PsiContact::setEditing(bool editing)
{
	if (this->editing() != editing) {
		ContactListItem::setEditing(editing);
		emit updated();
	}
}

#include "psicontact.moc"
