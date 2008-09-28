/*
 * psievent.h - events
 * Copyright (C) 2001, 2002  Justin Karneges
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

#include "psievent.h"

#include <qdom.h>
#include <QTextStream>
#include <QList>
#include <QCoreApplication>

#include "psicon.h"
#include "psiaccount.h"
#include "xmpp_xmlcommon.h"
#include "dummystream.h"
#include "filetransfer.h"
#include "applicationinfo.h"
#include "psicontactlist.h"
#include "atomicxmlfile.h"
#include "globaleventqueue.h"
#ifdef YAPSI
#include "psioptions.h"
#endif

using namespace XMPP;
using namespace XMLHelper;

//----------------------------------------------------------------------------
// PsiEvent
//----------------------------------------------------------------------------
PsiEvent::PsiEvent(PsiAccount *acc)
{
	v_originLocal = false;
	v_late = false;
	v_account = acc;
#ifdef YAPSI_ACTIVEX_SERVER
	v_shownInOnline = false;
#endif
#ifdef YAPSI
	v_id = -1;
#endif
}

PsiEvent::PsiEvent(const PsiEvent &from)
: QObject()
{
	v_originLocal = from.v_originLocal;
	v_late = from.v_late;
	v_ts = from.v_ts;
	v_jid = from.v_jid;
	v_account = from.v_account;
#ifdef YAPSI_ACTIVEX_SERVER
	v_shownInOnline = from.v_shownInOnline;
#endif
#ifdef YAPSI
	v_id = from.v_id;
#endif
}

PsiEvent::~PsiEvent()
{
}

XMPP::Jid PsiEvent::jid() const
{
	return v_jid;
}

void PsiEvent::setJid(const XMPP::Jid &j)
{
	v_jid = j;
}

PsiAccount *PsiEvent::account() const
{
	return v_account;
}

bool PsiEvent::originLocal() const
{
	return v_originLocal;
}

bool PsiEvent::late() const
{
	return v_late;
}

QDateTime PsiEvent::timeStamp() const
{
	return v_ts;
}

void PsiEvent::setOriginLocal(bool b)
{
	v_originLocal = b;
}

void PsiEvent::setLate(bool b)
{
	v_late = b;
}

void PsiEvent::setTimeStamp(const QDateTime &t)
{
	v_ts = t;
}

QDomElement PsiEvent::toXml(QDomDocument *doc) const
{
	QDomElement e = doc->createElement("event");
	e.setAttribute("type", className());
#ifdef YAPSI_ACTIVEX_SERVER
	e.setAttribute("shownInOnline", v_shownInOnline ? "true" : "false");
#endif
#ifdef YAPSI
	e.setAttribute("id", QString::number(v_id));
#endif

	e.appendChild( textTag(*doc, "originLocal",	v_originLocal) );
	e.appendChild( textTag(*doc, "late",		v_late) );
	e.appendChild( textTag(*doc, "ts",		v_ts.toString( Qt::ISODate )) );
	if ( !v_jid.full().isEmpty() )
		e.appendChild( textTag(*doc, "jid",		v_jid.full()) );

	if ( v_account )
		e.appendChild( textTag(*doc, "account",	v_account->name()) );

	return e;
}

bool PsiEvent::fromXml(PsiCon *psi, PsiAccount *account, const QDomElement *e)
{
	if ( e->tagName() != "event" )
		return false;
	if ( e->attribute("type") != className() )
		return false;
#ifdef YAPSI_ACTIVEX_SERVER
	v_shownInOnline = e->attribute("shownInOnline") == "true";
#endif
#ifdef YAPSI
	v_id = e->attribute("id").toInt();
#endif

	readBoolEntry(*e, "originLocal", &v_originLocal);
	readBoolEntry(*e, "late", &v_late);
	v_ts  = QDateTime::fromString(subTagText(*e, "ts"), Qt::ISODate);
	v_jid = Jid( subTagText(*e, "jid") );

	if ( account ) {
		v_account = account;
	}
	else if ( hasSubTag(*e, "account") ) {
		QString accName = subTagText(*e, "account");
		foreach(PsiAccount* account, psi->contactList()->accounts()) {
			if ( account->name() == accName ) {
				v_account = account;
				break;
			}
		}
	}

	return true;
}

int PsiEvent::priority() const
{
	return Options::EventPriorityDontCare;
}

QString PsiEvent::description() const
{
	return QString();
}

PsiEvent *PsiEvent::copy() const
{
	return 0;
}

#ifdef YAPSI_ACTIVEX_SERVER
bool PsiEvent::shownInOnline() const
{
	return v_shownInOnline;
}

void PsiEvent::setShownInOnline(bool shownInOnline)
{
	v_shownInOnline = shownInOnline;
}
#endif

#ifdef YAPSI
int PsiEvent::id() const
{
	return v_id;
}

void PsiEvent::setId(int id)
{
	v_id = id;
}
#endif

//----------------------------------------------------------------------------
// MessageEvent
//----------------------------------------------------------------------------

MessageEvent::MessageEvent(PsiAccount *acc)
: PsiEvent(acc)
{
	v_sentToChatWindow = false;
}

MessageEvent::MessageEvent(const XMPP::Message &m, PsiAccount *acc)
: PsiEvent(acc)
{
	v_sentToChatWindow = false;
	setMessage(m);
}

MessageEvent::MessageEvent(const MessageEvent &from)
: PsiEvent(from), v_m(from.v_m), v_sentToChatWindow(from.v_sentToChatWindow)
{
}

MessageEvent::~MessageEvent()
{
}

int MessageEvent::type() const
{
	return Message;
}

Jid MessageEvent::from() const
{
	return v_m.from();
}

void MessageEvent::setFrom(const Jid &j)
{
	v_m.setFrom(j);
}

const QString& MessageEvent::nick() const
{
	return v_m.nick();
}

void MessageEvent::setNick(const QString &nick)
{
	v_m.setNick(nick);
}

bool MessageEvent::sentToChatWindow() const
{
	return v_sentToChatWindow;
}

const XMPP::Message & MessageEvent::message() const
{
	return v_m;
}

void MessageEvent::setSentToChatWindow(bool b)
{
	v_sentToChatWindow = b;
}

void MessageEvent::setMessage(const XMPP::Message &m)
{
	v_m = m;
	setTimeStamp ( v_m.timeStamp() );
	setLate ( v_m.spooled() );
}

QDomElement MessageEvent::toXml(QDomDocument *doc) const
{
	QDomElement e = PsiEvent::toXml(doc);

	DummyStream stream;
	Stanza s = v_m.toStanza(&stream);
	e.appendChild( s.element() );

	return e;
}

bool MessageEvent::fromXml(PsiCon *psi, PsiAccount *account, const QDomElement *e)
{
	if ( !PsiEvent::fromXml(psi, account, e) )
		return false;

	bool found = false;
	QDomElement msg = findSubTag(*e, "message", &found);
	if ( found ) {
		DummyStream stream;
		Stanza s = stream.createStanza(addCorrectNS(msg));
		v_m.fromStanza(s, 0); // FIXME: fix tzoffset?

		// if message was not spooled, it will be initialized with the
		// current datetime. we want to reset it back to the original
		// receive time
		if (!v_m.timeStamp().secsTo(QDateTime::currentDateTime()))
			v_m.setTimeStamp(timeStamp());

		return true;
	}

	return false;
}

int MessageEvent::priority() const
{
	if ( v_m.type() == "headline" )
		return option.eventPriorityHeadline;
	else if ( v_m.type() == "chat" )
		return option.eventPriorityChat;

	return option.eventPriorityMessage;
}

QString MessageEvent::description() const
{
	QStringList result;
	if (!v_m.subject().isEmpty())
		result << v_m.subject();
	if (!v_m.body().isEmpty())
		result << v_m.body();
	foreach(Url url, v_m.urlList()) {
		QString text = url.url();
		if (!url.desc().isEmpty())
			text += QString("(%1)").arg(url.desc());
		result << text;
	}
	return result.join("\n");
}

PsiEvent *MessageEvent::copy() const
{
	return new MessageEvent( *this );
}

//----------------------------------------------------------------------------
// AuthEvent
//----------------------------------------------------------------------------

AuthEvent::AuthEvent(const Jid &j, const QString &authType, PsiAccount *acc)
: PsiEvent(acc)
{
	v_from = j;
	v_at = authType;
}

AuthEvent::AuthEvent(const AuthEvent &from)
: PsiEvent(from), v_from(from.v_from), v_at(from.v_at)
{
}

AuthEvent::~AuthEvent()
{
}

int AuthEvent::type() const
{
	return Auth;
}

Jid AuthEvent::from() const
{
	return v_from;
}

void AuthEvent::setFrom(const Jid &j)
{
	v_from = j;
}

const QString& AuthEvent::nick() const
{
	return v_nick;
}

void AuthEvent::setNick(const QString &nick)
{
	v_nick = nick;
}

QString AuthEvent::authType() const
{
	return v_at;
}

QDomElement AuthEvent::toXml(QDomDocument *doc) const
{
	QDomElement e = PsiEvent::toXml(doc);

	e.appendChild( textTag(*doc, "from",	 v_from.full()) );
	e.appendChild( textTag(*doc, "authType", v_at) );
	e.appendChild( textTag(*doc, "nick", v_nick) );

	return e;
}

bool AuthEvent::fromXml(PsiCon *psi, PsiAccount *account, const QDomElement *e)
{
	if ( !PsiEvent::fromXml(psi, account, e) )
		return false;

	v_from = Jid( subTagText(*e, "from") );
	v_at   = subTagText(*e, "authType");
	v_nick = subTagText(*e, "nick");

	return true;
}

int AuthEvent::priority() const
{
	return option.eventPriorityAuth;
}

QString AuthEvent::description() const
{
	QString result;
	if (authType() == "subscribe")
		result = tr("%1 wants to subscribe to your presence.").arg(from().bare());
	else if (authType() == "subscribed")
		result = tr("%1 authorized you to view his status.").arg(from().bare());
	else if (authType() == "unsubscribed")
		result = tr("%1 removed your authorization to view his status!").arg(from().bare());
	else
		Q_ASSERT(false);

	return result;
}

PsiEvent *AuthEvent::copy() const
{
	return new AuthEvent( *this );
}

#ifdef YAPSI
//----------------------------------------------------------------------------
// MoodEvent
//----------------------------------------------------------------------------
MoodEvent::MoodEvent(const XMPP::Jid& j, const QString& mood, PsiAccount* acc)
	: PsiEvent(acc)
	, v_mood(mood)
{
	setTimeStamp(QDateTime::currentDateTime());
	setJid(j);
}

MoodEvent::MoodEvent(const MoodEvent& from)
	: PsiEvent(from)
	, v_mood(from.v_mood)
{
}

MoodEvent::~MoodEvent()
{
}

int MoodEvent::type() const
{
	return Mood;
}

XMPP::Jid MoodEvent::from() const
{
	return jid();
}

void MoodEvent::setFrom(const XMPP::Jid &j)
{
	setJid(j);
}

const QString& MoodEvent::mood() const
{
	return v_mood;
}

void MoodEvent::setMood(const QString& mood)
{
	v_mood = mood;
}

QDomElement MoodEvent::toXml(QDomDocument* doc) const
{
	QDomElement e = PsiEvent::toXml(doc);

	e.appendChild(textTag(*doc, "mood", v_mood));

	return e;
}

bool MoodEvent::fromXml(PsiCon* psi, PsiAccount* account, const QDomElement* e)
{
	if (!PsiEvent::fromXml(psi, account, e))
		return false;

	v_mood = subTagText(*e, "mood");

	return true;
}

int MoodEvent::priority() const
{
	return 0;
}

QString MoodEvent::description() const
{
	return tr("%1 changed mood to: %2").arg(from().bare()).arg(mood());
}

PsiEvent *MoodEvent::copy() const
{
	return new MoodEvent(*this);
}
#endif

//----------------------------------------------------------------------------
// FileEvent
//----------------------------------------------------------------------------
FileEvent::FileEvent(const Jid &j, FileTransfer *_ft, PsiAccount *acc)
:PsiEvent(acc)
{
	v_from = j;
	ft = _ft;
}

FileEvent::~FileEvent()
{
	delete ft;
}

FileEvent::FileEvent(const FileEvent &from)
: PsiEvent(from.account())
{
	v_from = from.v_from;
	ft = from.ft->copy();
}

int FileEvent::priority() const
{
	return option.eventPriorityFile;
}

Jid FileEvent::from() const
{
	return v_from;
}

void FileEvent::setFrom(const Jid &j)
{
	v_from = j;
}

FileTransfer *FileEvent::takeFileTransfer()
{
	FileTransfer *_ft = ft;
	ft = 0;
	return _ft;
}

QString FileEvent::description() const
{
	return tr("This user wants to send you a file.");
}

PsiEvent *FileEvent::copy() const
{
	return new FileEvent( *this );
}

//----------------------------------------------------------------------------
// HttpAuthEvent
//----------------------------------------------------------------------------

HttpAuthEvent::HttpAuthEvent(const PsiHttpAuthRequest &req, PsiAccount *acc)
:MessageEvent(acc), v_req(req)
{
	const XMPP::Stanza &s = req.stanza();

	XMPP::Message m;

	if ( s.kind() == XMPP::Stanza::Message ) {
		m.fromStanza(s, acc->client()->timeZoneOffset());
	}
	else {
		m.setFrom(s.from());
		m.setTimeStamp(QDateTime::currentDateTime());
		m.setHttpAuthRequest(HttpAuthRequest(s.element().elementsByTagNameNS("http://jabber.org/protocol/http-auth", "confirm").item(0).toElement()));
	}

	setMessage(m);
}

HttpAuthEvent::~HttpAuthEvent()
{
}

QString HttpAuthEvent::description() const
{
	return tr("HTTP Authentication Request");
}

//----------------------------------------------------------------------------
// RosterExchangeEvent
//----------------------------------------------------------------------------
RosterExchangeEvent::RosterExchangeEvent(const Jid &j, const RosterExchangeItems& i, const QString& text, PsiAccount *acc)
:PsiEvent(acc)
{
	v_from = j;
	v_items = i;
	v_text = text;
}

int RosterExchangeEvent::priority() const
{
	return option.eventPriorityRosterExchange;
}

Jid RosterExchangeEvent::from() const
{
	return v_from;
}

void RosterExchangeEvent::setFrom(const Jid &j)
{
	v_from = j;
}

const RosterExchangeItems& RosterExchangeEvent::rosterExchangeItems() const
{
	return v_items;
}

void RosterExchangeEvent::setRosterExchangeItems(const RosterExchangeItems& i)
{
	v_items = i;
}

const QString& RosterExchangeEvent::text() const
{
	return v_text;
}

void RosterExchangeEvent::setText(const QString& text)
{
	v_text = text;
}

QString RosterExchangeEvent::description() const
{
	return tr("This user wants to modify your roster.");
}

//----------------------------------------------------------------------------
// Status
//----------------------------------------------------------------------------
/*StatusEvent::StatusEvent(const Jid &j, const XMPP::Status& s, PsiAccount *acc)
:PsiEvent(acc)
{
	v_from = j;
	v_status = s;
}

int StatusEvent::priority() const
{
	return option.eventPriorityChat;
}

Jid StatusEvent::from() const
{
	return v_from;
}

void StatusEvent::setFrom(const Jid &j)
{
	v_from = j;
}

const XMPP::Status& StatusEvent::status() const
{
	return v_status;
}

void StatusEvent::setStatus(const XMPP::Status& s)
{
	v_status = s;
}*/

