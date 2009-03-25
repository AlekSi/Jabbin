/*
 * yacommon.cpp - common YaPsi routines
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

#include "yacommon.h"

#include <QPixmap>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QTemporaryFile>
#include <QApplication>
#include <QDesktopWidget>

#include "xmpp_jid.h"
#include "avatars.h"
#include "eventdb.h"
#include "desktoputil.h"
#include "jidutil.h"
#include "textutil.h"
#include "userlist.h"
#include "psiaccount.h"
#include "psioptions.h"
#include "psievent.h"
#include "visibletext.h"
#include "yacontactlistviewdelegate.h"
#include "yavisualutil.h"
#include "jabbinpsi_revision.h"
#include "common.h"
#include "iconset.h"

const static QString FRIENDS_GROUP = QString::fromUtf8("Jabberout friends");
const static QChar FRIENDS_MARKER = QChar(0x263A); // ☺
const static QChar BUZZ_MARKER = QChar(0x2009); // n space
const static QString BUZZ_MESSAGE = QString::fromUtf8("/me ") + "%1" + BUZZ_MARKER;
const static QString S_ELLIPSIS = QString::fromUtf8("…");
const static QString S_HISTORY_IS_EMPTY = QString::fromUtf8("<h2>Chat history is empty</h2>");
const static QString CHAT_BUZZ_MESSAGE = QString::fromUtf8("options.ya.chat.message"); // No I18N !!
const static QString STR_YAPSI_HISTORY = QString::fromUtf8("chats_with_%1_at_%2.html"); // No I18N !!

const QString Ya::INFORMERS_GROUP_NAME = QString::fromUtf8("Informers");
const QStringList Ya::BOTS_GROUP_NAMES = QStringList() << Ya::INFORMERS_GROUP_NAME;

const QString Ya::TIMESTAMP = "#c7c7c7";
const QString Ya::MESAYS = "#DDA46D";
const QString Ya::HESAYS = "#13A9E8";
const QString Ya::SPOOLED = "#660000";

const int Ya::CONVERSATION_TIMEOUT = 300; // in secs

#include <QString>
// TODO can't understand why lrelease doesn't work here, left ugly
//const static QString DEFAULT_BUZZ_MESSAGE = QCoreApplication::instance()->translate("Ya", "requests your attention!");
const static QString DEFAULT_BUZZ_MESSAGE = QString::fromUtf8("Requests your attention!");

const QString& Ya::ellipsis()
{
	return S_ELLIPSIS;
}

QPixmap Ya::groupPixmap(QSize size, bool open, DecorationState state)
{
	QString group = open ? "open" : "closed";
	QString s;
	if (state == Hover)
		s = "hover";
	else
		s = "normal";
	return QPixmap(QString(":/images/group/%1_%2.png").arg(group, s));
}

QPixmap Ya::genderPixmap(XMPP::VCard::Gender gender)
{
	QString fn = "gn";
	if (gender == XMPP::VCard::Male)
		fn = "gm";
	else if (gender == XMPP::VCard::Female)
		fn = "gw";
	return QPixmap(QString(":/images/gender/%1.gif").arg(fn));
}

bool Ya::isYaInformer(PsiEvent* event)
{
	if (event->type() != PsiEvent::Message)
		return false;
	QStringList informers;
	informers
	  << "informer.ya.ru"
	  << "mail.ya.ru"
	  << "probki.ya.ru"
	  << "weather.ya.ru";
	return informers.contains(event->from().host());
}

bool Ya::isYaJid(const XMPP::Jid& jid)
{
	return jid.host() == "jabberout.com";
}

static const QChar& friendsChar()
{
	static QChar friends;
	if (friends.isNull())
		friends = FRIENDS_MARKER;
	return friends;
}

QString Ya::defaultFriendsGroup()
{
	// QString result = friendsChar();
	// result += QCoreApplication::instance()->translate("Ya", "Friends");
	// return result;
	return FRIENDS_GROUP;
}

bool Ya::isFriendsGroup(const QString& groupName)
{
#ifdef YAPSI_YARU_FRIENDS
	// return (groupName.length() > 0) && (groupName.at(0) == friendsChar());
	return groupName == FRIENDS_GROUP;
#else
	return false;
#endif
}

bool Ya::isInFriends(const UserListItem* u)
{
	if (!u)
		return false;

	foreach(QString group, u->groups()) {
		if (Ya::isFriendsGroup(group))
			return true;
	}

	return false;
}

QString Ya::statusFullName(XMPP::Status::Type status)
{
	QString result;
	switch (status) {
	case XMPP::Status::Offline:
		result = QCoreApplication::instance()->translate("Ya", "Offline");
		break;
	case XMPP::Status::XA:
		if (PsiOptions::instance()->getOption("options.ui.menu.status.xa").toBool()) {
			result = QCoreApplication::instance()->translate("Ya", "XA");
			break;
		}
	case XMPP::Status::Away:
		result = QCoreApplication::instance()->translate("Ya", "Away");
		break;
	case XMPP::Status::DND:
		result = QCoreApplication::instance()->translate("Ya", "DnD");
		break;
	case XMPP::Status::FFC:
		if (PsiOptions::instance()->getOption("options.ui.menu.status.chat").toBool()) {
			result = QCoreApplication::instance()->translate("Ya", "Chat");
			break;
		}
	case XMPP::Status::Online:
		result = QCoreApplication::instance()->translate("Ya", "Online");
		break;
	case XMPP::Status::Invisible:
		result = QCoreApplication::instance()->translate("Ya", "Invisible");
		break;
	default:
		Q_ASSERT(false);
	}

	return result;
}

QString Ya::statusFullText(XMPP::Status::Type status)
{
	QString result;
	switch (status) {
	case XMPP::Status::Offline:
		result = QCoreApplication::instance()->translate("Ya", "Offline");
		break;
	case XMPP::Status::XA:
		if (PsiOptions::instance()->getOption("options.ui.menu.status.xa").toBool()) {
			result = QCoreApplication::instance()->translate("Ya", "Long gone");
			break;
		}
	case XMPP::Status::Away:
		result = QCoreApplication::instance()->translate("Ya", "Away");
		break;
	case XMPP::Status::DND:
		result = QCoreApplication::instance()->translate("Ya", "Don't disturb");
		break;
	case XMPP::Status::FFC:
		if (PsiOptions::instance()->getOption("options.ui.menu.status.chat").toBool()) {
			result = QCoreApplication::instance()->translate("Ya", "Free for chat");
			break;
		}
	case XMPP::Status::Online:
		result = QCoreApplication::instance()->translate("Ya", "Available");
		break;
	case XMPP::Status::Invisible:
		result = QCoreApplication::instance()->translate("Ya", "Invisible");
		break;
	case XMPP::Status::NotAuthorizedToSeeStatus:
		result = QCoreApplication::instance()->translate("Ya", "Not authorized");
		break;
	default:
		Q_ASSERT(false);
	}

	return result;
}
QString Ya::statusName(XMPP::Status::Type status)
{
	QString result = Ya::statusFullName(status).toLower();
	switch (status) {
	case XMPP::Status::FFC:
	case XMPP::Status::Online:
	case XMPP::Status::Offline:
	case XMPP::Status::Invisible:
		result = QString();
		break;
	default:
		break;
	}

	return result;
}

QString Ya::statusDescription(XMPP::Status::Type type)
{
	switch (type) {
	case XMPP::Status::Offline:
		return QCoreApplication::instance()->translate("Ya", "offline");
	case XMPP::Status::Blocked:
		return QCoreApplication::instance()->translate("Ya", "blocked");
	case XMPP::Status::Reconnecting:
		return QCoreApplication::instance()->translate("Ya", "reconnecting");
	case XMPP::Status::NotAuthorizedToSeeStatus:
		return QCoreApplication::instance()->translate("Ya", "not authorized");
	case XMPP::Status::XA:
	case XMPP::Status::Away:
		return QCoreApplication::instance()->translate("Ya", "away");
	case XMPP::Status::DND:
		return QCoreApplication::instance()->translate("Ya", "do not disturb");
	case XMPP::Status::FFC:
	case XMPP::Status::Online:
	case XMPP::Status::Invisible:
		return QCoreApplication::instance()->translate("Ya", "online");
	}
	Q_ASSERT(false);
	return QString();
}

QColor Ya::statusColor(XMPP::Status::Type status)
{
	return Ya::VisualUtil::statusColor(status, false);
}

QString Ya::normalizeStanza(QString txt)
{
	/*
	// bloody kopete inserts superfluous paras
	QRegExp rx("\\s*<span\\s+xmlns=\".*\">\\s*<p\\s+style=\".*\"\\s*>(.*)</p>\\s*</span>\\s*");
	if (rx.indexIn(txt) > -1) {
		txt = rx.cap(1);
	}
	// bloody pandeon inserts superfluous spans
	QRegExp rx("\\s*<span\\s+xmlns=\".*\">\\s*<span\\s+style=\".*\"\\s*>(.*)</span>\\s*</span>\\s*");
	if (rx.indexIn(txt) > -1) {
		txt = rx.cap(1);
	}
	*/
	return txt;
}

