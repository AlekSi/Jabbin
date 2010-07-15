/****************************************************************************
** Meta object code from reading C++ file 'securestream.h'
**
** Created: Thu Jul 15 01:35:39 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../xmpp-core/securestream.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'securestream.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SecureStream[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x05,
      30,   13,   13,   13, 0x05,

 // slots: signature, parameters, type, tag, flags
      42,   13,   13,   13, 0x08,
      57,   13,   13,   13, 0x08,
      78,   13,   13,   13, 0x08,
     100,   13,   13,   13, 0x08,
     128,   13,   13,   13, 0x08,
     156,   13,   13,   13, 0x08,
     184,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SecureStream[] = {
    "SecureStream\0\0tlsHandshaken()\0tlsClosed()\0"
    "bs_readyRead()\0bs_bytesWritten(int)\0"
    "layer_tlsHandshaken()\0layer_tlsClosed(QByteArray)\0"
    "layer_readyRead(QByteArray)\0"
    "layer_needWrite(QByteArray)\0"
    "layer_error(int)\0"
};

const QMetaObject SecureStream::staticMetaObject = {
    { &ByteStream::staticMetaObject, qt_meta_stringdata_SecureStream,
      qt_meta_data_SecureStream, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SecureStream::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SecureStream::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SecureStream::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SecureStream))
        return static_cast<void*>(const_cast< SecureStream*>(this));
    return ByteStream::qt_metacast(_clname);
}

int SecureStream::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = ByteStream::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: tlsHandshaken(); break;
        case 1: tlsClosed(); break;
        case 2: bs_readyRead(); break;
        case 3: bs_bytesWritten((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: layer_tlsHandshaken(); break;
        case 5: layer_tlsClosed((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 6: layer_readyRead((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 7: layer_needWrite((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 8: layer_error((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void SecureStream::tlsHandshaken()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void SecureStream::tlsClosed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}
QT_END_MOC_NAMESPACE