//----------------------------------------------------------------------------
// EventIdGenerator
//----------------------------------------------------------------------------

class EventIdGenerator : public QObject
{
	Q_OBJECT
public:
	static EventIdGenerator* instance();

	int getId();

private:
	static EventIdGenerator* instance_;
	int id_;

	EventIdGenerator();
};

EventIdGenerator* EventIdGenerator::instance_ = 0;

EventIdGenerator* EventIdGenerator::instance()
{
	if (!instance_) {
		instance_ = new EventIdGenerator();
	}
	return instance_;
}

static const QString idGeneratorOptionPath = "options.ya.last-event-id";

EventIdGenerator::EventIdGenerator()
	: QObject(QCoreApplication::instance())
{
#ifdef YAPSI
	id_ = PsiOptions::instance()->getOption(idGeneratorOptionPath).toInt();
#else
	id_ = 0;
#endif
}

int EventIdGenerator::getId()
{
	int result = id_;
#ifdef YAPSI
	// TODO: upgrade to uint64
	PsiOptions::instance()->setOption(idGeneratorOptionPath, ++id_);
#else
	++id_;
#endif

	if (id_ > 0x7FFFFFFF) {
		id_ = 0;
	}

	Q_ASSERT(id_ >= 0);
	Q_ASSERT(result >= 0);
	return result;
}