QString Ya::buildBuzzMessage(QString txt)
{
	if (txt.isEmpty()) {
		txt = DEFAULT_BUZZ_MESSAGE;
		PsiOptions::instance()->setOption(CHAT_BUZZ_MESSAGE, txt);
	}
	return txt;
}

QString Ya::buzzMessage(QString txt)
{
	if (txt.isEmpty()) {
		txt = buildBuzzMessage(PsiOptions::instance()->getOption(CHAT_BUZZ_MESSAGE).toString());
	}
	return BUZZ_MESSAGE.arg(txt);
}

bool Ya::isBuzzMessage(const QString txt)
{
	return normalizeStanza(txt).contains(BUZZ_MARKER);
}

QString Ya::visibleText(const QString& fullText, const QFontMetrics& fontMetrics, int width, int height, int numLines)
{
	return ::visibleText(fullText, fontMetrics, width, height, numLines);
}

QRect Ya::circle_bounds(const QPointF &center, double radius, double compensation)
{
	return QRect(
	           qRound(center.x() - radius - compensation),
	           qRound(center.y() - radius - compensation),
	           qRound((radius + compensation) * 2),
	           qRound((radius + compensation) * 2)
	       );
}

const QString Ya::truncate(const QString &s, int len)
{
	return len <= 0 || len >= s.length() ? s : s.left(len - ellipsis().length()) + ellipsis();
}

