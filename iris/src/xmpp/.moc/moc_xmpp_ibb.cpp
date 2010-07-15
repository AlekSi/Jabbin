/****************************************************************************
** Meta object code from reading C++ file 'xmpp_ibb.h'
**
** Created: Thu Jul 15 15:30:26 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../xmpp-im/xmpp_ibb.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'xmpp_ibb.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_XMPP__IBBConnection[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      21,   20,   20,   20, 0x05,

 // slots: signature, parameters, type, tag, flags
      33,   20,   20,   20, 0x08,
      48,   20,   20,   20, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__IBBConnection[] = {
    "XMPP::IBBConnection\0\0connected()\0"
    "ibb_finished()\0trySend()\0"
};

const QMetaObject XMPP::IBBConnection::staticMetaObject = {
    { &ByteStream::staticMetaObject, qt_meta_stringdata_XMPP__IBBConnection,
      qt_meta_data_XMPP__IBBConnection, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::IBBConnection::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::IBBConnection::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::IBBConnection::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__IBBConnection))
        return static_cast<void*>(const_cast< IBBConnection*>(this));
    return ByteStream::qt_metacast(_clname);
}

int XMPP::IBBConnection::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ByteStream::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connected(); break;
        case 1: ibb_finished(); break;
        case 2: trySend(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void XMPP::IBBConnection::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_XMPP__IBBManager[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      43,   34,   17,   17, 0x08,
     116,   88,   17,   17, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__IBBManager[] = {
    "XMPP::IBBManager\0\0incomingReady()\0"
    "from,id,\0ibb_incomingRequest(Jid,QString,QDomElement)\0"
    "from,streamid,id,data,close\0"
    "ibb_incomingData(Jid,QString,QString,QByteArray,bool)\0"
};

const QMetaObject XMPP::IBBManager::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__IBBManager,
      qt_meta_data_XMPP__IBBManager, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::IBBManager::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::IBBManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::IBBManager::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__IBBManager))
        return static_cast<void*>(const_cast< IBBManager*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::IBBManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: incomingReady(); break;
        case 1: ibb_incomingRequest((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QDomElement(*)>(_a[3]))); break;
        case 2: ibb_incomingData((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QByteArray(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void XMPP::IBBManager::incomingReady()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_XMPP__JT_IBB[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   14,   13,   13, 0x05,
      92,   64,   13,   13, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__JT_IBB[] = {
    "XMPP::JT_IBB\0\0from,id,\0"
    "incomingRequest(Jid,QString,QDomElement)\0"
    "from,streamid,id,data,close\0"
    "incomingData(Jid,QString,QString,QByteArray,bool)\0"
};

const QMetaObject XMPP::JT_IBB::staticMetaObject = {
    { &Task::staticMetaObject, qt_meta_stringdata_XMPP__JT_IBB,
      qt_meta_data_XMPP__JT_IBB, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::JT_IBB::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::JT_IBB::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::JT_IBB::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__JT_IBB))
        return static_cast<void*>(const_cast< JT_IBB*>(this));
    return Task::qt_metacast(_clname);
}

int XMPP::JT_IBB::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Task::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: incomingRequest((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QDomElement(*)>(_a[3]))); break;
        case 1: incomingData((*reinterpret_cast< const Jid(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2])),(*reinterpret_cast< const QString(*)>(_a[3])),(*reinterpret_cast< const QByteArray(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void XMPP::JT_IBB::incomingRequest(const Jid & _t1, const QString & _t2, const QDomElement & _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void XMPP::JT_IBB::incomingData(const Jid & _t1, const QString & _t2, const QString & _t3, const QByteArray & _t4, bool _t5)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
