/****************************************************************************
** Meta object code from reading C++ file 'fakeelliptical.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "fakeelliptical.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'fakeelliptical.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_fakeelliptical_t {
    QByteArrayData data[13];
    char stringdata0[168];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_fakeelliptical_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_fakeelliptical_t qt_meta_stringdata_fakeelliptical = {
    {
QT_MOC_LITERAL(0, 0, 14), // "fakeelliptical"
QT_MOC_LITERAL(1, 15, 12), // "disconnected"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 5), // "debug"
QT_MOC_LITERAL(4, 35, 6), // "string"
QT_MOC_LITERAL(5, 42, 26), // "changeInclinationRequested"
QT_MOC_LITERAL(6, 69, 5), // "grade"
QT_MOC_LITERAL(7, 75, 10), // "percentage"
QT_MOC_LITERAL(8, 86, 6), // "update"
QT_MOC_LITERAL(9, 93, 25), // "ftmsCharacteristicChanged"
QT_MOC_LITERAL(10, 119, 24), // "QLowEnergyCharacteristic"
QT_MOC_LITERAL(11, 144, 14), // "characteristic"
QT_MOC_LITERAL(12, 159, 8) // "newValue"

    },
    "fakeelliptical\0disconnected\0\0debug\0"
    "string\0changeInclinationRequested\0"
    "grade\0percentage\0update\0"
    "ftmsCharacteristicChanged\0"
    "QLowEnergyCharacteristic\0characteristic\0"
    "newValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_fakeelliptical[] = {

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
       1,    0,   39,    2, 0x06 /* Public */,
       3,    1,   40,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    2,   43,    2, 0x08 /* Private */,
       8,    0,   48,    2, 0x08 /* Private */,
       9,    2,   49,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    6,    7,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10, QMetaType::QByteArray,   11,   12,

       0        // eod
};

void fakeelliptical::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<fakeelliptical *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->changeInclinationRequested((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 3: _t->update(); break;
        case 4: _t->ftmsCharacteristicChanged((*reinterpret_cast< const QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyCharacteristic >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (fakeelliptical::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&fakeelliptical::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (fakeelliptical::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&fakeelliptical::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject fakeelliptical::staticMetaObject = { {
    QMetaObject::SuperData::link<elliptical::staticMetaObject>(),
    qt_meta_stringdata_fakeelliptical.data,
    qt_meta_data_fakeelliptical,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *fakeelliptical::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *fakeelliptical::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_fakeelliptical.stringdata0))
        return static_cast<void*>(this);
    return elliptical::qt_metacast(_clname);
}

int fakeelliptical::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = elliptical::qt_metacall(_c, _id, _a);
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
void fakeelliptical::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void fakeelliptical::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