bool Ya::isSubscriptionRequest(PsiEvent* event)
{
	return dynamic_cast<AuthEvent*>(event) && dynamic_cast<AuthEvent*>(event)->authType() == "subscribe";
}

QList<Ya::SpooledMessage> Ya::lastMessages(const PsiAccount* me, const XMPP::Jid& interlocutor, unsigned int count)
{
	QList<Ya::SpooledMessage> result;

	EDBHandle exp(me->edb());
	exp.getLatest(interlocutor, count);

	while (exp.busy()) {
		QCoreApplication::instance()->processEvents();
	}

	const EDBResult* r = exp.result();
	if (r && r->count() > 0) {
		Q3PtrListIterator<EDBItem> it(*r);
		it.toLast();
		for (; it.current(); --it) {
			PsiEvent *e = it.current()->event();

			if (e->type() == PsiEvent::Message) {
				MessageEvent* me = static_cast<MessageEvent*>(e);
				result << Ya::SpooledMessage(me->originLocal(), me->timeStamp(), me->message());
			}
			else if (e->type() == PsiEvent::Mood) {
				MoodEvent* me = static_cast<MoodEvent*>(e);
				result << Ya::SpooledMessage(me->timeStamp(), me->mood());
			}
		}
	}

	return result;
}

static QList<Ya::SpooledMessage> messagesFor(const PsiAccount* me, XMPP::Jid interlocutor)
{
	QList<Ya::SpooledMessage> result;

	EDBHandle exp(me->edb());
	QString id;

	while (true) {
		exp.get(interlocutor, id, EDB::Forward, 1000);

		while (exp.busy()) {
			QCoreApplication::instance()->processEvents();
		}

		const EDBResult* r = exp.result();
		if (r && r->count() > 0) {
			Q3PtrListIterator<EDBItem> it(*r);
			for (; it.current(); ++it) {
				id = it.current()->nextId();
				PsiEvent *e = it.current()->event();

				if (e->type() == PsiEvent::Message) {
					MessageEvent* me = static_cast<MessageEvent*>(e);
					result << Ya::SpooledMessage(me->originLocal(), me->timeStamp(), me->message());
				}
				else if (e->type() == PsiEvent::Mood) {
					MoodEvent* me = static_cast<MoodEvent*>(e);
					result << Ya::SpooledMessage(me->timeStamp(), me->mood());
				}
				else if (e->type() == PsiEvent::Auth) {
					AuthEvent* ae = static_cast<AuthEvent*>(e);
					XMPP:Message message;
					message.setBody(ae->description());
					result << Ya::SpooledMessage(ae->originLocal(), ae->timeStamp(), message);
				}
			}
		}
		else {
			break;
		}

		if (id.isEmpty()) {
			break;
		}
	}

	return result;
}

