/****************************************************************************
** Meta object code from reading C++ file 'httpconnect.h'
**
** Created: Thu Jul 15 01:35:30 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "cutestuff/httpconnect.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'httpconnect.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_HttpConnect[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      13,   12,   12,   12, 0x05,

 // slots: signature, parameters, type, tag, flags
      25,   12,   12,   12, 0x08,
      42,   12,   12,   12, 0x08,
      66,   12,   12,   12, 0x08,
      94,   12,   12,   12, 0x08,
     111,   12,   12,   12, 0x08,
     134,   12,   12,   12, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_HttpConnect[] = {
    "HttpConnect\0\0connected()\0sock_connected()\0"
    "sock_connectionClosed()\0"
    "sock_delayedCloseFinished()\0"
    "sock_readyRead()\0sock_bytesWritten(int)\0"
    "sock_error(int)\0"
};

const QMetaObject HttpConnect::staticMetaObject = {
    { &ByteStream::staticMetaObject, qt_meta_stringdata_HttpConnect,
      qt_meta_data_HttpConnect, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &HttpConnect::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *HttpConnect::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *HttpConnect::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_HttpConnect))
        return static_cast<void*>(const_cast< HttpConnect*>(this));
    return ByteStream::qt_metacast(_clname);
}

int HttpConnect::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ByteStream::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connected(); break;
        case 1: sock_connected(); break;
        case 2: sock_connectionClosed(); break;
        case 3: sock_delayedCloseFinished(); break;
        case 4: sock_readyRead(); break;
        case 5: sock_bytesWritten((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: sock_error((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void HttpConnect::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
