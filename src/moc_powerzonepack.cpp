/****************************************************************************
** Meta object code from reading C++ file 'powerzonepack.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "powerzonepack.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'powerzonepack.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_powerzonepack_t {
    QByteArrayData data[12];
    char stringdata0[148];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_powerzonepack_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_powerzonepack_t qt_meta_stringdata_powerzonepack = {
    {
QT_MOC_LITERAL(0, 0, 13), // "powerzonepack"
QT_MOC_LITERAL(1, 14, 14), // "workoutStarted"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 16), // "QList<trainrow>*"
QT_MOC_LITERAL(4, 47, 4), // "list"
QT_MOC_LITERAL(5, 52, 10), // "loginState"
QT_MOC_LITERAL(6, 63, 2), // "ok"
QT_MOC_LITERAL(7, 66, 23), // "search_workout_onfinish"
QT_MOC_LITERAL(8, 90, 7), // "message"
QT_MOC_LITERAL(9, 98, 5), // "error"
QT_MOC_LITERAL(10, 104, 28), // "QAbstractSocket::SocketError"
QT_MOC_LITERAL(11, 133, 14) // "login_onfinish"

    },
    "powerzonepack\0workoutStarted\0\0"
    "QList<trainrow>*\0list\0loginState\0ok\0"
    "search_workout_onfinish\0message\0error\0"
    "QAbstractSocket::SocketError\0"
    "login_onfinish"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_powerzonepack[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       5,    1,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   45,    2, 0x08 /* Private */,
       9,    1,   48,    2, 0x08 /* Private */,
      11,    1,   51,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Bool,    6,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, 0x80000000 | 10,    9,
    QMetaType::Void, QMetaType::QString,    8,

       0        // eod
};

void powerzonepack::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<powerzonepack *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->workoutStarted((*reinterpret_cast< QList<trainrow>*(*)>(_a[1]))); break;
        case 1: _t->loginState((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->search_workout_onfinish((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->error((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 4: _t->login_onfinish((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (powerzonepack::*)(QList<trainrow> * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&powerzonepack::workoutStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (powerzonepack::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&powerzonepack::loginState)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject powerzonepack::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_powerzonepack.data,
    qt_meta_data_powerzonepack,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *powerzonepack::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *powerzonepack::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_powerzonepack.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int powerzonepack::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void powerzonepack::workoutStarted(QList<trainrow> * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void powerzonepack::loginState(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
