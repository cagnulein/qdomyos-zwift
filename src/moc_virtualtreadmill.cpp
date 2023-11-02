/****************************************************************************
** Meta object code from reading C++ file 'virtualtreadmill.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "virtualtreadmill.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'virtualtreadmill.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_virtualtreadmill_t {
    QByteArrayData data[16];
    char stringdata0[229];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_virtualtreadmill_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_virtualtreadmill_t qt_meta_stringdata_virtualtreadmill = {
    {
QT_MOC_LITERAL(0, 0, 16), // "virtualtreadmill"
QT_MOC_LITERAL(1, 17, 5), // "debug"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 6), // "string"
QT_MOC_LITERAL(4, 31, 17), // "changeInclination"
QT_MOC_LITERAL(5, 49, 5), // "grade"
QT_MOC_LITERAL(6, 55, 10), // "percentage"
QT_MOC_LITERAL(7, 66, 25), // "ftmsCharacteristicChanged"
QT_MOC_LITERAL(8, 92, 24), // "QLowEnergyCharacteristic"
QT_MOC_LITERAL(9, 117, 14), // "characteristic"
QT_MOC_LITERAL(10, 132, 8), // "newValue"
QT_MOC_LITERAL(11, 141, 21), // "characteristicChanged"
QT_MOC_LITERAL(12, 163, 17), // "treadmillProvider"
QT_MOC_LITERAL(13, 181, 9), // "reconnect"
QT_MOC_LITERAL(14, 191, 12), // "slopeChanged"
QT_MOC_LITERAL(15, 204, 24) // "dirconChangedInclination"

    },
    "virtualtreadmill\0debug\0\0string\0"
    "changeInclination\0grade\0percentage\0"
    "ftmsCharacteristicChanged\0"
    "QLowEnergyCharacteristic\0characteristic\0"
    "newValue\0characteristicChanged\0"
    "treadmillProvider\0reconnect\0slopeChanged\0"
    "dirconChangedInclination"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_virtualtreadmill[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x06 /* Public */,
       4,    2,   57,    2, 0x06 /* Public */,
       7,    2,   62,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    2,   67,    2, 0x08 /* Private */,
      12,    0,   72,    2, 0x08 /* Private */,
      13,    0,   73,    2, 0x08 /* Private */,
      14,    0,   74,    2, 0x08 /* Private */,
      15,    2,   75,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    5,    6,
    QMetaType::Void, 0x80000000 | 8, QMetaType::QByteArray,    9,   10,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 8, QMetaType::QByteArray,    9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    5,    6,

       0        // eod
};

void virtualtreadmill::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<virtualtreadmill *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->changeInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 2: _t->ftmsCharacteristicChanged((*reinterpret_cast< const QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 3: _t->characteristicChanged((*reinterpret_cast< const QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 4: _t->treadmillProvider(); break;
        case 5: _t->reconnect(); break;
        case 6: _t->slopeChanged(); break;
        case 7: _t->dirconChangedInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyCharacteristic >(); break;
            }
            break;
        case 3:
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
            using _t = void (virtualtreadmill::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&virtualtreadmill::debug)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (virtualtreadmill::*)(double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&virtualtreadmill::changeInclination)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (virtualtreadmill::*)(const QLowEnergyCharacteristic & , const QByteArray & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&virtualtreadmill::ftmsCharacteristicChanged)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject virtualtreadmill::staticMetaObject = { {
    QMetaObject::SuperData::link<virtualdevice::staticMetaObject>(),
    qt_meta_stringdata_virtualtreadmill.data,
    qt_meta_data_virtualtreadmill,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *virtualtreadmill::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *virtualtreadmill::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_virtualtreadmill.stringdata0))
        return static_cast<void*>(this);
    return virtualdevice::qt_metacast(_clname);
}

int virtualtreadmill::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = virtualdevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void virtualtreadmill::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void virtualtreadmill::changeInclination(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void virtualtreadmill::ftmsCharacteristicChanged(const QLowEnergyCharacteristic & _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