//----------------------------------------------------------------------------
// EventItem
//----------------------------------------------------------------------------

EventItem::EventItem(PsiEvent *_e)
{
	e = _e;
#ifdef YAPSI
	if (e->id() >= 0) {
		v_id = e->id();
	}
	else {
		Q_ASSERT(e->account());
		v_id = EventIdGenerator::instance()->getId();
		e->setId(v_id);
	}
#else
	Q_ASSERT(e->account());
	v_id = EventIdGenerator::instance()->getId();
#endif
}

EventItem::EventItem(const EventItem &from)
{
	e = from.e->copy();
	v_id = from.v_id;
}

EventItem::~EventItem()
{
}

int EventItem::id() const
{
	return v_id;
}

PsiEvent* EventItem::event() const
{
	return e;
}

//----------------------------------------------------------------------------
// CallEvent
//----------------------------------------------------------------------------
CallEvent::CallEvent(TINSSession *s, const Jid &j, PsiAccount *acc)
:PsiEvent(acc)
{
	v_from = j;
    session = s;
    eventTimer = new QTimer(this);
    eventTimer->start(60*1000, true);
    connect(eventTimer, SIGNAL(timeout()), acc, SLOT(dequeueTimeoutEvent()));
}

CallEvent::~CallEvent()
{
    eventTimer->stop();
    delete eventTimer;
}