const QString Ya::history(const PsiAccount* me, const XMPP::Jid& interlocutor)
{
	QString result;
	QTextStream stream(&result, QIODevice::WriteOnly);

	QString us = me->nick();
	UserListItem *u = me->findFirstRelevant(interlocutor);
	QString them = JIDUtil::nickOrJid(u->name(), u->jid().full());

	QDateTime lastDateTime;
	QString lastSpeaker = QString();
	foreach(SpooledMessage message, messagesFor(me, interlocutor)) {
		QString speaker = message.originLocal ? us : them;
		bool merge = (lastSpeaker == speaker) &&
		             (lastDateTime.secsTo(message.timeStamp) < 5 * 60);

		if (lastDateTime.isNull() || lastDateTime.date().daysTo(message.timeStamp.date()) != 0) {
			merge = false;
			stream
			<< "<div class=\"date-\">"
			<< Ya::DateFormatter::instance()->dateAndWeekday(message.timeStamp.date())
			<< "</div>";
		}

		lastSpeaker = speaker;
		lastDateTime = message.timeStamp;

		QString msg;
		if (message.isMood) {
			msg = message.mood;
		}
		else {
			msg = message.message.body();
		}

		msg = TextUtil::plain2rich(msg);
		msg = TextUtil::linkify(msg);
		msg = TextUtil::emoticonifyForHtml(msg);

		if (message.isMood) {
			stream << (Ya::formatStanza(
			               true,
			               colorString(false, speaker == us),
			               message.timeStamp.toString("hh:mm"),
			               speaker,
			               QObject::tr("has changed mood to: %1").arg(msg)
			           ));
		}
		else {
			stream << (Ya::formatStanza(
			               !merge,
			               colorString(false, speaker == us),
			               message.timeStamp.toString("hh:mm"),
			               speaker,
			               msg
			           ));
		}
	}

	return result;
}

