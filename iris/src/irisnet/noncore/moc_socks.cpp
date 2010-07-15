/****************************************************************************
** Meta object code from reading C++ file 'socks.h'
**
** Created: Thu Jul 15 01:35:31 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "cutestuff/socks.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'socks.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SocksUDP[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      15,   10,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
      39,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SocksUDP[] = {
    "SocksUDP\0\0data\0packetReady(QByteArray)\0"
    "sd_activated()\0"
};

const QMetaObject SocksUDP::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SocksUDP,
      qt_meta_data_SocksUDP, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SocksUDP::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SocksUDP::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SocksUDP::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SocksUDP))
        return static_cast<void*>(const_cast< SocksUDP*>(this));
    return QObject::qt_metacast(_clname);
}

int SocksUDP::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: packetReady((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 1: sd_activated(); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void SocksUDP::packetReady(const QByteArray & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
static const uint qt_meta_data_SocksClient[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      25,   12,   12,   12, 0x05,
      56,   46,   12,   12, 0x05,
      96,   86,   12,   12, 0x05,
     132,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
     162,   12,   12,   12, 0x08,
     179,   12,   12,   12, 0x08,
     203,   12,   12,   12, 0x08,
     231,   12,   12,   12, 0x08,
     248,   12,   12,   12, 0x08,
     271,   12,   12,   12, 0x08,
     287,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SocksClient[] = {
    "SocksClient\0\0connected()\0incomingMethods(int)\0"
    "user,pass\0incomingAuth(QString,QString)\0"
    "host,port\0incomingConnectRequest(QString,int)\0"
    "incomingUDPAssociateRequest()\0"
    "sock_connected()\0sock_connectionClosed()\0"
    "sock_delayedCloseFinished()\0"
    "sock_readyRead()\0sock_bytesWritten(int)\0"
    "sock_error(int)\0serve()\0"
};

const QMetaObject SocksClient::staticMetaObject = {
    { &ByteStream::staticMetaObject, qt_meta_stringdata_SocksClient,
      qt_meta_data_SocksClient, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SocksClient::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SocksClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SocksClient::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SocksClient))
        return static_cast<void*>(const_cast< SocksClient*>(this));
    return ByteStream::qt_metacast(_clname);
}

int SocksClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ByteStream::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connected(); break;
        case 1: incomingMethods((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: incomingAuth((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 3: incomingConnectRequest((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 4: incomingUDPAssociateRequest(); break;
        case 5: sock_connected(); break;
        case 6: sock_connectionClosed(); break;
        case 7: sock_delayedCloseFinished(); break;
        case 8: sock_readyRead(); break;
        case 9: sock_bytesWritten((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 10: sock_error((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: serve(); break;
        default: ;
        }
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void SocksClient::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void SocksClient::incomingMethods(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SocksClient::incomingAuth(const QString & _t1, const QString & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void SocksClient::incomingConnectRequest(const QString & _t1, int _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void SocksClient::incomingUDPAssociateRequest()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}
static const uint qt_meta_data_SocksServer[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,
      60,   29,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
     113,   12,   12,   12, 0x08,
     134,   12,   12,   12, 0x08,
     152,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SocksServer[] = {
    "SocksServer\0\0incomingReady()\0"
    "host,port,addr,sourcePort,data\0"
    "incomingUDP(QString,int,QHostAddress,int,QByteArray)\0"
    "connectionReady(int)\0connectionError()\0"
    "sd_activated()\0"
};

const QMetaObject SocksServer::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_SocksServer,
      qt_meta_data_SocksServer, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SocksServer::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SocksServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SocksServer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SocksServer))
        return static_cast<void*>(const_cast< SocksServer*>(this));
    return QObject::qt_metacast(_clname);
}

int SocksServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: incomingReady(); break;
        case 1: incomingUDP((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2])),(*reinterpret_cast< const QHostAddress(*)>(_a[3])),(*reinterpret_cast< int(*)>(_a[4])),(*reinterpret_cast< const QByteArray(*)>(_a[5]))); break;
        case 2: connectionReady((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: connectionError(); break;
        case 4: sd_activated(); break;
        default: ;
        }
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void SocksServer::incomingReady()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void SocksServer::incomingUDP(const QString & _t1, int _t2, const QHostAddress & _t3, int _t4, const QByteArray & _t5)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)), const_cast<void*>(reinterpret_cast<const void*>(&_t5)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