void CallEvent::stopTimer()
{
    eventTimer->stop();
}

Jid CallEvent::from() const
{
	return v_from;
}

void CallEvent::setFrom(const Jid &j)
{
	v_from = j;
}

TINSSession * CallEvent::tinsSession()
{
    return session;
}

//----------------------------------------------------------------------------
// EventQueue
//----------------------------------------------------------------------------

EventQueue::EventQueue(PsiAccount *account)
	: psi_(0)
	, account_(0)
	, enabled_(false)
{
	account_ = account;
	psi_ = account_->psi();
}

EventQueue::EventQueue(const EventQueue &from)
	: QObject()
	, psi_(0)
	, account_(0)
	, enabled_(false)
{
	Q_ASSERT(false);
}

EventQueue::~EventQueue()
{
}

bool EventQueue::enabled() const
{
	return enabled_;
}

void EventQueue::setEnabled(bool enabled)
{
	if (enabled_ != enabled) {
		enabled_ = enabled;
		foreach(EventItem* i, list_) {
			if (enabled)
				GlobalEventQueue::instance()->enqueue(i);
			else
				GlobalEventQueue::instance()->dequeue(i);
		}
	}
}

EventQueue &EventQueue::operator= (const EventQueue &from)
{
	while(!list_.isEmpty())
		delete list_.takeFirst();

	psi_ = from.psi_;
	account_ = from.account_;

	foreach(EventItem *i, from.list_) {
		PsiEvent *e = i->event();
		enqueue( e->copy() );
	}

	return *this;
}

