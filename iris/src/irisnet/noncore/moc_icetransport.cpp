/****************************************************************************
** Meta object code from reading C++ file 'icetransport.h'
**
** Created: Thu Jul 15 18:46:39 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "icetransport.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'icetransport.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_XMPP__IceTransport[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   19,   19,   19, 0x05,
      30,   19,   19,   19, 0x05,
      42,   40,   19,   19, 0x05,
      58,   53,   19,   19, 0x05,
      94,   73,   19,   19, 0x05,
     141,  137,   19,   19, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__IceTransport[] = {
    "XMPP::IceTransport\0\0started()\0stopped()\0"
    "e\0error(int)\0path\0readyRead(int)\0"
    "path,count,addr,port\0"
    "datagramsWritten(int,int,QHostAddress,int)\0"
    "str\0debugLine(QString)\0"
};

const QMetaObject XMPP::IceTransport::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__IceTransport,
      qt_meta_data_XMPP__IceTransport, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::IceTransport::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::IceTransport::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::IceTransport::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__IceTransport))
        return static_cast<void*>(const_cast< IceTransport*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::IceTransport::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: started(); break;
        case 1: stopped(); break;
        case 2: error((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: readyRead((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: datagramsWritten((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QHostAddress(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4]))); break;
        case 5: debugLine((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void XMPP::IceTransport::started()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void XMPP::IceTransport::stopped()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void XMPP::IceTransport::error(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void XMPP::IceTransport::readyRead(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void XMPP::IceTransport::datagramsWritten(int _t1, int _t2, const QHostAddress & _t3, int _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void XMPP::IceTransport::debugLine(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_END_MOC_NAMESPACE
