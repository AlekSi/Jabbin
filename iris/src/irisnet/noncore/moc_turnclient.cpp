/****************************************************************************
** Meta object code from reading C++ file 'turnclient.h'
**
** Created: Thu Jul 15 15:29:48 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "turnclient.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'turnclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_XMPP__TurnClient[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x05,
      30,   17,   17,   17, 0x05,
      46,   17,   17,   17, 0x05,
      55,   17,   17,   17, 0x05,
      72,   17,   17,   17, 0x05,
      83,   17,   17,   17, 0x05,
      95,   17,   17,   17, 0x05,
     123,  107,   17,   17, 0x05,
     162,  160,   17,   17, 0x05,
     197,  193,   17,   17, 0x05,
     231,  226,   17,   17, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__TurnClient[] = {
    "XMPP::TurnClient\0\0connected()\0"
    "tlsHandshaken()\0closed()\0needAuthParams()\0"
    "retrying()\0activated()\0readyRead()\0"
    "count,addr,port\0packetsWritten(int,QHostAddress,int)\0"
    "e\0error(XMPP::TurnClient::Error)\0buf\0"
    "outgoingDatagram(QByteArray)\0line\0"
    "debugLine(QString)\0"
};

const QMetaObject XMPP::TurnClient::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__TurnClient,
      qt_meta_data_XMPP__TurnClient, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::TurnClient::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::TurnClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::TurnClient::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__TurnClient))
        return static_cast<void*>(const_cast< TurnClient*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::TurnClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connected(); break;
        case 1: tlsHandshaken(); break;
        case 2: closed(); break;
        case 3: needAuthParams(); break;
        case 4: retrying(); break;
        case 5: activated(); break;
        case 6: readyRead(); break;
        case 7: packetsWritten((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QHostAddress(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 8: error((*reinterpret_cast< XMPP::TurnClient::Error(*)>(_a[1]))); break;
        case 9: outgoingDatagram((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 10: debugLine((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void XMPP::TurnClient::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void XMPP::TurnClient::tlsHandshaken()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void XMPP::TurnClient::closed()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void XMPP::TurnClient::needAuthParams()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void XMPP::TurnClient::retrying()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void XMPP::TurnClient::activated()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void XMPP::TurnClient::readyRead()
{
    QMetaObject::activate(this, &staticMetaObject, 6, 0);
}

// SIGNAL 7
void XMPP::TurnClient::packetsWritten(int _t1, const QHostAddress & _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void XMPP::TurnClient::error(XMPP::TurnClient::Error _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void XMPP::TurnClient::outgoingDatagram(const QByteArray & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void XMPP::TurnClient::debugLine(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}
QT_END_MOC_NAMESPACE