int EventQueue::nextId() const
{
	if (list_.isEmpty())
		return -1;

	EventItem *i = list_.first();
	if(!i)
		return -1;
	return i->id();
}

int EventQueue::count() const
{
	return list_.count();
}

int EventQueue::count(const Jid &j, bool compareRes) const
{
	int total = 0;
	foreach(EventItem *i, list_) {
		Jid j2(i->event()->jid());
		if(j.compare(j2, compareRes))
			++total;
	}
	return total;
}

void EventQueue::enqueue(PsiEvent *e)
{
	EventItem *i = new EventItem(e);

	if (enabled_) {
		GlobalEventQueue::instance()->enqueue(i);
	}

	int prior  = e->priority();
	bool found = false;

	// skip all with higher or equal priority
	foreach(EventItem *ei, list_) {
		if (ei && ei->event()->priority() < prior ) {
			list_.insert(list_.find(ei), i);
			found = true;
			break;
		}
	}

	// everything else
	if ( !found )
		list_.append(i);

	emit queueChanged();
}

void EventQueue::dequeue(PsiEvent *e)
{
	if ( !e )
		return;

	foreach(EventItem *i, list_) {
		if ( e == i->event() ) {
			if (enabled_) {
				GlobalEventQueue::instance()->dequeue(i);
			}
			list_.remove(i);
			emit queueChanged();
			delete i;
			return;
		}
	}
}

