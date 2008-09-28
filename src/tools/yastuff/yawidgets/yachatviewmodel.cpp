/*
 * yachatviewmodel.cpp - stores items of custom chatlog
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

#include "yachatviewmodel.h"

#include <QDateTime>
#include <QTextDocument> // for Qt::mightBeRichText()
#include <QTimer>

#include "textutil.h"
#include "yacommon.h"

static const int mergeInterval = 5; // in minutes

//----------------------------------------------------------------------------
// YaChatViewModel::Notice
//----------------------------------------------------------------------------

YaChatViewModel::Notice::Notice(QStandardItemModel* parent, YaChatViewModel::Type type, QString message)
	: QObject(parent)
	, type_(type)
	, shouldBeVisible_(false)
	, priority_(0)
	, message_(message)
	, persistent_(true)
	, model_(parent)
	, item_(0)
{
}

int YaChatViewModel::Notice::priority() const
{
	return priority_;
}

void YaChatViewModel::Notice::setPriority(int priority)
{
	priority_ = priority;
}

bool YaChatViewModel::Notice::shouldBeVisible() const
{
	return shouldBeVisible_;
}

void YaChatViewModel::Notice::setShouldBeVisible(bool visible)
{
	shouldBeVisible_ = visible;
}

bool YaChatViewModel::Notice::isVisible() const
{
	return item_;
}

QString YaChatViewModel::Notice::message() const
{
	return message_;
}

void YaChatViewModel::Notice::setMessage(const QString& message)
{
	message_ = message;
	if (isVisible()) {
		setVisible(false);
		setVisible(true);
	}
}

bool YaChatViewModel::Notice::persistent() const
{
	return persistent_;
}

void YaChatViewModel::Notice::setPersistent(bool persistent)
{
	persistent_ = persistent;
}

void YaChatViewModel::Notice::setVisible(bool visible)
{
	if (visible == isVisible())
		return;

	if (!visible && isVisible()) {
		model_->invisibleRootItem()->removeRow(item_->row());
		item_ = 0;
	}

	if (visible && !isVisible()) {
		item_ = new QStandardItem(message_);
		item_->setData(QVariant(type_), TypeRole);
		item_->setData(QVariant(persistent_), PersistentNoticeRole);
		item_->setData(QVariant(false), MessagePlainTextRole);
		item_->setData(QVariant(true), IncomingRole);
		item_->setData(QVariant(true), MergeRole);
		item_->setData(QVariant(true), EmoteRole);
		item_->setData(QVariant(QDateTime::currentDateTime()), DateTimeRole);
		item_->setData(QVariant(Spooled_None), SpooledRole);
		model_->invisibleRootItem()->appendRow(item_);
	}
}

bool YaChatViewModel::yaChatViewModelNoticeLessThan(const YaChatViewModel::Notice* n1, const YaChatViewModel::Notice* n2)
{
	return n1->priority() < n2->priority();
}

//----------------------------------------------------------------------------
// YaChatViewModel
//----------------------------------------------------------------------------

YaChatViewModel::YaChatViewModel()
	: updateNoticesTimer_(0)
	, composingEvent_(0)
	, userIsOfflineNotice_(0)
	, userIsBlockedNotice_(0)
	, userNotInListNotice_(0)
	, notAuthorizedToSeeUserStatusNotice_(0)
	, accountIsOfflineNotice_(0)
	, accountIsDisabledNotice_(0)
	, statusTypeChangedNotice_(0)
{
	addDummyHeader();

	composingEvent_ = new Notice(this, ContactComposing,
	                             tr(" is composing a message..."));
	userIsOfflineNotice_ = new Notice(this, UserIsOfflineNotice,
	                                  tr(" is currently offline. You could send a message and it"
	                                     " will be delivered when the contact comes back online."));
	userIsOfflineNotice_->setPriority(5);
	userIsBlockedNotice_ = new Notice(this, UserIsBlockedNotice,
	                                  tr(" is blocked. That is you are unable to send the messages"
	                                     " and to view the online status of each other. To unblock"
	                                     " the contact simply add it to your roster."));
	userIsBlockedNotice_->setPriority(2);
	userNotInListNotice_ = new Notice(this, UserNotInListNotice,
	                                  tr(" is not in contact list. You need to add him to your "
	                                     "contact list in order to see the online status of each "
	                                     "other."));
	userNotInListNotice_->setPriority(3);
	notAuthorizedToSeeUserStatusNotice_ = new Notice(this, NotAuthorizedToSeeUserStatusNotice,
	                                                 tr("You're not authorized to see the online "
	                                                    "status of this contact."));
	notAuthorizedToSeeUserStatusNotice_->setPriority(4);
	accountIsOfflineNotice_ = new Notice(this, AccountIsOfflineNotice,
	                                     tr("You're currently offline. You can't send or receive"
	                                        " messages unless you became online."));
	accountIsOfflineNotice_->setPriority(1);
	accountIsDisabledNotice_ = new Notice(this, AccountIsDisabledNotice,
	                                      tr("This account is currently disabled. You can't send or"
	                                         " receive messages unless you enable it."));
	accountIsDisabledNotice_->setPriority(0);
	statusTypeChangedNotice_ = new Notice(this, StatusTypeChangedNotice, QString());
	statusTypeChangedNotice_->setPersistent(false);

	notices_ << userIsOfflineNotice_
	         << userIsBlockedNotice_
	         << userNotInListNotice_
	         << notAuthorizedToSeeUserStatusNotice_
	         << accountIsOfflineNotice_
	         << accountIsDisabledNotice_;

	qStableSort(notices_.begin(), notices_.end(), YaChatViewModel::yaChatViewModelNoticeLessThan);

	updateNoticesTimer_ = new QTimer(this);
	updateNoticesTimer_->setInterval(10);
	updateNoticesTimer_->setSingleShot(true);
	connect(updateNoticesTimer_, SIGNAL(timeout()), SLOT(updateNotices()));
}

void YaChatViewModel::addDummyHeader()
{
	QStandardItem* item = new QStandardItem();
	item->setData(QVariant(DummyHeader), TypeRole);
	invisibleRootItem()->insertRow(newMessageRow(), item);
}

void YaChatViewModel::doClear()
{
	// we need to make sure YaChatView::rowsAboutToBeRemoved() are really called
	QStandardItem* parentItem = invisibleRootItem();
	while (parentItem->rowCount() > 0) {
		parentItem->removeRow(0);
	}

	clear();
}

YaChatViewModel::Type YaChatViewModel::childType(const QStandardItem* item) const
{
	return static_cast<YaChatViewModel::Type>(item->data(YaChatViewModel::TypeRole).toInt());
}

QStandardItem* YaChatViewModel::lastMessageOrDateHeader() const
{
	QStandardItem* parentItem = invisibleRootItem();
	if (parentItem->rowCount() > 0) {
		for (int row = parentItem->rowCount() - 1; row >= 0; --row) {
			QStandardItem* child = parentItem->child(row);
			switch (childType(child)) {
			case Message:
			case MoodChanged:
			case StatusTypeChangedNotice:
			case DateHeader:
				return child;
			default:
				;
			}
		}
	}
	return 0;
}

QStandardItem* YaChatViewModel::lastMessage() const
{
	QStandardItem* result = lastMessageOrDateHeader();
	if (!result || childType(result) == DateHeader) {
		result = 0;
	}
	return result;
}

void YaChatViewModel::ensureDateHeader(const QDateTime& time)
{
	if (shouldAddDateHeader(time)) {
		addDateHeader(time);
	}
}

bool YaChatViewModel::shouldAddDateHeader(const QDateTime& time) const
{
	QStandardItem* item = lastMessageOrDateHeader();
	if (item) {
		if (childType(item) == DateHeader) {
			invisibleRootItem()->removeRow(item->row());
			return true;
		}

		QDateTime lastDate = item->data(DateTimeRole).toDateTime();
		return lastDate.daysTo(time) != 0;
	}

	return true;
}

bool YaChatViewModel::shouldMergeWith(const QDateTime& time, bool emote, bool local, YaChatViewModel::SpooledType spooled) const
{
	if (emote)
		return false;

	QStandardItem* message = lastMessage();
	if (message) {
		if (message->data(TypeRole) != Message &&
		    message->data(TypeRole) != MoodChanged)
		{
			return false;
		}

		if (message->data(IncomingRole) == !local &&
		    spooledType(message->index()) == spooled)
		{
			QDateTime lastTime = message->data(DateTimeRole).toDateTime();
			if (lastTime.secsTo(time) < mergeInterval * 60)
				return true;
		}
	}

	return false;
}

#ifndef YAPSI
void YaChatViewModel::addMessage(bool incoming, const QString& msg)
{
	addMessage(false, QDateTime::currentDateTime(), !incoming, msg);
}

void YaChatViewModel::addEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt)
{
	addMessageHelper(spooled, time, local, txt, true);
}

void YaChatViewModel::addMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt)
{
	addMessageHelper(spooled, time, local, txt, false);
}
#else
void YaChatViewModel::addMessage(bool incoming, const QString& msg)
{
	addMessage(YaChatViewModel::Spooled_None, QDateTime::currentDateTime(), !incoming, 0, msg);
}

void YaChatViewModel::addEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString txt)
{
	addMessageHelper(spooled, time, local, txt, true, spamFlag);
}

void YaChatViewModel::addMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString txt)
{
	addMessageHelper(spooled, time, local, txt, false, spamFlag);
}
#endif

#ifndef YAPSI
void YaChatViewModel::addMessageHelper(SpooledType spooled, const QDateTime& time, bool local, QString _txt, bool emote)
#else
void YaChatViewModel::addMessageHelper(SpooledType spooled, const QDateTime& time, bool local, QString _txt, bool emote, int spamFlag)
#endif
{
	if (_txt.isEmpty()) {
		qWarning("YaChatViewModel::addMessageHelper: dropping empty message");
		return;
	}

	ensureDateHeader(time);

	QString txt = TextUtil::linkifyClever(_txt);
	QStandardItem* item = new QStandardItem(txt);
	item->setData(QVariant(Message), TypeRole);
	item->setData(QVariant(!Qt::mightBeRichText(txt)), MessagePlainTextRole);
	item->setData(QVariant(!local), IncomingRole);
	item->setData(QVariant(shouldMergeWith(time, emote, local, spooled)), MergeRole);
	item->setData(QVariant(time), DateTimeRole);
	item->setData(QVariant(spooled), SpooledRole);
	item->setData(QVariant(emote), EmoteRole);
#ifdef YAPSI
	item->setData(QVariant(spamFlag), SpamRole);
#endif

	invisibleRootItem()->insertRow(newMessageRow(), item);
}

void YaChatViewModel::addDateHeader(const QDateTime& time)
{
	QStandardItem* item = new QStandardItem();
	item->setData(QVariant(DateHeader), TypeRole);
	item->setData(QVariant(time), DateTimeRole);

	invisibleRootItem()->insertRow(newMessageRow(), item);
}

int YaChatViewModel::newMessageRow() const
{
	int rowNum = 0;
	for (int row = invisibleRootItem()->rowCount() - 1; row >= 0; --row) {
		if (invisibleRootItem()->child(row)->data(YaChatViewModel::PersistentNoticeRole).toBool()) {
			continue;
		}
		else {
			rowNum = row + 1;
			break;
		}
	}

	// QStandardItem* message = lastMessage();
	// if (message)
	// 	rowNum = message->row() + 1;

	return rowNum;
}

bool YaChatViewModel::composingEventVisible() const
{
	return composingEvent_->isVisible();
}

void YaChatViewModel::setComposingEventVisible(bool visible)
{
	composingEvent_->setVisible(visible);
}

bool YaChatViewModel::userIsOfflineNoticeVisible() const
{
	return userIsOfflineNotice_->shouldBeVisible();
}

void YaChatViewModel::setUserIsOfflineNoticeVisible(bool visible)
{
	userIsOfflineNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::userIsBlockedNoticeVisible() const
{
	return userIsBlockedNotice_->shouldBeVisible();
}

void YaChatViewModel::setUserIsBlockedNoticeVisible(bool visible)
{
	userIsBlockedNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::userNotInListNoticeVisible() const
{
	return userNotInListNotice_->shouldBeVisible();
}

void YaChatViewModel::setUserNotInListNoticeVisible(bool visible)
{
	userNotInListNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::notAuthorizedToSeeUserStatusNoticeVisible() const
{
	return notAuthorizedToSeeUserStatusNotice_->shouldBeVisible();
}

void YaChatViewModel::setNotAuthorizedToSeeUserStatusNoticeVisible(bool visible)
{
	notAuthorizedToSeeUserStatusNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::accountIsOfflineNoticeVisible() const
{
	return accountIsOfflineNotice_->shouldBeVisible();
}

void YaChatViewModel::setAccountIsOfflineNoticeVisible(bool visible)
{
	accountIsOfflineNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

bool YaChatViewModel::accountIsDisabledNoticeVisible() const
{
	return accountIsDisabledNotice_->shouldBeVisible();
}

void YaChatViewModel::setAccountIsDisabledNoticeVisible(bool visible)
{
	accountIsDisabledNotice_->setShouldBeVisible(visible);
	updateNoticesTimer_->start();
}

void YaChatViewModel::setStatusTypeChangedNotice(XMPP::Status::Type status)
{
	if (status == XMPP::Status::Offline) {
		// persistent notice will be shown for this status type anyway
		// so there's no need for duplicate information
		statusTypeChangedNotice_->setVisible(false);
		return;
	}

	ensureDateHeader(QDateTime::currentDateTime());

	statusTypeChangedNotice_->setMessage(
	    tr(" is currently %1").arg(Ya::statusFullName(status))
	);
	statusTypeChangedNotice_->setVisible(true);
}

void YaChatViewModel::addMoodChange(SpooledType spooled, const QString& _mood, const QDateTime& timeStamp)
{
	// FIXME: create sexTr() function?
	// сменил(а|о) настроение на:
	QString mood = TextUtil::plain2richSimple(_mood.trimmed());
	QString message;
	if (mood.isEmpty())
		message = tr("is not in a mood");
	else
		message = tr("has changed mood to: %1").arg(mood);
	QStandardItem* item = new QStandardItem(message);
	item->setData(QVariant(MoodChanged), TypeRole);
	item->setData(QVariant(false), PersistentNoticeRole);
	item->setData(QVariant(true), MessagePlainTextRole);
	item->setData(QVariant(true), IncomingRole);
	item->setData(QVariant(false), MergeRole);
	item->setData(QVariant(true), EmoteRole);
	item->setData(QVariant(timeStamp), DateTimeRole);
	item->setData(QVariant(spooled), SpooledRole);

	ensureDateHeader(item->data(DateTimeRole).toDateTime());

	invisibleRootItem()->appendRow(item);
}

YaChatViewModel::Type YaChatViewModel::type(const QModelIndex& index)
{
	return static_cast<YaChatViewModel::Type>(index.data(YaChatViewModel::TypeRole).toInt());
}

YaChatViewModel::SpooledType YaChatViewModel::spooledType(const QModelIndex& index)
{
	return static_cast<YaChatViewModel::SpooledType>(index.data(YaChatViewModel::SpooledRole).toInt());
}

void YaChatViewModel::updateNotices()
{
	bool foundVisible = false;
	foreach(Notice* notice, notices_) {
		notice->setVisible(!foundVisible && notice->shouldBeVisible());
		if (notice->isVisible()) {
			foundVisible = true;
		}
	}
}
