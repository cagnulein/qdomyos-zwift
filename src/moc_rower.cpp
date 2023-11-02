/****************************************************************************
** Meta object code from reading C++ file 'rower.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "rower.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'rower.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_rower_t {
    QByteArrayData data[24];
    char stringdata0[258];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_rower_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_rower_t qt_meta_stringdata_rower = {
    {
QT_MOC_LITERAL(0, 0, 5), // "rower"
QT_MOC_LITERAL(1, 6, 11), // "bikeStarted"
QT_MOC_LITERAL(2, 18, 0), // ""
QT_MOC_LITERAL(3, 19, 17), // "resistanceChanged"
QT_MOC_LITERAL(4, 37, 12), // "resistance_t"
QT_MOC_LITERAL(5, 50, 10), // "resistance"
QT_MOC_LITERAL(6, 61, 14), // "resistanceRead"
QT_MOC_LITERAL(7, 76, 16), // "changeResistance"
QT_MOC_LITERAL(8, 93, 3), // "res"
QT_MOC_LITERAL(9, 97, 13), // "changeCadence"
QT_MOC_LITERAL(10, 111, 7), // "int16_t"
QT_MOC_LITERAL(11, 119, 3), // "cad"
QT_MOC_LITERAL(12, 123, 11), // "changePower"
QT_MOC_LITERAL(13, 135, 7), // "int32_t"
QT_MOC_LITERAL(14, 143, 5), // "power"
QT_MOC_LITERAL(15, 149, 32), // "changeRequestedPelotonResistance"
QT_MOC_LITERAL(16, 182, 6), // "int8_t"
QT_MOC_LITERAL(17, 189, 13), // "cadenceSensor"
QT_MOC_LITERAL(18, 203, 7), // "uint8_t"
QT_MOC_LITERAL(19, 211, 7), // "cadence"
QT_MOC_LITERAL(20, 219, 11), // "powerSensor"
QT_MOC_LITERAL(21, 231, 8), // "uint16_t"
QT_MOC_LITERAL(22, 240, 11), // "changeSpeed"
QT_MOC_LITERAL(23, 252, 5) // "speed"

    },
    "rower\0bikeStarted\0\0resistanceChanged\0"
    "resistance_t\0resistance\0resistanceRead\0"
    "changeResistance\0res\0changeCadence\0"
    "int16_t\0cad\0changePower\0int32_t\0power\0"
    "changeRequestedPelotonResistance\0"
    "int8_t\0cadenceSensor\0uint8_t\0cadence\0"
    "powerSensor\0uint16_t\0changeSpeed\0speed"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_rower[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x06 /* Public */,
       3,    1,   65,    2, 0x06 /* Public */,
       6,    1,   68,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   71,    2, 0x0a /* Public */,
       9,    1,   74,    2, 0x0a /* Public */,
      12,    1,   77,    2, 0x0a /* Public */,
      15,    1,   80,    2, 0x0a /* Public */,
      17,    1,   83,    2, 0x0a /* Public */,
      20,    1,   86,    2, 0x0a /* Public */,
      22,    1,   89,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4,    8,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void, 0x80000000 | 16,    5,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void, 0x80000000 | 21,   14,
    QMetaType::Void, QMetaType::Double,   23,

       0        // eod
};

void rower::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<rower *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->bikeStarted(); break;
        case 1: _t->resistanceChanged((*reinterpret_cast< resistance_t(*)>(_a[1]))); break;
        case 2: _t->resistanceRead((*reinterpret_cast< resistance_t(*)>(_a[1]))); break;
        case 3: _t->changeResistance((*reinterpret_cast< resistance_t(*)>(_a[1]))); break;
        case 4: _t->changeCadence((*reinterpret_cast< int16_t(*)>(_a[1]))); break;
        case 5: _t->changePower((*reinterpret_cast< int32_t(*)>(_a[1]))); break;
        case 6: _t->changeRequestedPelotonResistance((*reinterpret_cast< int8_t(*)>(_a[1]))); break;
        case 7: _t->cadenceSensor((*reinterpret_cast< uint8_t(*)>(_a[1]))); break;
        case 8: _t->powerSensor((*reinterpret_cast< uint16_t(*)>(_a[1]))); break;
        case 9: _t->changeSpeed((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (rower::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&rower::bikeStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (rower::*)(resistance_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&rower::resistanceChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (rower::*)(resistance_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&rower::resistanceRead)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject rower::staticMetaObject = { {
    QMetaObject::SuperData::link<bluetoothdevice::staticMetaObject>(),
    qt_meta_stringdata_rower.data,
    qt_meta_data_rower,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *rower::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *rower::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_rower.stringdata0))
        return static_cast<void*>(this);
    return bluetoothdevice::qt_metacast(_clname);
}

int rower::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = bluetoothdevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void rower::bikeStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void rower::resistanceChanged(resistance_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void rower::resistanceRead(resistance_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
