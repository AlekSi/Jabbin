/*
 * yachatviewmodel.h - stores items of custom chatlog
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

#ifndef YACHATVIEWMODEL_H
#define YACHATVIEWMODEL_H

#include <QStandardItemModel>

#include "xmpp_status.h"

class YaChatViewModel : public QStandardItemModel
{
	Q_OBJECT
public:
	YaChatViewModel();

	enum {
		TypeRole         = Qt::UserRole + 0,
		DelegateDataRole = Qt::UserRole + 1,
		PersistentNoticeRole = Qt::UserRole + 2, // bool

		// Type = Message
		// DisplayRole stores the actual message text (plain-text or XHTML-IM)
		IncomingRole         = Qt::UserRole + 3, // bool
		MergeRole            = Qt::UserRole + 4, // bool, if following the message with the same incoming value (automatically calculated when appending message)
		DateTimeRole         = Qt::UserRole + 5, // QDateTime
		MessagePlainTextRole = Qt::UserRole + 6, // bool, true if MessageRole is plain-text (probably should ditch it)
		SpooledRole          = Qt::UserRole + 7, // SpooledType, true for offline messages and queued history loads
		EmoteRole            = Qt::UserRole + 8, // bool, true for /me messages
#ifdef YAPSI
		SpamRole             = Qt::UserRole + 9  // int, 0 for not spam, >0 for marked as spam
#endif

		// Type = DateHeader
		// DateTimeRole is reused here

		// Type = FileTransfer
		// Type = SomethingElse
	};

	enum Type {
		DummyHeader,
		Message,
		DateHeader,
		ContactComposing,
		UserIsOfflineNotice,
		UserIsBlockedNotice,
		UserNotInListNotice,
		NotAuthorizedToSeeUserStatusNotice,
		AccountIsOfflineNotice,
		AccountIsDisabledNotice,
		StatusTypeChangedNotice,
		MoodChanged
	};

	enum SpooledType {
		Spooled_None,
		Spooled_OfflineStorage,
		Spooled_History
	};

	void addDummyHeader();

	void doClear();

#ifndef YAPSI
	void addEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt);
	void addMessage(SpooledType spooled, const QDateTime& time, bool local, QString txt);
#else
	void addEmoteMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString txt);
	void addMessage(SpooledType spooled, const QDateTime& time, bool local, int spamFlag, QString txt);
#endif
	void addMessage(bool incoming, const QString& msg);

	bool composingEventVisible() const;
	void setComposingEventVisible(bool visible);

	bool userIsOfflineNoticeVisible() const;
	void setUserIsOfflineNoticeVisible(bool visible);

	bool userIsBlockedNoticeVisible() const;
	void setUserIsBlockedNoticeVisible(bool visible);

	bool userNotInListNoticeVisible() const;
	void setUserNotInListNoticeVisible(bool visible);

	bool notAuthorizedToSeeUserStatusNoticeVisible() const;
	void setNotAuthorizedToSeeUserStatusNoticeVisible(bool visible);

	bool accountIsOfflineNoticeVisible() const;
	void setAccountIsOfflineNoticeVisible(bool visible);

	bool accountIsDisabledNoticeVisible() const;
	void setAccountIsDisabledNoticeVisible(bool visible);

	void setStatusTypeChangedNotice(XMPP::Status::Type status);
	void addMoodChange(SpooledType spooled, const QString& mood, const QDateTime& timeStamp);

	static YaChatViewModel::Type type(const QModelIndex& index);
	static YaChatViewModel::SpooledType spooledType(const QModelIndex& index);

private slots:
	void updateNotices();

private:
	Type childType(const QStandardItem* item) const;
	QStandardItem* lastMessageOrDateHeader() const;
	QStandardItem* lastMessage() const;
	void ensureDateHeader(const QDateTime& time);
	bool shouldAddDateHeader(const QDateTime& time) const;
	bool shouldMergeWith(const QDateTime& time, bool emote, bool local, YaChatViewModel::SpooledType spooled) const;
#ifndef YAPSI
	void addMessageHelper(YaChatViewModel::SpooledType spooled, const QDateTime& time, bool local, QString txt, bool emote);
#else
	void addMessageHelper(YaChatViewModel::SpooledType spooled, const QDateTime& time, bool local, QString txt, bool emote, int spamFlag);
#endif
	void addDateHeader(const QDateTime& time);
	int newMessageRow() const;

	void appendNotice(QStandardItem* item);

	class Notice : public QObject
	{
	public:
		Notice(QStandardItemModel* parent, YaChatViewModel::Type type, QString message);

		int priority() const;
		void setPriority(int priority);

		bool isVisible() const;
		void setVisible(bool visible);

		bool shouldBeVisible() const;
		void setShouldBeVisible(bool visible);

		QString message() const;
		void setMessage(const QString& message);

		bool persistent() const;
		void setPersistent(bool persistent);

	private:
		YaChatViewModel::Type type_;
		bool shouldBeVisible_;
		int priority_;
		QString message_;
		bool persistent_;
		QStandardItemModel* model_;
		QStandardItem* item_;
	};

	static bool yaChatViewModelNoticeLessThan(const YaChatViewModel::Notice* n1, const YaChatViewModel::Notice* n2);

	QTimer* updateNoticesTimer_;
	Notice* composingEvent_;
	Notice* userIsOfflineNotice_;
	Notice* userIsBlockedNotice_;
	Notice* userNotInListNotice_;
	Notice* notAuthorizedToSeeUserStatusNotice_;
	Notice* accountIsOfflineNotice_;
	Notice* accountIsDisabledNotice_;
	Notice* statusTypeChangedNotice_;
	QList<Notice*> notices_;
};

#endif
