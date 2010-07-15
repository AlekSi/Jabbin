/****************************************************************************
** Meta object code from reading C++ file 'servsock.h'
**
** Created: Thu Jul 15 15:29:50 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "legacy/servsock.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'servsock.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ServSock[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      31,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ServSock[] = {
    "ServSock\0\0connectionReady(int)\0"
    "sss_connectionReady(int)\0"
};

const QMetaObject ServSock::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ServSock,
      qt_meta_data_ServSock, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ServSock::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ServSock::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ServSock::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ServSock))
        return static_cast<void*>(const_cast< ServSock*>(this));
    return QObject::qt_metacast(_clname);
}

int ServSock::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connectionReady((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: sss_connectionReady((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ServSock::connectionReady(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_ServSockSignal[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      16,   15,   15,   15, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_ServSockSignal[] = {
    "ServSockSignal\0\0connectionReady(int)\0"
};

const QMetaObject ServSockSignal::staticMetaObject = {
    { &QTcpServer::staticMetaObject, qt_meta_stringdata_ServSockSignal,
      qt_meta_data_ServSockSignal, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ServSockSignal::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ServSockSignal::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ServSockSignal::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ServSockSignal))
        return static_cast<void*>(const_cast< ServSockSignal*>(this));
    return QTcpServer::qt_metacast(_clname);
}

int ServSockSignal::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTcpServer::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connectionReady((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void ServSockSignal::connectionReady(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