const void Ya::showHistory(const PsiAccount* me, const XMPP::Jid& interlocutor)
{
	Q_ASSERT(me);
	QFile file(QDir::tempPath() + "/" + STR_YAPSI_HISTORY.arg(interlocutor.user().toLower()).arg(interlocutor.host().toLower()));
	if (file.open(QFile::WriteOnly)) {
		QTextStream out(&file);
		out.setCodec("UTF-8");

		out << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" << endl;
		out << "<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"ru\">" << endl;
		out << "<head>" << endl;
		out << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />" << endl;
		out << "<title>" << QCoreApplication::instance()->translate("Ya", "%1: Chat History").arg(TextUtil::escape(interlocutor.bare())) << "</title>" << endl;
		out << "<meta name=\"Generator\" content=\"YaChat :: " << JABBINPSI_VERSION << "." << JABBINPSI_REVISION << "\" />" << endl;
		out << "<meta name=\"Keywords\" content=\"" << TextUtil::escape(me->name()) << "," << TextUtil::escape(interlocutor.full()) << "\" />" << endl;
		out << "<meta name=\"Description\" content=\"" << TextUtil::escape(me->name()) << "," << TextUtil::escape(interlocutor.full()) << "\" />" << endl;

		out << "<style>" << endl;
		out << "body { background: #777777 }" << endl;
		out << ".chat-qool { background: #ffffff; padding: 2px; }" << endl;
		out << ".chat-qool .line- { margin:  0; padding: 0 5px; }" << endl;
		out << ".chat-qool .hr- { border-top: 1px solid #e5e5e5; padding-top:5px; }" << endl;
		out << ".chat-qool .hr2-  { border-top: 1px solid #e5e5e5; }" << endl;
		out << ".chat-qool .time- { float:right; font: 70% Verdana, Arial; color: #999999; margin-top:2px; }" << endl;
		out << ".chat-qool .author- { font: bold 80% Arial; color: #000000; text-decoration:underline; }" << endl;
		out << ".chat-qool .author- b { color: #ff0000; text-decoration:underline; }" << endl;
		out << ".chat-qool .date- { color: #999999; font: 110% Arial; padding: 5px 5px 5px; }" << endl;

		QFont chatFont;
		chatFont.fromString(option.font[fChat]);
		out << QString(".chat-qool .text-  { font:%1pt %2; line-height:1.3em; color: #000000; margin: .5em 0 .5em 1.5em; }")
		.arg(chatFont.pointSize())
		.arg(chatFont.family()) << endl;
		out << "</style>" << endl;

		out << "</head>" << endl;
		out << "<body>" << endl;
		out << "<table><tr><td width=\"550px\">" << endl;
		out << "<div class=\"chat chat-qool\">" << endl;
		QString h = Ya::history(me, interlocutor);
		out << (h.isEmpty() ? S_HISTORY_IS_EMPTY : h);
		out << "</div>" << endl;
		out << "</table>" << endl;
		out << "</body>" << endl;
		out << "</html>" << endl;

		QString url = file.fileName();
#ifdef Q_WS_MAC
		url = "file://" + url;
#endif

		DesktopUtil::openUrl(url);
	}
}

QString Ya::limitText(const QString& text, int limit)
{
	if (text.length() > limit) {
		return text.left(limit) + "...";
	}
	return text;
}

QString Ya::messageNotifierText(const QString& messageText)
{
	if (!PsiOptions::instance()->getOption("options.ya.popups.message.show-text").toBool()) {
		return QObject::tr("New message.");
	}

	return messageText;
}

QString Ya::yaRuAliasing(const QString& jid)
{
	QString tmp = jid;
	tmp.replace(QRegExp("@(joim|jabbin|jabberout)\\.(com|net)$"), "@jabberout.com");
	return tmp;
}

QString Ya::stripYaRuHostname(const QString& jid)
{
	QString tmp = jid;
	tmp.replace(QRegExp("@(joim|jabbin|jabberout)\\.(com|net)$"), "");
	return tmp;
}

const QString Ya::colorString(bool local, bool spooled)
{
	return spooled ? SPOOLED : local ? MESAYS : HESAYS;
}

