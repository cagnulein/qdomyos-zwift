/****************************************************************************
** Meta object code from reading C++ file 'hostname_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "qmdnsengine/src/src/hostname_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'hostname_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QMdnsEngine__HostnamePrivate_t {
    QByteArrayData data[7];
    char stringdata0[107];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QMdnsEngine__HostnamePrivate_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QMdnsEngine__HostnamePrivate_t qt_meta_stringdata_QMdnsEngine__HostnamePrivate = {
    {
QT_MOC_LITERAL(0, 0, 28), // "QMdnsEngine::HostnamePrivate"
QT_MOC_LITERAL(1, 29, 17), // "onMessageReceived"
QT_MOC_LITERAL(2, 47, 0), // ""
QT_MOC_LITERAL(3, 48, 7), // "Message"
QT_MOC_LITERAL(4, 56, 7), // "message"
QT_MOC_LITERAL(5, 64, 21), // "onRegistrationTimeout"
QT_MOC_LITERAL(6, 86, 20) // "onRebroadcastTimeout"

    },
    "QMdnsEngine::HostnamePrivate\0"
    "onMessageReceived\0\0Message\0message\0"
    "onRegistrationTimeout\0onRebroadcastTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QMdnsEngine__HostnamePrivate[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x08 /* Private */,
       5,    0,   32,    2, 0x08 /* Private */,
       6,    0,   33,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void QMdnsEngine::HostnamePrivate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<HostnamePrivate *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onMessageReceived((*reinterpret_cast< const Message(*)>(_a[1]))); break;
        case 1: _t->onRegistrationTimeout(); break;
        case 2: _t->onRebroadcastTimeout(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject QMdnsEngine::HostnamePrivate::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_QMdnsEngine__HostnamePrivate.data,
    qt_meta_data_QMdnsEngine__HostnamePrivate,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QMdnsEngine::HostnamePrivate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QMdnsEngine::HostnamePrivate::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QMdnsEngine__HostnamePrivate.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int QMdnsEngine::HostnamePrivate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
