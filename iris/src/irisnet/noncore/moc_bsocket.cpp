/****************************************************************************
** Meta object code from reading C++ file 'bsocket.h'
**
** Created: Thu Jul 15 01:35:30 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "cutestuff/bsocket.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'bsocket.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_BSocket[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
       9,    8,    8,    8, 0x05,
      21,    8,    8,    8, 0x05,

 // slots: signature, parameters, type, tag, flags
      33,    8,    8,    8, 0x08,
      48,    8,    8,    8, 0x08,
      63,    8,    8,    8, 0x08,
      75,    8,    8,    8, 0x08,
      90,    8,    8,    8, 0x08,
     114,    8,    8,    8, 0x08,
     153,    8,    8,    8, 0x08,
     164,    8,    8,    8, 0x08,
     176,    8,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_BSocket[] = {
    "BSocket\0\0hostFound()\0connected()\0"
    "qs_hostFound()\0qs_connected()\0qs_closed()\0"
    "qs_readyRead()\0qs_bytesWritten(qint64)\0"
    "qs_error(QAbstractSocket::SocketError)\0"
    "srv_done()\0ndns_done()\0do_connect()\0"
};

const QMetaObject BSocket::staticMetaObject = {
    { &ByteStream::staticMetaObject, qt_meta_stringdata_BSocket,
      qt_meta_data_BSocket, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &BSocket::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *BSocket::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *BSocket::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_BSocket))
        return static_cast<void*>(const_cast< BSocket*>(this));
    return ByteStream::qt_metacast(_clname);
}

int BSocket::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ByteStream::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: hostFound(); break;
        case 1: connected(); break;
        case 2: qs_hostFound(); break;
        case 3: qs_connected(); break;
        case 4: qs_closed(); break;
        case 5: qs_readyRead(); break;
        case 6: qs_bytesWritten((*reinterpret_cast< qint64(*)>(_a[1]))); break;
        case 7: qs_error((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 8: srv_done(); break;
        case 9: ndns_done(); break;
        case 10: do_connect(); break;
        default: ;
        }
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void BSocket::hostFound()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void BSocket::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