const QString Ya::formatStanza(bool needOpeningTitle, QString color, QString timestr, QString who, QString txt)
{
	QString result;
	QTextStream stream(&result, QIODevice::WriteOnly);
	if (who.isEmpty()) { // system message
		stream
			<< "<hr>"
			<< "<span style='margin: 0; padding: 0; color: "
			<< TIMESTAMP
			<< "; font-size: smaller;'>"
			<< timestr
			<< ": "
			<< txt
			<< "</span><br>" << endl;
	} else {
		if (needOpeningTitle) {
			stream
			<< "<div class=\"line- hr- hr2-\">"
			<< "<div class=\"time-\">"
			<< timestr
			<< "</div>"
			<< "<div class=\"author-\">"
			<< "<b>" << who.left(1) << "</b>" << who.mid(1)
			<< "</div>"
			<< "</div>";
		}
		txt = TextUtil::linkifyClever(txt);
		stream << "<div class=\"text-\">" << txt << "</div>" << endl;
	}
	return result;
}

QString Ya::createUniqueName(QString baseName, QStringList existingNames)
{
	int index = 1;
	bool found = true;
	QString result;
	while (found) {
		result = baseName;
		if (index > 1)
			result += QString(" %1").arg(index);

		found = existingNames.contains(result);

		if (found) {
			++index;
		}
	}

	return result;
}

// note the YAPSI_PALE_ROSTER_JIDS in yacontactlistviewdelegate.cpp
QString Ya::contactName(const QString& name, const QString& jid)
{
	if (name == jid) {
		XMPP::Jid jid(name);
		return jid.node();
	}

	return name;
}

QString Ya::emoticonToolTipSimple(const PsiIcon* icon, const QString& _text)
{
	Q_ASSERT(icon);
	QString text = _text.isEmpty() ? icon->defaultText() : _text;
	QString result;
	// if (icon->description().isEmpty())
		result = text;
	// else
	// 	result = QString("%1 %2").arg(icon->description(), text);
	return result;
}

QString Ya::emoticonToolTip(const PsiIcon* icon, const QString& _text)
{
	return "<div style='white-space:pre'>" + TextUtil::plain2rich(emoticonToolTipSimple(icon, _text)) + "</div>";
}

/**
 * Filters out moods that come from the automatic statuses, and also blocks
 * away statuses since they're automatic 99% of the time
 */
QString Ya::processMood(const QString& oldMood, const QString& newMood, XMPP::Status::Type statusType)
{
	static QStringList autoStatus;
	if (autoStatus.isEmpty()) {
		autoStatus << QString::fromUtf8("Auto Status (idle)");
		autoStatus << QString::fromUtf8("Auto Status (idle)");
		autoStatus << QString::fromUtf8("Yep, I'm here.");
		autoStatus << QString::fromUtf8("Yep, I'm here.");
		autoStatus << QString::fromUtf8("Available");
		autoStatus << QString::fromUtf8("Available");
		autoStatus << QString::fromUtf8("Not available as a result of being idle");
		autoStatus << QString::fromUtf8("Not available as a result of being idle");
		autoStatus << QString::fromUtf8("I'm not here right now");
		autoStatus << QString::fromUtf8("I'm not here right now");
	}

	if (autoStatus.contains(newMood)) {
		// qWarning("processMood: autoStatus: %s", qPrintable(newMood));
		return oldMood;
	}

	if (statusType == XMPP::Status::Away ||
	    statusType == XMPP::Status::XA   ||
	    statusType == XMPP::Status::Offline)
	{
		// qWarning("processMood: away: %s, %d", qPrintable(newMood), statusType);
		return oldMood;
	}

	return newMood;
}

void Ya::initializeDefaultGeometry(QWidget* w, const QString& geometryOptionPath, const QRect& _defaultGeometry, bool centerOnScreen)
{
	QRect savedGeometry = PsiOptions::instance()->getOption(geometryOptionPath).toRect();
	QByteArray byteArray = PsiOptions::instance()->getOption(geometryOptionPath).toByteArray();
	if (byteArray.isEmpty() && !savedGeometry.width() && !savedGeometry.height()) {
		QRect defaultGeometry = _defaultGeometry;
		if (centerOnScreen) {
			QRect r = QApplication::desktop()->availableGeometry(w);
			defaultGeometry.moveTo((r.width() - defaultGeometry.width()) / 2,
			                       (r.height() - defaultGeometry.height()) / 2);
		}

		PsiOptions::instance()->setOption(geometryOptionPath, defaultGeometry);
	}
}

