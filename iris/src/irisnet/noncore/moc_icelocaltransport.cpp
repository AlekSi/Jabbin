/****************************************************************************
** Meta object code from reading C++ file 'icelocaltransport.h'
**
** Created: Thu Jul 15 01:35:32 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "icelocaltransport.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'icelocaltransport.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_XMPP__IceLocalTransport[] = {

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
      25,   24,   24,   24, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__IceLocalTransport[] = {
    "XMPP::IceLocalTransport\0\0addressesChanged()\0"
};

const QMetaObject XMPP::IceLocalTransport::staticMetaObject = {
    { &IceTransport::staticMetaObject, qt_meta_stringdata_XMPP__IceLocalTransport,
      qt_meta_data_XMPP__IceLocalTransport, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::IceLocalTransport::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::IceLocalTransport::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::IceLocalTransport::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__IceLocalTransport))
        return static_cast<void*>(const_cast< IceLocalTransport*>(this));
    return IceTransport::qt_metacast(_clname);
}

int XMPP::IceLocalTransport::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = IceTransport::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: addressesChanged(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void XMPP::IceLocalTransport::addressesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
