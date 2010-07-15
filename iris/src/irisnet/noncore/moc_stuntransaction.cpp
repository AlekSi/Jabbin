/****************************************************************************
** Meta object code from reading C++ file 'stuntransaction.h'
**
** Created: Thu Jul 15 01:35:31 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "stuntransaction.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'stuntransaction.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_XMPP__StunTransaction[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      37,   23,   22,   22, 0x05,
      72,   63,   22,   22, 0x05,
     106,  100,   22,   22, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__StunTransaction[] = {
    "XMPP::StunTransaction\0\0transactionId\0"
    "createMessage(QByteArray)\0response\0"
    "finished(XMPP::StunMessage)\0error\0"
    "error(XMPP::StunTransaction::Error)\0"
};

const QMetaObject XMPP::StunTransaction::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__StunTransaction,
      qt_meta_data_XMPP__StunTransaction, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::StunTransaction::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::StunTransaction::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::StunTransaction::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__StunTransaction))
        return static_cast<void*>(const_cast< StunTransaction*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::StunTransaction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: createMessage((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 1: finished((*reinterpret_cast< const XMPP::StunMessage(*)>(_a[1]))); break;
        case 2: error((*reinterpret_cast< XMPP::StunTransaction::Error(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void XMPP::StunTransaction::createMessage(const QByteArray & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void XMPP::StunTransaction::finished(const XMPP::StunMessage & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void XMPP::StunTransaction::error(XMPP::StunTransaction::Error _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_XMPP__StunTransactionPool[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      44,   27,   26,   26, 0x05,
      89,   26,   26,   26, 0x05,
     111,  106,   26,   26, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__StunTransactionPool[] = {
    "XMPP::StunTransactionPool\0\0packet,addr,port\0"
    "outgoingMessage(QByteArray,QHostAddress,int)\0"
    "needAuthParams()\0line\0debugLine(QString)\0"
};

const QMetaObject XMPP::StunTransactionPool::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__StunTransactionPool,
      qt_meta_data_XMPP__StunTransactionPool, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::StunTransactionPool::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::StunTransactionPool::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::StunTransactionPool::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__StunTransactionPool))
        return static_cast<void*>(const_cast< StunTransactionPool*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::StunTransactionPool::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: outgoingMessage((*reinterpret_cast< const QByteArray(*)>(_a[1])),(*reinterpret_cast< const QHostAddress(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3]))); break;
        case 1: needAuthParams(); break;
        case 2: debugLine((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void XMPP::StunTransactionPool::outgoingMessage(const QByteArray & _t1, const QHostAddress & _t2, int _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void XMPP::StunTransactionPool::needAuthParams()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void XMPP::StunTransactionPool::debugLine(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
