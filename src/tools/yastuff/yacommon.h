/*
 * yacommon.h - common YaPsi routines
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

#ifndef YACOMMON_H
#define YACOMMON_H

class QPixmap;
class QColor;

#include <QDateTime>
#include <QRect>
#include <QHash>

#include "xmpp_jid.h"
#include "xmpp_status.h"
#include "xmpp_vcard.h"
#include "xmpp_message.h"

class UserListItem;
class QChar;
class QFontMetrics;
class PsiAccount;
class PsiEvent;
class PsiIcon;

namespace Ya {
	extern const QString INFORMERS_GROUP_NAME;
	extern const QStringList BOTS_GROUP_NAMES;

	extern const QString TIMESTAMP;
	extern const QString MAROON;
	extern const QString NAVY;
	extern const QString DARKGREEN;
	extern const QString BLACK;

	extern const QString MESAYS;
	extern const QString HESAYS;
	extern const QString SPOOLED;

	extern const QString WHO_FONT_SIZE;
	extern const QString WHAT_FONT_SIZE;
	extern const QString WHEN_FONT_SIZE;

	extern const int CONVERSATION_TIMEOUT; // in secs

	enum DecorationState {
		Normal = 0,
		Hover,
		Pressed
	};

	const QString& ellipsis();
	QPixmap groupPixmap(QSize size, bool open, DecorationState state);
	QPixmap genderPixmap(XMPP::VCard::Gender gender);
	bool isYaInformer(PsiEvent* event);
	bool isYaJid(const XMPP::Jid&);
	QString defaultFriendsGroup();
	bool isFriendsGroup(const QString& groupName);
	bool isInFriends(const UserListItem*);
	QString statusFullName(XMPP::Status::Type status);
	QString statusName(XMPP::Status::Type status);
	QString statusDescription(XMPP::Status::Type status);
	QColor statusColor(XMPP::Status::Type status);
	QString visibleText(const QString& fullText, const QFontMetrics& fontMetrics, int width, int height, int numLines);
	QString normalizeStanza(QString txt);
	QString buildBuzzMessage(QString txt);
	QString buzzMessage(QString txt = QString::null);
	bool isBuzzMessage(const QString txt);
	QRect circle_bounds(const QPointF &center, double radius, double compensation);
	const QString truncate(const QString &s, int len);
	bool isSubscriptionRequest(PsiEvent* event);
	QString createUniqueName(QString baseName, QStringList existingNames);
	QString contactName(const QString& name, const QString& jid);
	QString emoticonToolTipSimple(const PsiIcon* icon, const QString& text = QString());
	QString emoticonToolTip(const PsiIcon* icon, const QString& text = QString());
	QString processMood(const QString& oldMood, const QString& newMood, XMPP::Status::Type statusType);
	QString limitText(const QString& text, int limit);
	QString messageNotifierText(const QString& messageText);
	QString yaRuAliasing(const QString& jid);
	QString stripYaRuHostname(const QString& jid);

	void initializeDefaultGeometry(QWidget* w, const QString& geometryOptionPath, const QRect& defaultGeometry, bool centerOnScreen);

	// history dealing
	struct SpooledMessage {
		SpooledMessage(bool _originLocal, const QDateTime& _timeStamp, const XMPP::Message& _message)
			: originLocal(_originLocal)
			, timeStamp(_timeStamp)
			, message(_message)
			, isMood(false)
		{}

		SpooledMessage(const QDateTime& _timeStamp, const QString& _mood)
			: originLocal(false)
			, timeStamp(_timeStamp)
			, isMood(true)
			, mood(_mood)
		{}

		bool originLocal;
		QDateTime timeStamp;
		XMPP::Message message;

		bool isMood;
		QString mood;
	};

	QList<SpooledMessage> lastMessages(const PsiAccount* me, const XMPP::Jid& interlocutor, unsigned int count = 0);
	const QString history(const PsiAccount* me, const XMPP::Jid& interlocutor);
	const void showHistory(const PsiAccount* me, const XMPP::Jid& interlocutor);

	// colorize stanzas
	const QString colorString(bool local, bool spooled);
	const QString formatStanza(bool needOpeningTitle, QString color, QString timestr, QString who, QString txt);

	class DateFormatter : public QObject
	{
		Q_OBJECT
	public:
		static DateFormatter* instance();

		QString daySuffix(int day);
		QString dayWithSuffix(int day);
		QString dateAndWeekday(QDate date);
		QString longDateTime(QDateTime dateTime);

	private:
		DateFormatter();

		static DateFormatter* instance_;
		QStringList months;
		QHash<Qt::DayOfWeek, QString> daysOfWeek;
	};

	class AgeFormatter : public QObject
	{
		Q_OBJECT
	public:
		static QString ageSuffix(int age);
		static QString ageInYears(int age);
	};
};

#endif