PsiEvent *EventQueue::dequeue(const Jid &j, bool compareRes)
{
	foreach(EventItem *i, list_) {
		PsiEvent *e = i->event();
		Jid j2(e->jid());
		if(j.compare(j2, compareRes)) {
			if (enabled_) {
				GlobalEventQueue::instance()->dequeue(i);
			}
			list_.remove(i);
			emit queueChanged();
			delete i;
			return e;
		}
	}

	return 0;
}

PsiEvent *EventQueue::peek(const Jid &j, bool compareRes) const
{
	foreach(EventItem *i, list_) {
		PsiEvent *e = i->event();
		Jid j2(e->jid());
		if(j.compare(j2, compareRes)) {
			return e;
		}
	}

	return 0;
}

PsiEvent *EventQueue::dequeueNext()
{
	if (list_.isEmpty())
		return 0;

	EventItem *i = list_.first();
	if(!i)
		return 0;
	PsiEvent *e = i->event();
	if (enabled_) {
		GlobalEventQueue::instance()->dequeue(i);
	}
	list_.remove(i);
	emit queueChanged();
	delete i;
	return e;
}

PsiEvent *EventQueue::peekNext() const
{
	if (list_.isEmpty())
		return 0;

	EventItem *i = list_.first();
	if(!i)
		return 0;
	return i->event();
}

PsiEvent *EventQueue::peekFirstChat(const Jid &j, bool compareRes) const
{
	foreach(EventItem *i, list_) {
		PsiEvent *e = i->event();
		if(e->type() == PsiEvent::Message) {
			MessageEvent *me = (MessageEvent *)e;
			if(j.compare(me->from(), compareRes) && me->message().type() == "chat")
				return e;
		}
	}

	return 0;
}

bool EventQueue::hasChats(const Jid &j, bool compareRes) const
{
	return (peekFirstChat(j, compareRes) ? true: false);
}

// this function extracts all chats from the queue, and returns a list of queue positions
void EventQueue::extractChats(QList<PsiEvent*> *el, const Jid &j, bool compareRes)
{
	bool changed = false;

	for(QList<EventItem*>::Iterator it = list_.begin(); it != list_.end();) {
		PsiEvent *e = (*it)->event();
		bool extract = false;
		if(e->type() == PsiEvent::Message) {
			MessageEvent *me = (MessageEvent *)e;
			if(j.compare(me->from(), compareRes) && me->message().type() == "chat") {
				extract = true;
			}
		}
#ifdef YAPSI
		else if (e->type() == PsiEvent::Mood) {
			MoodEvent* moodEvent = static_cast<MoodEvent*>(e);
			if(j.compare(moodEvent->from(), false)) {
				extract = true;
			}
		}
#endif

		if (extract) {
			el->append(e);
			EventItem* ei = *it;
			if (enabled_) {
				GlobalEventQueue::instance()->dequeue(ei);
			}
			it = list_.erase(it);
			delete ei;
			changed = true;
			continue;
		}

		++it;
	}

	if ( changed )
		emit queueChanged();
}

