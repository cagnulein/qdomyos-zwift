/****************************************************************************
** Meta object code from reading C++ file 'homefitnessbuddy.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "homefitnessbuddy.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'homefitnessbuddy.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_homefitnessbuddy_t {
    QByteArrayData data[16];
    char stringdata0[203];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_homefitnessbuddy_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_homefitnessbuddy_t qt_meta_stringdata_homefitnessbuddy = {
    {
QT_MOC_LITERAL(0, 0, 16), // "homefitnessbuddy"
QT_MOC_LITERAL(1, 17, 14), // "workoutStarted"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 16), // "QList<trainrow>*"
QT_MOC_LITERAL(4, 50, 4), // "list"
QT_MOC_LITERAL(5, 55, 10), // "loginState"
QT_MOC_LITERAL(6, 66, 2), // "ok"
QT_MOC_LITERAL(7, 69, 14), // "login_onfinish"
QT_MOC_LITERAL(8, 84, 14), // "QNetworkReply*"
QT_MOC_LITERAL(9, 99, 5), // "reply"
QT_MOC_LITERAL(10, 105, 23), // "search_workout_onfinish"
QT_MOC_LITERAL(11, 129, 22), // "search_detail_onfinish"
QT_MOC_LITERAL(12, 152, 5), // "error"
QT_MOC_LITERAL(13, 158, 27), // "QNetworkReply::NetworkError"
QT_MOC_LITERAL(14, 186, 4), // "code"
QT_MOC_LITERAL(15, 191, 11) // "startEngine"

    },
    "homefitnessbuddy\0workoutStarted\0\0"
    "QList<trainrow>*\0list\0loginState\0ok\0"
    "login_onfinish\0QNetworkReply*\0reply\0"
    "search_workout_onfinish\0search_detail_onfinish\0"
    "error\0QNetworkReply::NetworkError\0"
    "code\0startEngine"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_homefitnessbuddy[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       5,    1,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   55,    2, 0x08 /* Private */,
      10,    1,   58,    2, 0x08 /* Private */,
      11,    1,   61,    2, 0x08 /* Private */,
      12,    1,   64,    2, 0x08 /* Private */,
      15,    0,   67,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Bool,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 8,    9,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void,

       0        // eod
};

void homefitnessbuddy::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<homefitnessbuddy *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->workoutStarted((*reinterpret_cast< QList<trainrow>*(*)>(_a[1]))); break;
        case 1: _t->loginState((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->login_onfinish((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 3: _t->search_workout_onfinish((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 4: _t->search_detail_onfinish((*reinterpret_cast< QNetworkReply*(*)>(_a[1]))); break;
        case 5: _t->error((*reinterpret_cast< QNetworkReply::NetworkError(*)>(_a[1]))); break;
        case 6: _t->startEngine(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply* >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply* >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply* >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QNetworkReply::NetworkError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (homefitnessbuddy::*)(QList<trainrow> * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&homefitnessbuddy::workoutStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (homefitnessbuddy::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&homefitnessbuddy::loginState)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject homefitnessbuddy::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_homefitnessbuddy.data,
    qt_meta_data_homefitnessbuddy,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *homefitnessbuddy::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *homefitnessbuddy::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_homefitnessbuddy.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int homefitnessbuddy::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void homefitnessbuddy::workoutStarted(QList<trainrow> * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void homefitnessbuddy::loginState(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
