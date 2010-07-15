/****************************************************************************
** Meta object code from reading C++ file 'irisnetplugin.h'
**
** Created: Thu Jul 15 15:29:46 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../corelib/irisnetplugin.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'irisnetplugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_XMPP__IrisNetProvider[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_XMPP__IrisNetProvider[] = {
    "XMPP::IrisNetProvider\0"
};

const QMetaObject XMPP::IrisNetProvider::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__IrisNetProvider,
      qt_meta_data_XMPP__IrisNetProvider, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::IrisNetProvider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::IrisNetProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::IrisNetProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__IrisNetProvider))
        return static_cast<void*>(const_cast< IrisNetProvider*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::IrisNetProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_XMPP__NetInterfaceProvider[] = {

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
      28,   27,   27,   27, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__NetInterfaceProvider[] = {
    "XMPP::NetInterfaceProvider\0\0updated()\0"
};

const QMetaObject XMPP::NetInterfaceProvider::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__NetInterfaceProvider,
      qt_meta_data_XMPP__NetInterfaceProvider, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::NetInterfaceProvider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::NetInterfaceProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::NetInterfaceProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__NetInterfaceProvider))
        return static_cast<void*>(const_cast< NetInterfaceProvider*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::NetInterfaceProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updated(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void XMPP::NetInterfaceProvider::updated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_XMPP__NetAvailabilityProvider[] = {

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
      31,   30,   30,   30, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__NetAvailabilityProvider[] = {
    "XMPP::NetAvailabilityProvider\0\0updated()\0"
};

const QMetaObject XMPP::NetAvailabilityProvider::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__NetAvailabilityProvider,
      qt_meta_data_XMPP__NetAvailabilityProvider, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::NetAvailabilityProvider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::NetAvailabilityProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::NetAvailabilityProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__NetAvailabilityProvider))
        return static_cast<void*>(const_cast< NetAvailabilityProvider*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::NetAvailabilityProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: updated(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void XMPP::NetAvailabilityProvider::updated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_XMPP__NameProvider[] = {

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
      31,   20,   19,   19, 0x05,
      86,   81,   19,   19, 0x05,
     139,  131,   19,   19, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__NameProvider[] = {
    "XMPP::NameProvider\0\0id,results\0"
    "resolve_resultsReady(int,QList<XMPP::NameRecord>)\0"
    "id,e\0resolve_error(int,XMPP::NameResolver::Error)\0"
    "id,name\0resolve_useLocal(int,QByteArray)\0"
};

const QMetaObject XMPP::NameProvider::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__NameProvider,
      qt_meta_data_XMPP__NameProvider, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::NameProvider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::NameProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::NameProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__NameProvider))
        return static_cast<void*>(const_cast< NameProvider*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::NameProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: resolve_resultsReady((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QList<XMPP::NameRecord>(*)>(_a[2]))); break;
        case 1: resolve_error((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< XMPP::NameResolver::Error(*)>(_a[2]))); break;
        case 2: resolve_useLocal((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void XMPP::NameProvider::resolve_resultsReady(int _t1, const QList<XMPP::NameRecord> & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void XMPP::NameProvider::resolve_error(int _t1, XMPP::NameResolver::Error _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void XMPP::NameProvider::resolve_useLocal(int _t1, const QByteArray & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
static const uint qt_meta_data_XMPP__ServiceProvider[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       9,       // signalCount

 // signals: signature, parameters, type, tag, flags
      35,   23,   22,   22, 0x05,
      87,   23,   22,   22, 0x05,
     146,  141,   22,   22, 0x05,
     203,  192,   22,   22, 0x05,
     273,  141,   22,   22, 0x05,
     324,  321,   22,   22, 0x05,
     347,  141,   22,   22, 0x05,
     401,  321,   22,   22, 0x05,
     430,  141,   22,   22, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__ServiceProvider[] = {
    "XMPP::ServiceProvider\0\0id,instance\0"
    "browse_instanceAvailable(int,XMPP::ServiceInstance)\0"
    "browse_instanceUnavailable(int,XMPP::ServiceInstance)\0"
    "id,e\0browse_error(int,XMPP::ServiceBrowser::Error)\0"
    "id,results\0"
    "resolve_resultsReady(int,QList<XMPP::ServiceProvider::ResolveResult>)\0"
    "resolve_error(int,XMPP::ServiceResolver::Error)\0"
    "id\0publish_published(int)\0"
    "publish_error(int,XMPP::ServiceLocalPublisher::Error)\0"
    "publish_extra_published(int)\0"
    "publish_extra_error(int,XMPP::ServiceLocalPublisher::Error)\0"
};

const QMetaObject XMPP::ServiceProvider::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_XMPP__ServiceProvider,
      qt_meta_data_XMPP__ServiceProvider, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::ServiceProvider::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::ServiceProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::ServiceProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__ServiceProvider))
        return static_cast<void*>(const_cast< ServiceProvider*>(this));
    return QObject::qt_metacast(_clname);
}

int XMPP::ServiceProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: browse_instanceAvailable((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const XMPP::ServiceInstance(*)>(_a[2]))); break;
        case 1: browse_instanceUnavailable((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const XMPP::ServiceInstance(*)>(_a[2]))); break;
        case 2: browse_error((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< XMPP::ServiceBrowser::Error(*)>(_a[2]))); break;
        case 3: resolve_resultsReady((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< const QList<XMPP::ServiceProvider::ResolveResult>(*)>(_a[2]))); break;
        case 4: resolve_error((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< XMPP::ServiceResolver::Error(*)>(_a[2]))); break;
        case 5: publish_published((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: publish_error((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< XMPP::ServiceLocalPublisher::Error(*)>(_a[2]))); break;
        case 7: publish_extra_published((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: publish_extra_error((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< XMPP::ServiceLocalPublisher::Error(*)>(_a[2]))); break;
        default: ;
        }
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void XMPP::ServiceProvider::browse_instanceAvailable(int _t1, const XMPP::ServiceInstance & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void XMPP::ServiceProvider::browse_instanceUnavailable(int _t1, const XMPP::ServiceInstance & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void XMPP::ServiceProvider::browse_error(int _t1, XMPP::ServiceBrowser::Error _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void XMPP::ServiceProvider::resolve_resultsReady(int _t1, const QList<XMPP::ServiceProvider::ResolveResult> & _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void XMPP::ServiceProvider::resolve_error(int _t1, XMPP::ServiceResolver::Error _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void XMPP::ServiceProvider::publish_published(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void XMPP::ServiceProvider::publish_error(int _t1, XMPP::ServiceLocalPublisher::Error _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void XMPP::ServiceProvider::publish_extra_published(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void XMPP::ServiceProvider::publish_extra_error(int _t1, XMPP::ServiceLocalPublisher::Error _t2)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}
QT_END_MOC_NAMESPACE
