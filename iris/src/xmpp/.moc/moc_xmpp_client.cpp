/****************************************************************************
** Meta object code from reading C++ file 'xmpp_client.h'
**
** Created: Thu Jul 15 15:30:29 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../xmpp-im/xmpp_client.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'xmpp_client.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_XMPP__Client[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      32,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      21,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,
      26,   13,   13,   13, 0x05,
      44,   41,   13,   13, 0x05,
      84,   13,   13,   13, 0x05,
     112,   13,   13,   13, 0x05,
     142,   13,   13,   13, 0x05,
     174,  172,   13,   13, 0x05,
     206,  172,   13,   13, 0x05,
     240,   41,   13,   13, 0x05,
     271,   41,   13,   13, 0x05,
     305,   13,   13,   13, 0x05,
     330,   13,   13,   13, 0x05,
     349,   13,   13,   13, 0x05,
     370,   13,   13,   13, 0x05,
     391,   13,   13,   13, 0x05,
     412,   13,   13,   13, 0x05,
     431,  172,   13,   13, 0x05,
     461,   41,   13,   13, 0x05,
     493,   13,   13,   13, 0x05,
     511,   13,   13,   13, 0x05,
     531,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
     549,   13,   13,   13, 0x08,
     566,   13,   13,   13, 0x08,
     584,   13,   13,   13, 0x08,
     611,   13,   13,   13, 0x08,
     638,   13,   13,   13, 0x08,
     666,   41,   13,   13, 0x08,
     702,  172,   13,   13, 0x08,
     725,   13,   13,   13, 0x08,
     744,   13,   13,   13, 0x08,
     761,   13,   13,   13, 0x08,
     781,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__Client[] = {
    "XMPP::Client\0\0activated()\0disconnected()\0"
    ",,\0rosterRequestFinished(bool,int,QString)\0"
    "rosterItemAdded(RosterItem)\0"
    "rosterItemUpdated(RosterItem)\0"
    "rosterItemRemoved(RosterItem)\0,\0"
    "resourceAvailable(Jid,Resource)\0"
    "resourceUnavailable(Jid,Resource)\0"
    "presenceError(Jid,int,QString)\0"
    "subscription(Jid,QString,QString)\0"
    "messageReceived(Message)\0debugText(QString)\0"
    "xmlIncoming(QString)\0xmlOutgoing(QString)\0"
    "groupChatJoined(Jid)\0groupChatLeft(Jid)\0"
    "groupChatPresence(Jid,Status)\0"
    "groupChatError(Jid,int,QString)\0"
    "incomingJidLink()\0beginImportRoster()\0"
    "endImportRoster()\0streamError(int)\0"
    "streamReadyRead()\0streamIncomingXml(QString)\0"
    "streamOutgoingXml(QString)\0"
    "slotRosterRequestFinished()\0"
    "ppSubscription(Jid,QString,QString)\0"
    "ppPresence(Jid,Status)\0pmMessage(Message)\0"
    "prRoster(Roster)\0s5b_incomingReady()\0"
    "ibb_incomingReady()\0"
};

const QMetaObject XMPP::Client::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__Client,
      qt_meta_data_XMPP__Client, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::Client::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::Client::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::Client::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__Client))
        return static_cast<void*>(const_cast< Client*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::Client::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: activated(); break;
        case 1: disconnected(); break;
        case 2: rosterRequestFinished((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 3: rosterItemAdded((*reinterpret_cast< const RosterItem(*)>(_a[1]))); break;
        case 4: rosterItemUpdated((*reinterpret_cast< const RosterItem(*)>(_a[1]))); break;
        case 5: rosterItemRemoved((*reinterpret_cast< const RosterItem(*)>(_a[1]))); break;
        case 6: resourceAvailable((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const Resource(*)>(_a[2]))); break;
        case 7: resourceUnavailable((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const Resource(*)>(_a[2]))); break;
        case 8: presenceError((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 9: subscription((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 10: messageReceived((*reinterpret_cast< const Message(*)>(_a[1]))); break;
        case 11: debugText((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 12: xmlIncoming((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 13: xmlOutgoing((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 14: groupChatJoined((*reinterpret_cast< const Jid(*)>(_a[1]))); break;
        case 15: groupChatLeft((*reinterpret_cast< const Jid(*)>(_a[1]))); break;
        case 16: groupChatPresence((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const Status(*)>(_a[2]))); break;
        case 17: groupChatError((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 18: incomingJidLink(); break;
        case 19: beginImportRoster(); break;
        case 20: endImportRoster(); break;
        case 21: streamError((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 22: streamReadyRead(); break;
        case 23: streamIncomingXml((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 24: streamOutgoingXml((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 25: slotRosterRequestFinished(); break;
        case 26: ppSubscription((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        case 27: ppPresence((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const Status(*)>(_a[2]))); break;
        case 28: pmMessage((*reinterpret_cast< const Message(*)>(_a[1]))); break;
        case 29: prRoster((*reinterpret_cast< const Roster(*)>(_a[1]))); break;
        case 30: s5b_incomingReady(); break;
        case 31: ibb_incomingReady(); break;
        default: ;
        }
        _id -= 32;
    }
    return _id;
}

// SIGNAL 0
void XMPP::Client::activated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void XMPP::Client::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void XMPP::Client::rosterRequestFinished(bool _t1, int _t2, const QString & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void XMPP::Client::rosterItemAdded(const RosterItem & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void XMPP::Client::rosterItemUpdated(const RosterItem & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void XMPP::Client::rosterItemRemoved(const RosterItem & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void XMPP::Client::resourceAvailable(const Jid & _t1, const Resource & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void XMPP::Client::resourceUnavailable(const Jid & _t1, const Resource & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void XMPP::Client::presenceError(const Jid & _t1, int _t2, const QString & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void XMPP::Client::subscription(const Jid & _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void XMPP::Client::messageReceived(const Message & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void XMPP::Client::debugText(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void XMPP::Client::xmlIncoming(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void XMPP::Client::xmlOutgoing(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void XMPP::Client::groupChatJoined(const Jid & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void XMPP::Client::groupChatLeft(const Jid & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void XMPP::Client::groupChatPresence(const Jid & _t1, const Status & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void XMPP::Client::groupChatError(const Jid & _t1, int _t2, const QString & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}

// SIGNAL 18
void XMPP::Client::incomingJidLink()
{
    QMetaObject::activate(this, &staticMetaObject, 18, 0);
}

// SIGNAL 19
void XMPP::Client::beginImportRoster()
{
    QMetaObject::activate(this, &staticMetaObject, 19, 0);
}

// SIGNAL 20
void XMPP::Client::endImportRoster()
{
    QMetaObject::activate(this, &staticMetaObject, 20, 0);
}
QT_END_MOC_NAMESPACE
