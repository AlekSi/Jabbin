/****************************************************************************
** Meta object code from reading C++ file 'xmpp_tasks.h'
**
** Created: Thu Jul 15 01:35:40 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../xmpp-im/xmpp_tasks.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'xmpp_tasks.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_XMPP__JT_Register[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_Register[] = {
    "XMPP::JT_Register\0"
};

const QMetaObject XMPP::JT_Register::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_Register,
      qt_meta_data_XMPP__JT_Register, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_Register::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_Register::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_Register::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_Register))
        return static_cast<void*>(const_cast< JT_Register*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_Register::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_UnRegister[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x08,
      39,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_UnRegister[] = {
    "XMPP::JT_UnRegister\0\0getFormFinished()\0"
    "unregFinished()\0"
};

const QMetaObject XMPP::JT_UnRegister::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_UnRegister,
      qt_meta_data_XMPP__JT_UnRegister, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_UnRegister::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_UnRegister::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_UnRegister::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_UnRegister))
        return static_cast<void*>(const_cast< JT_UnRegister*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_UnRegister::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: getFormFinished(); break;
        case 1: unregFinished(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}
static const uint qt_meta_data_XMPP__JT_Roster[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_Roster[] = {
    "XMPP::JT_Roster\0"
};

const QMetaObject XMPP::JT_Roster::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_Roster,
      qt_meta_data_XMPP__JT_Roster, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_Roster::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_Roster::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_Roster::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_Roster))
        return static_cast<void*>(const_cast< JT_Roster*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_Roster::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_PushRoster[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_PushRoster[] = {
    "XMPP::JT_PushRoster\0\0roster(Roster)\0"
};

const QMetaObject XMPP::JT_PushRoster::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_PushRoster,
      qt_meta_data_XMPP__JT_PushRoster, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_PushRoster::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_PushRoster::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_PushRoster::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_PushRoster))
        return static_cast<void*>(const_cast< JT_PushRoster*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_PushRoster::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: roster((*reinterpret_cast< const Roster(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void XMPP::JT_PushRoster::roster(const Roster & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_XMPP__JT_Presence[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_Presence[] = {
    "XMPP::JT_Presence\0"
};

const QMetaObject XMPP::JT_Presence::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_Presence,
      qt_meta_data_XMPP__JT_Presence, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_Presence::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_Presence::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_Presence::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_Presence))
        return static_cast<void*>(const_cast< JT_Presence*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_Presence::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_PushPresence[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      25,   23,   22,   22, 0x05,
      49,   46,   22,   22, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_PushPresence[] = {
    "XMPP::JT_PushPresence\0\0,\0presence(Jid,Status)\0"
    ",,\0subscription(Jid,QString,QString)\0"
};

const QMetaObject XMPP::JT_PushPresence::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_PushPresence,
      qt_meta_data_XMPP__JT_PushPresence, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_PushPresence::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_PushPresence::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_PushPresence::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_PushPresence))
        return static_cast<void*>(const_cast< JT_PushPresence*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_PushPresence::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: presence((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const Status(*)>(_a[2]))); break;
        case 1: subscription((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void XMPP::JT_PushPresence::presence(const Jid & _t1, const Status & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void XMPP::JT_PushPresence::subscription(const Jid & _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
static const uint qt_meta_data_XMPP__JT_Message[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_Message[] = {
    "XMPP::JT_Message\0"
};

const QMetaObject XMPP::JT_Message::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_Message,
      qt_meta_data_XMPP__JT_Message, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_Message::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_Message::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_Message::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_Message))
        return static_cast<void*>(const_cast< JT_Message*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_Message::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_PushMessage[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   21,   21,   21, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_PushMessage[] = {
    "XMPP::JT_PushMessage\0\0message(Message)\0"
};

const QMetaObject XMPP::JT_PushMessage::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_PushMessage,
      qt_meta_data_XMPP__JT_PushMessage, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_PushMessage::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_PushMessage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_PushMessage::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_PushMessage))
        return static_cast<void*>(const_cast< JT_PushMessage*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_PushMessage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: message((*reinterpret_cast< const Message(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void XMPP::JT_PushMessage::message(const Message & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_XMPP__JT_GetServices[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_GetServices[] = {
    "XMPP::JT_GetServices\0"
};

const QMetaObject XMPP::JT_GetServices::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_GetServices,
      qt_meta_data_XMPP__JT_GetServices, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_GetServices::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_GetServices::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_GetServices::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_GetServices))
        return static_cast<void*>(const_cast< JT_GetServices*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_GetServices::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_VCard[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_VCard[] = {
    "XMPP::JT_VCard\0"
};

const QMetaObject XMPP::JT_VCard::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_VCard,
      qt_meta_data_XMPP__JT_VCard, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_VCard::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_VCard::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_VCard::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_VCard))
        return static_cast<void*>(const_cast< JT_VCard*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_VCard::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_Search[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_Search[] = {
    "XMPP::JT_Search\0"
};

const QMetaObject XMPP::JT_Search::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_Search,
      qt_meta_data_XMPP__JT_Search, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_Search::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_Search::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_Search::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_Search))
        return static_cast<void*>(const_cast< JT_Search*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_Search::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_ClientVersion[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_ClientVersion[] = {
    "XMPP::JT_ClientVersion\0"
};

const QMetaObject XMPP::JT_ClientVersion::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_ClientVersion,
      qt_meta_data_XMPP__JT_ClientVersion, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_ClientVersion::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_ClientVersion::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_ClientVersion::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_ClientVersion))
        return static_cast<void*>(const_cast< JT_ClientVersion*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_ClientVersion::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_ServInfo[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_ServInfo[] = {
    "XMPP::JT_ServInfo\0"
};

const QMetaObject XMPP::JT_ServInfo::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_ServInfo,
      qt_meta_data_XMPP__JT_ServInfo, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_ServInfo::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_ServInfo::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_ServInfo::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_ServInfo))
        return static_cast<void*>(const_cast< JT_ServInfo*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_ServInfo::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_Gateway[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_Gateway[] = {
    "XMPP::JT_Gateway\0"
};

const QMetaObject XMPP::JT_Gateway::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_Gateway,
      qt_meta_data_XMPP__JT_Gateway, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_Gateway::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_Gateway::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_Gateway::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_Gateway))
        return static_cast<void*>(const_cast< JT_Gateway*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_Gateway::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_Browse[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_Browse[] = {
    "XMPP::JT_Browse\0"
};

const QMetaObject XMPP::JT_Browse::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_Browse,
      qt_meta_data_XMPP__JT_Browse, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_Browse::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_Browse::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_Browse::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_Browse))
        return static_cast<void*>(const_cast< JT_Browse*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_Browse::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_DiscoItems[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_DiscoItems[] = {
    "XMPP::JT_DiscoItems\0"
};

const QMetaObject XMPP::JT_DiscoItems::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_DiscoItems,
      qt_meta_data_XMPP__JT_DiscoItems, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_DiscoItems::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_DiscoItems::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_DiscoItems::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_DiscoItems))
        return static_cast<void*>(const_cast< JT_DiscoItems*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_DiscoItems::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__JT_DiscoPublish[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_DiscoPublish[] = {
    "XMPP::JT_DiscoPublish\0"
};

const QMetaObject XMPP::JT_DiscoPublish::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_DiscoPublish,
      qt_meta_data_XMPP__JT_DiscoPublish, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_DiscoPublish::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_DiscoPublish::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_DiscoPublish::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_DiscoPublish))
        return static_cast<void*>(const_cast< JT_DiscoPublish*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_DiscoPublish::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
QT_END_MOC_NAMESPACE