Ya::DateFormatter* Ya::DateFormatter::instance_ = 0;

Ya::DateFormatter* Ya::DateFormatter::instance()
{
	if (!instance_) {
		instance_ = new DateFormatter();
	}
	return instance_;
}

Ya::DateFormatter::DateFormatter()
	: QObject(QCoreApplication::instance())
{
	months
		<< tr("January")
		<< tr("February")
		<< tr("March")
		<< tr("April")
		<< tr("May")
		<< tr("June")
		<< tr("July")
		<< tr("August")
		<< tr("September")
		<< tr("October")
		<< tr("November")
		<< tr("December");

	daysOfWeek[Qt::Monday]    = tr("Monday");
	daysOfWeek[Qt::Tuesday]   = tr("Tuesday");
	daysOfWeek[Qt::Wednesday] = tr("Wednesday");
	daysOfWeek[Qt::Thursday]  = tr("Thursday");
	daysOfWeek[Qt::Friday]    = tr("Friday");
	daysOfWeek[Qt::Saturday]  = tr("Saturday");
	daysOfWeek[Qt::Sunday]    = tr("Sunday");
}

QString Ya::DateFormatter::daySuffix(int day)
{
	static QString st = tr("st", "st as in 1st");
	static QString nd = tr("nd", "nd as in 2nd");
	static QString rd = tr("rd", "rd as in 3rd");
	static QString th = tr("th", "th as in 4th");
	if (day == 11 || day == 12 || day == 13)
		return th;
	if (day % 10 == 1)
		return st;
	if (day % 10 == 2)
		return nd;
	if (day % 10 == 3)
		return rd;
	return th;
}

QString Ya::DateFormatter::dayWithSuffix(int day)
{
	return tr("%1%2", "1-day with 2-suffix").arg(day)
	       .arg(daySuffix(day));
}

QString Ya::DateFormatter::dateAndWeekday(QDate date)
{
	return tr("%1, %2 %3", "1-day-of-week, 2-month 3-day")
	       .arg(daysOfWeek[(Qt::DayOfWeek)date.dayOfWeek()])
	       .arg(months[date.month()-1])
	       .arg(date.day());
}


QString Ya::DateFormatter::longDateTime(QDateTime dateTime)
{
	return tr("%1, %2 %3 %4, %5", "1-day-of-week, 2-month 3-day 4-year, 5-time")
	       .arg(daysOfWeek[(Qt::DayOfWeek)dateTime.date().dayOfWeek()])
	       .arg(months[dateTime.date().month()-1])
	       .arg(dateTime.date().day())
	       .arg(dateTime.date().year())
	       .arg(dateTime.toString(tr("hh:mm:ss")));
}

QString Ya::AgeFormatter::ageSuffix(int age)
{
	static QString god_singular = tr("year", "1 year / 1 год");
	static QString god_plural   = tr("years", "21 years / 21 год");
	static QString goda         = tr("years", "2 years / 2 года");
	static QString let          = tr("years", "10 years / 10 лет");
	if (age == 1)
		return god_singular;
	if (age % 10 == 0 || (age >= 11 && age <= 19))
		return let;
	if (age % 10 == 1)
		return god_plural;
	if (age % 10 == 2 || age % 10 == 3 || age % 10 == 4)
		return goda;
	return let;
}

QString Ya::AgeFormatter::ageInYears(int age)
{
	return tr("%1 %2", "1-age-number, 2-age-suffix")
	       .arg(age)
	       .arg(ageSuffix(age));
}