// this function extracts all messages from the queue, and returns a list of them
void EventQueue::extractMessages(QList<PsiEvent*> *el)
{
	bool changed = false;

	for(QList<EventItem*>::Iterator it = list_.begin(); it != list_.end();) {
		PsiEvent *e = (*it)->event();
		if(e->type() == PsiEvent::Message) {
			el->append(e);
			EventItem* ei = *it;
			if (enabled_) {
				GlobalEventQueue::instance()->dequeue(ei);
			}
			it = list_.erase(it);
			delete ei;
			changed = true;
			continue;
		}
		++it;
	}

	if ( changed )
		emit queueChanged();
}

void EventQueue::printContent() const
{
	foreach(EventItem *i, list_) {
		PsiEvent *e = i->event();
		printf("  %d: (%d) from=[%s] jid=[%s]\n", i->id(), e->type(), e->from().full().latin1(), e->jid().full().latin1());
	}
}

void EventQueue::clear()
{
	while(!list_.isEmpty())
		delete list_.takeFirst();

	emit queueChanged();
}

// this function removes all events associated with the input jid
void EventQueue::clear(const Jid &j, bool compareRes)
{
	bool changed = false;

	for(QList<EventItem*>::Iterator it = list_.begin(); it != list_.end();) {
		PsiEvent *e = (*it)->event();
		Jid j2(e->jid());
		if(j.compare(j2, compareRes)) {
			EventItem* ei = *it;
			if (enabled_) {
				GlobalEventQueue::instance()->dequeue(ei);
			}
			it = list_.erase(it);
			delete ei;
			changed = true;
		}
		else
			++it;
	}

	if ( changed )
		emit queueChanged();
}

QDomElement EventQueue::toXml(QDomDocument *doc) const
{
	QDomElement e = doc->createElement("eventQueue");
	e.setAttribute("version", "1.0");
	e.appendChild(textTag(doc, "progver", ApplicationInfo::version()));

	foreach(EventItem *i, list_) {
		QDomElement event = i->event()->toXml(doc);
		e.appendChild( event );
	}

	return e;
}

bool EventQueue::fromXml(const QDomElement *q)
{
	if ( !q )
		return false;

	if ( q->tagName() != "eventQueue" )
		return false;

	if ( q->attribute("version") != "1.0" )
		return false;

	QString progver = subTagText(*q, "progver");

	for(QDomNode n = q->firstChild(); !n.isNull(); n = n.nextSibling()) {
		QDomElement e = n.toElement();
		if( e.isNull() )
			continue;

		if ( e.tagName() != "event" )
			continue;

		PsiEvent *event = 0;
		QString eventType = e.attribute("type");
		if ( eventType == "MessageEvent" ) {
			event = new MessageEvent(0);
			if ( !event->fromXml(psi_, account_, &e) ) {
				delete event;
				event = 0;
			}
		}
		else if ( eventType == "AuthEvent" ) {
			event = new AuthEvent("", "", 0);
			if ( !event->fromXml(psi_, account_, &e) ) {
				delete event;
				event = 0;
			}
		}
		else if ( eventType == "MoodEvent" ) {
			event = new MoodEvent("", "", 0);
			if ( !event->fromXml(psi_, account_, &e) ) {
				delete event;
				event = 0;
			}
		}

		if ( event )
			emit eventFromXml( event );
	}

	return true;
}

bool EventQueue::toFile(const QString &fname)
{
	QDomDocument doc;
	QDomElement element = toXml(&doc);
	doc.appendChild(element);

	AtomicXmlFile f(fname);
	return f.saveDocument(doc);
}

bool EventQueue::fromFile(const QString &fname)
{
	AtomicXmlFile f(fname);
	QDomDocument doc;
	if (!f.loadDocument(&doc))
		return false;

	QDomElement base = doc.documentElement();
	return fromXml(&base);
}

#include "psievent.moc"
