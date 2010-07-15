/****************************************************************************
** Meta object code from reading C++ file 'xmpp_clientstream.h'
**
** Created: Thu Jul 15 18:46:45 2010
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../xmpp-core/xmpp_clientstream.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'xmpp_clientstream.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_XMPP__ClientStream[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: signature, parameters, type, tag, flags
      20,   19,   19,   19, 0x05,
      32,   19,   19,   19, 0x05,
      76,   60,   19,   19, 0x05,
     107,   19,   19,   19, 0x05,
     123,   19,   19,   19, 0x05,
     138,  136,   19,   19, 0x05,
     159,  136,   19,   19, 0x05,

 // slots: signature, parameters, type, tag, flags
     180,   19,   19,   19, 0x0a,
     203,   19,   19,   19, 0x08,
     218,   19,   19,   19, 0x08,
     229,   19,   19,   19, 0x08,
     251,   19,   19,   19, 0x08,
     277,   19,   19,   19, 0x08,
     291,   19,   19,   19, 0x08,
     306,   19,   19,   19, 0x08,
     327,   19,   19,   19, 0x08,
     346,   19,   19,   19, 0x08,
     361,   19,   19,   19, 0x08,
     377,  375,   19,   19, 0x08,
     424,  415,   19,   19, 0x08,
     450,   19,   19,   19, 0x08,
     498,  485,   19,   19, 0x08,
     530,   19,   19,   19, 0x08,
     551,   19,   19,   19, 0x08,
     564,   19,   19,   19, 0x08,
     573,   19,   19,   19, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_XMPP__ClientStream[] = {
    "XMPP::ClientStream\0\0connected()\0"
    "securityLayerActivated(int)\0user,pass,realm\0"
    "needAuthParams(bool,bool,bool)\0"
    "authenticated()\0warning(int)\0s\0"
    "incomingXml(QString)\0outgoingXml(QString)\0"
    "continueAfterWarning()\0cr_connected()\0"
    "cr_error()\0bs_connectionClosed()\0"
    "bs_delayedCloseFinished()\0bs_error(int)\0"
    "ss_readyRead()\0ss_bytesWritten(int)\0"
    "ss_tlsHandshaken()\0ss_tlsClosed()\0"
    "ss_error(int)\0,\0sasl_clientFirstStep(bool,QByteArray)\0"
    "stepData\0sasl_nextStep(QByteArray)\0"
    "sasl_needParams(QCA::SASL::Params)\0"
    "user,authzid\0sasl_authCheck(QString,QString)\0"
    "sasl_authenticated()\0sasl_error()\0"
    "doNoop()\0doReadyRead()\0"
};

const QMetaObject XMPP::ClientStream::staticMetaObject = {
    { &Stream::staticMetaObject, qt_meta_stringdata_XMPP__ClientStream,
      qt_meta_data_XMPP__ClientStream, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &XMPP::ClientStream::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *XMPP::ClientStream::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *XMPP::ClientStream::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_XMPP__ClientStream))
        return static_cast<void*>(const_cast< ClientStream*>(this));
    return Stream::qt_metacast(_clname);
}

int XMPP::ClientStream::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = Stream::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: connected(); break;
        case 1: securityLayerActivated((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: needAuthParams((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 3: authenticated(); break;
        case 4: warning((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: incomingXml((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 6: outgoingXml((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 7: continueAfterWarning(); break;
        case 8: cr_connected(); break;
        case 9: cr_error(); break;
        case 10: bs_connectionClosed(); break;
        case 11: bs_delayedCloseFinished(); break;
        case 12: bs_error((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: ss_readyRead(); break;
        case 14: ss_bytesWritten((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: ss_tlsHandshaken(); break;
        case 16: ss_tlsClosed(); break;
        case 17: ss_error((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 18: sasl_clientFirstStep((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 19: sasl_nextStep((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 20: sasl_needParams((*reinterpret_cast< const QCA::SASL::Params(*)>(_a[1]))); break;
        case 21: sasl_authCheck((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 22: sasl_authenticated(); break;
        case 23: sasl_error(); break;
        case 24: doNoop(); break;
        case 25: doReadyRead(); break;
        default: ;
        }
        _id -= 26;
    }
    return _id;
}

// SIGNAL 0
void XMPP::ClientStream::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void XMPP::ClientStream::securityLayerActivated(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void XMPP::ClientStream::needAuthParams(bool _t1, bool _t2, bool _t3)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void XMPP::ClientStream::authenticated()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void XMPP::ClientStream::warning(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void XMPP::ClientStream::incomingXml(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void XMPP::ClientStream::outgoingXml(const QString & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_END_MOC_NAMESPACE
