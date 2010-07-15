/****************************************************************************
** Meta object code from reading C++ file 'compressionhandler.h'
**
** Created: Thu Jul 15 18:46:44 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../xmpp-core/compressionhandler.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'compressionhandler.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CompressionHandler[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   19,   19,   19, 0x05,
      32,   19,   19,   19, 0x05,
      52,   19,   19,   19, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_CompressionHandler[] = {
    "CompressionHandler\0\0readyRead()\0"
    "readyReadOutgoing()\0error()\0"
};

const QMetaObject CompressionHandler::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CompressionHandler,
      qt_meta_data_CompressionHandler, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CompressionHandler::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CompressionHandler::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CompressionHandler::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CompressionHandler))
        return static_cast<void*>(const_cast< CompressionHandler*>(this));
    return QObject::qt_metacast(_clname);
}

int CompressionHandler::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: readyRead(); break;
        case 1: readyReadOutgoing(); break;
        case 2: error(); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void CompressionHandler::readyRead()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void CompressionHandler::readyReadOutgoing()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void CompressionHandler::error()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
QT_END_MOC_NAMESPACE
