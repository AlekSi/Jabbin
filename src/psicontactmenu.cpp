/*
 * psicontactmenu.cpp - a PsiContact context menu
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

#include "psicontactmenu.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QPointer>

#include "groupmenu.h"
#include "iconaction.h"
#include "iconset.h"
#include "psiaccount.h"
#include "psicontact.h"
#include "psioptions.h"
#include "resourcemenu.h"
#include "contactlistmodel.h"
#include "shortcutmanager.h"
#include "yaprofile.h"
#include "psicon.h"
#ifdef YAPSI
#include "yatoastercentral.h"
#endif

class InviteToGroupChatMenu : public QMenu
{
	Q_OBJECT
public:
	InviteToGroupChatMenu(QWidget* parent, PsiAccount* account)
		: QMenu(parent)
	{
		foreach(QString groupChat, account->groupchats()) {
			QAction* action = new QAction(groupChat, this);
			addAction(action);
			action->setProperty("groupChat", QVariant(groupChat));
			connect(action, SIGNAL(activated()), SLOT(actionActivated()));
		}
	}

signals:
	void inviteToGroupchat(QString groupChat);

private slots:
	void actionActivated()
	{
		QAction* action = static_cast<QAction*>(sender());
		emit inviteToGroupchat(action->property("groupChat").toString());
	}
};

class PsiContactMenu::Private : public QObject
{
	Q_OBJECT

	QPointer<PsiContact> contact_;
	PsiContactMenu* menu_;

public:
	QAction* rename_;

#ifdef YAPSI
	QAction* openChat_;
	QAction* openHistory_;
	QAction* yaProfile_;
	QAction* yaPhotos_;
	QAction* yaEmail_;
	QAction* add_;
	QAction* remove_;
	QAction* auth_;
	QAction* block_;
	QAction* disableMoodNotifications_;
#endif

public:
	Private(PsiContactMenu* menu, PsiContact* _contact)
		: QObject(0)
		, contact_(_contact)
		, menu_(menu)
	{
		connect(PsiOptions::instance(), SIGNAL(optionChanged(const QString&)), SLOT(optionChanged(const QString&)));
		connect(menu, SIGNAL(aboutToShow()), SLOT(updateActions()));

		connect(contact_, SIGNAL(updated()), SLOT(updateActions()));

		rename_ = new QAction(tr("Re&name"), this);
		rename_->setShortcuts(menu->shortcuts("contactlist.rename"));
		connect(rename_, SIGNAL(activated()), this, SLOT(rename()));

#ifdef YAPSI
		openChat_ = new QAction(tr("&Chat"), this);
		connect(openChat_, SIGNAL(activated()), contact_, SLOT(openChat()));

		openHistory_ = new QAction(tr("&History"), this);
		connect(openHistory_, SIGNAL(activated()), contact_, SLOT(history()));

		yaProfile_ = new QAction(tr("Pro&file"), this);
		connect(yaProfile_, SIGNAL(activated()), contact_, SLOT(yaProfile()));

		yaPhotos_ = new QAction(tr("&Photos"), this);
		connect(yaPhotos_, SIGNAL(activated()), contact_, SLOT(yaPhotos()));

		yaEmail_ = new QAction(tr("Send &E-mail"), this);
		connect(yaEmail_, SIGNAL(activated()), contact_, SLOT(yaEmail()));

		add_ = new QAction(tr("&Add"), this);
		connect(add_, SIGNAL(activated()), SLOT(addContact()));

		remove_ = new QAction(tr("&Remove"), this);
		remove_->setShortcuts(ShortcutManager::instance()->shortcuts("contactlist.delete"));
		connect(remove_, SIGNAL(activated()), SLOT(removeContact()));

		auth_ = new QAction(tr("A&uth"), this);
		connect(auth_, SIGNAL(activated()), contact_, SLOT(rerequestAuthorizationFrom()));

		block_ = new QAction(tr("&Block"), this);
		connect(block_, SIGNAL(activated()), contact_, SLOT(toggleBlockedState()));

		disableMoodNotifications_ = new QAction(tr("Disable mood notifications"), this);
		disableMoodNotifications_->setCheckable(true);
		connect(disableMoodNotifications_, SIGNAL(triggered()), SLOT(disableMoodNotificationsTriggered()));

		updateActions();

		menu->addAction(openChat_);
		menu->addAction(openHistory_);
		menu->addAction(yaProfile_);
		menu->addAction(yaPhotos_);
		menu->addSeparator();
		menu->addAction(rename_);
		menu->addAction(remove_);
		menu->addAction(add_);
		menu->addAction(auth_);
		menu->addAction(block_);
		menu_->addSeparator();
		menu_->addAction(disableMoodNotifications_);

		// menu->addSeparator();
		// menu->addAction(yaEmail_);
#else
	{
		bool isPrivate = false;
		bool isAgent   = false;

		IconAction* act_receiveIncomingEvent = new IconAction("", tr("&Receive incoming event"), menu->shortcut("contactlist.event"), this);
		connect(act_receiveIncomingEvent, SIGNAL(activated()), contact_, SLOT(receiveIncomingEvent()));
		IconAction* act_sendMessage = new IconAction("", "psi/sendMessage", tr("Send &message"), menu->shortcut("contactlist.message"), this);
		connect(act_sendMessage, SIGNAL(activated()), contact_, SLOT(sendMessage()));
		ResourceMenu* act_sendMessageTo = contact->createResourceMenu(menu);
		act_sendMessageTo->setTitle(tr("Send message to"));
		act_sendMessageTo->setEnabled(!act_sendMessageTo->isEmpty());
		connect(act_sendMessageTo, SIGNAL(resourceActivated(QString)), contact_, SLOT(sendMessageTo(QString)));
		IconAction* act_openChat = new IconAction("", "psi/start-chat", tr("Open &chat window"), menu->shortcut("contactlist.chat"), this);
		connect(act_openChat, SIGNAL(activated()), contact_, SLOT(openChat()));
		ResourceMenu* act_openChatTo = contact->createResourceMenu(menu);
		act_openChatTo->setTitle(tr("Open chat to"));
		act_openChatTo->setEnabled(!act_openChatTo->isEmpty());
		connect(act_openChatTo, SIGNAL(resourceActivated(QString)), contact_, SLOT(openChatTo(QString)));
#ifdef WHITEBOARDING
		IconAction* act_openWhiteboard = new IconAction("", "psi/start-chat", tr("Open a &whiteboard"), menu->shortcut("contactlist.whiteboard"), this);
		connect(act_openWhiteboard, SIGNAL(activated()), contact_, SLOT(openWhiteboard()));
		ResourceMenu* act_openWhiteboardTo = contact->createResourceMenu(menu);
		act_openWhiteboardTo->setTitle(tr("Open a whiteboard to"));
		act_openWhiteboardTo->setEnabled(!act_openChatTo->isEmpty());
		connect(act_openWhiteboardTo, SIGNAL(resourceActivated(QString)), contact_, SLOT(openWhiteboardTo(QString)));
#endif
		ResourceMenu* act_executeCommand = contact->createResourceMenu(menu);
		act_executeCommand->setTitle(tr("E&xecute command"));
		act_executeCommand->setEnabled(!act_executeCommand->isEmpty());
		connect(act_executeCommand, SIGNAL(resourceActivated(QString)), contact_, SLOT(executeCommand(QString)));
		ResourceMenu* act_openActiveChat = contact->createActiveChatsMenu(menu);
		act_openActiveChat->setTitle(tr("Active chats"));
		act_openActiveChat->setEnabled(!act_openActiveChat->isEmpty());
		connect(act_openActiveChat, SIGNAL(resourceActivated(QString)), contact_, SLOT(openActiveChat(QString)));
		IconAction* act_sendFile = new IconAction("", "psi/upload", tr("Send &file"), 0, this);
		connect(act_sendFile, SIGNAL(activated()), contact_, SLOT(sendFile()));
		InviteToGroupChatMenu* act_inviteToGroupchat = new InviteToGroupChatMenu(menu, contact->account());
		act_inviteToGroupchat->setTitle(tr("Invite to"));
		act_inviteToGroupchat->setIcon(IconsetFactory::iconPixmap("psi/groupChat"));
		act_inviteToGroupchat->setEnabled(!act_inviteToGroupchat->isEmpty());
		connect(act_inviteToGroupchat, SIGNAL(inviteToGroupchat(QString)), this, SLOT(inviteToGroupchat(QString)));
		GroupMenu* act_group = contact->createGroupMenu(menu);
		act_group->setTitle(tr("Group"));
		act_group->setEnabled(contact->isEditable());
		connect(act_group, SIGNAL(groupActivated(QString)), this, SLOT(setGroup(QString)));
		act_group->addSeparator();
		IconAction* act_createGroup = new IconAction("", tr("&Create new..."), 0, this);
		act_group->addAction(act_createGroup);
		act_createGroup->setEnabled(contact->isEditable());
		connect(act_createGroup, SIGNAL(activated()), this, SLOT(createGroup()));

		// IconAction* act_logon = new IconAction("", tr("&Log on"), menu->shortcut("contactlist.login-transport"), this);
		// act_logon->setEnabled( !avail && online );
		// act_logon->setIcon(PsiIconset::instance()->status(e->u.jid(), STATUS_ONLINE));
		// connect(act_logon, SIGNAL(activated()), contact_, SLOT(logon()));
		// IconAction* act_logoff = new IconAction("", tr("Log off"), menu->shortcut("contactlist.logout-transport"), this);
		// act_logoff->setIcon(PsiIconset::instance()->status(e->u.jid(), STATUS_OFFLINE));
		// connect(act_logoff, SIGNAL(activated()), contact_, SLOT(logoff()));

		// connect(act_resendAuthorizationTo, SIGNAL(activated()), contact_, SLOT(resendAuthorizationTo()));
		// connect(act_rerequestAuthorizationFrom, SIGNAL(activated()), contact_, SLOT(rerequestAuthorizationFrom()));
		// connect(act_removeAuthorizationFrom, SIGNAL(activated()), contact_, SLOT(removeAuthorizationFrom()));
		// IconAction* act_remove = new IconAction("", "psi/remove", tr("Rem&ove"), menu->shortcut("contactlist.delete"), this, "act_remove");
		// act_remove->setEnabled(contact->isEditable());
		// connect(act_remove, SIGNAL(activated()), this, SLOT(remove()));

		IconAction* act_assignCustomPicture = new IconAction("", tr("&Assign Custom Picture"), menu->shortcut("contactlist.assign-custom-avatar"), this);
		connect(act_assignCustomPicture, SIGNAL(activated()), contact_, SLOT(assignCustomPicture()));
		IconAction* act_clearCustomPicture = new IconAction("", tr("&Clear Custom Picture"), menu->shortcut("contactlist.clear-custom-avatar"), this);
		// act_clearCustomPicture->setEnabled(contact->account()->avatarFactory()->hasManualAvatar(u->jid()));
		connect(act_clearCustomPicture, SIGNAL(activated()), contact_, SLOT(clearCustomPicture()));

		IconAction* act_userInfo = new IconAction("", "psi/vCard", tr("User &Info"), menu->shortcut("common.user-info"), this);
		connect(act_userInfo, SIGNAL(activated()), contact_, SLOT(userInfo()));
		IconAction* act_history = new IconAction("", "psi/history", tr("&History"), menu->shortcut("common.history"), this);
		connect(act_history, SIGNAL(activated()), contact_, SLOT(history()));

		// if(!self && !inList && !isPrivate && !option.lockdown.roster) {
		// 	pm.insertItem(IconsetFactory::icon("psi/addContact"), tr("Add/Authorize to contact list"), 10);
		// 	if(!online)
		// 		pm.setItemEnabled(10, false);
		// 	pm.insertSeparator();
		// }
		// 
		// if ( (self  && i->isAlerting()) ||
		//      (!self && e->alerting) ) {
		// 	d->cv->qa_recv->addTo(&pm);
		// 	pm.insertSeparator();
		// }

		// menu->addAction(act_receiveIncomingEvent);
		// menu->addSeparator();
		if (PsiOptions::instance()->getOption("options.ui.message.enabled").toBool())
			menu->addAction(act_sendMessage);
		if (!isPrivate && PsiOptions::instance()->getOption("options.ui.message.enabled").toBool())
			menu->addMenu(act_sendMessageTo);
		menu->addAction(act_openChat);
#ifndef YAPSI
		if (!isPrivate)
			menu->addMenu(act_openChatTo);
#endif
#ifdef WHITEBOARDING
		menu->addAction(act_openWhiteboard);
#ifndef YAPSI
		if (!isPrivate)
			menu->addMenu(act_openWhiteboardTo);
#endif
#endif
#if !defined(YAPSI) || defined(YAPSI_DEV)
		if (!isPrivate)
			menu->addMenu(act_executeCommand);
#endif
		if (!isPrivate && PsiOptions::instance()->getOption("options.ui.menu.contact.active-chats").toBool())
			menu->addMenu(act_openActiveChat);

		// // Voice call
		// if(d->pa->voiceCaller() && !isAgent) {
		// 	pm.insertItem(IconsetFactory::icon("psi/voice"), tr("Voice Call"), 24);
		// 	if(!online) {
		// 		pm.setItemEnabled(24, false);
		// 	}
		// 	else {
		// 		bool hasVoice = false;
		// 		const UserResourceList &rl = u->userResourceList();
		// 		for (UserResourceList::ConstIterator it = rl.begin(); it != rl.end() && !hasVoice; ++it) {
		// 			hasVoice = psiAccount()->capsManager()->features(u->jid().withResource((*it).name())).canVoice();
		// 		}
		// 		pm.setItemEnabled(24,!psiAccount()->capsManager()->isEnabled() || hasVoice);
		// 	}
		// }

		menu->addSeparator();
		if (!isAgent)
			menu->addAction(act_sendFile);
		if (!isPrivate && !isAgent)
			menu->addMenu(act_inviteToGroupchat);

		if (!isAgent)
			menu->addMenu(act_group);

		// if (isAgent) {
		// 	menu->addAction(act_logon);
		// 	menu->addAction(act_logoff);
		// }
		// menu->addSeparator();
		if (!option.lockdown.roster)
			menu->addAction(act_rename);
		// if (inList && !option.lockdown.roster) {
		// 	QMenu* authMenu = menu->addMenu(tr("Authorization"));
		// 	authMenu->setIcon(IconsetFactory::iconPixmap("psi/register"));
		// 	authMenu->addAction(act_resendAuthorizationTo);
		// 	authMenu->addAction(act_rerequestAuthorizationFrom);
		// 	authMenu->addAction(act_removeAuthorizationFrom);
		// }
		// menu->addAction(act_remove);
		menu->addSeparator();
		if (PsiOptions::instance()->getOption("options.ui.menu.contact.custom-picture").toBool()) {
			QMenu* pictureMenu = menu->addMenu(tr("&Picture"));
			pictureMenu->addAction(act_assignCustomPicture);
			pictureMenu->addAction(act_clearCustomPicture);
		}
		// if(PGPUtil::pgpAvailable() && PsiOptions::instance()->getOption("options.ui.menu.contact.custom-pgp-key").toBool()) {
		// 	if(u->publicKeyID().isEmpty())
		// 		pm.insertItem(IconsetFactory::icon("psi/gpg-yes"), tr("Assign Open&PGP key"), 21);
		// 	else
		// 		pm.insertItem(IconsetFactory::icon("psi/gpg-no"), tr("Unassign Open&PGP key"), 22);
		// }
		menu->addAction(act_userInfo);
		menu->addAction(act_history);
	}
#endif
	}

private slots:
	void optionChanged(const QString& option)
	{
		if (option == "options.ya.popups.moods.enable") {
			updateActions();
		}
	}

	void updateActions()
	{
#ifdef YAPSI
		if (!contact_)
			return;

		YaProfile* profile = YaProfile::create(contact_->account(), contact_->jid());
		openHistory_->setEnabled(profile->haveHistory());
		delete profile;

		rename_->setEnabled(contact_->isEditable());
		yaProfile_->setEnabled(contact_->isYaJid());
		yaPhotos_->setEnabled(contact_->isYaJid());
		add_->setVisible(contact_->addAvailable());
		remove_->setVisible(contact_->removeAvailable());
		auth_->setVisible(contact_->authAvailable());
		block_->setEnabled(contact_->blockAvailable());

		block_->setText(contact_->isBlocked() ?
		               tr("&Unblock") : tr("&Block"));

		disableMoodNotifications_->setChecked(contact_->account()->psi()->yaToasterCentral()->moodNotificationsDisabled(contact_->jid()));
		disableMoodNotifications_->setEnabled(PsiOptions::instance()->getOption("options.ya.popups.moods.enable").toBool());
#endif
	}

	void disableMoodNotificationsTriggered()
	{
		if (contact_) {
			contact_->account()->psi()->yaToasterCentral()->setMoodNotificationsDisabled(contact_->jid(), disableMoodNotifications_->isChecked());
		}
	}

	void rename()
	{
		if (contact_) {
			menu_->model()->renameSelectedItem();
		}
	}

	void addContact()
	{
		emit menu_->addSelection();
	}

	void removeContact()
	{
		emit menu_->removeSelection();
	}

#ifndef YAPSI
	void inviteToGroupchat(QString groupChat)
	{
		if (contact_) {
			contact_->inviteToGroupchat(groupChat);

			QMessageBox::information(0, tr("Invitation"),
			tr("Sent groupchat invitation to <b>%1</b>.").arg(contact_->name()));
		}
	}

	void setGroup(QString groupName)
	{
		if (contact_) {
			contact_->setGroups(QStringList() << groupName);
		}
	}

	void createGroup()
	{
		if (contact_) {
			while (1) {
				bool ok = false;
				QString newGroup = QInputDialog::getText(tr("Create New Group"), tr("Enter the new Group name:"), QLineEdit::Normal, QString::null, &ok, 0);
				if (!ok)
					break;

				if (newGroup.isEmpty()) {
					QMessageBox::information(0, tr("Error"), tr("You can't enter a blank group name."));
					continue;
				}

				// make sure we don't have it already
				bool found = false;
				foreach(QString groupName, contact_->account()->groupList()) {
					if(groupName == newGroup) {
						found = true;
						break;
					}
				}

				if (!found) {
					contact_->setGroups(QStringList() << newGroup);
					break;
				}
			}
		}
	}

	void remove()
	{
		if (contact_) {
			int n = QMessageBox::information(0, tr("Remove"),
				tr("Are you sure you want to remove <b>%1</b> from your contact list?").arg(contact_->name()),
				tr("&Yes"), tr("&No"));

			if (n == 0)
				contact_->remove();
		}
	}
#endif
};

/**
 * TODO
 */
PsiContactMenu::PsiContactMenu(PsiContact* contact, ContactListModel* model)
	: ContactListItemMenu(contact, model)
{
	d = new Private(this, contact);
}

/**
 * TODO
 */
PsiContactMenu::~PsiContactMenu()
{
	delete d;
}

// TODO: Need to think of a way to delete an entire selection
// with a single keystroke. Probably it could be menu merging
// based on common actions
QList<QAction*> PsiContactMenu::availableActions() const
{
	QList<QAction*> result;
	foreach(QAction* a, ContactListItemMenu::availableActions()) {
		if (a != d->remove_)
			result << a;
	}
	return result;
}

#include "psicontactmenu.moc"
