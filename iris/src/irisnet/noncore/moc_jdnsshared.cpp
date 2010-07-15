/****************************************************************************
** Meta object code from reading C++ file 'jdnsshared.h'
**
** Created: Thu Jul 15 18:46:35 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../corelib/jdnsshared.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'jdnsshared.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_JDnsSharedDebug[] = {

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
      17,   16,   16,   16, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_JDnsSharedDebug[] = {
    "JDnsSharedDebug\0\0readyRead()\0"
};

const QMetaObject JDnsSharedDebug::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_JDnsSharedDebug,
      qt_meta_data_JDnsSharedDebug, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &JDnsSharedDebug::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *JDnsSharedDebug::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *JDnsSharedDebug::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_JDnsSharedDebug))
        return static_cast<void*>(const_cast< JDnsSharedDebug*>(this));
    return QObject::qt_metacast(_clname);
}

int JDnsSharedDebug::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: readyRead(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void JDnsSharedDebug::readyRead()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_JDnsSharedRequest[] = {

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
      19,   18,   18,   18, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_JDnsSharedRequest[] = {
    "JDnsSharedRequest\0\0resultsReady()\0"
};

const QMetaObject JDnsSharedRequest::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_JDnsSharedRequest,
      qt_meta_data_JDnsSharedRequest, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &JDnsSharedRequest::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *JDnsSharedRequest::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *JDnsSharedRequest::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_JDnsSharedRequest))
        return static_cast<void*>(const_cast< JDnsSharedRequest*>(this));
    return QObject::qt_metacast(_clname);
}

int JDnsSharedRequest::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: resultsReady(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void JDnsSharedRequest::resultsReady()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
static const uint qt_meta_data_JDnsShared[] = {

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
      12,   11,   11,   11, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_JDnsShared[] = {
    "JDnsShared\0\0shutdownFinished()\0"
};

const QMetaObject JDnsShared::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_JDnsShared,
      qt_meta_data_JDnsShared, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &JDnsShared::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *JDnsShared::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *JDnsShared::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_JDnsShared))
        return static_cast<void*>(const_cast< JDnsShared*>(this));
    return QObject::qt_metacast(_clname);
}

int JDnsShared::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: shutdownFinished(); break;
        default: ;
        }
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void JDnsShared::shutdownFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}
QT_END_MOC_NAMESPACE
