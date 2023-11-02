/****************************************************************************
** Meta object code from reading C++ file 'bike.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "bike.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'bike.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_bike_t {
    QByteArrayData data[29];
    char stringdata0[349];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_bike_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_bike_t qt_meta_stringdata_bike = {
    {
QT_MOC_LITERAL(0, 0, 4), // "bike"
QT_MOC_LITERAL(1, 5, 11), // "bikeStarted"
QT_MOC_LITERAL(2, 17, 0), // ""
QT_MOC_LITERAL(3, 18, 17), // "resistanceChanged"
QT_MOC_LITERAL(4, 36, 12), // "resistance_t"
QT_MOC_LITERAL(5, 49, 10), // "resistance"
QT_MOC_LITERAL(6, 60, 14), // "resistanceRead"
QT_MOC_LITERAL(7, 75, 20), // "steeringAngleChanged"
QT_MOC_LITERAL(8, 96, 5), // "angle"
QT_MOC_LITERAL(9, 102, 16), // "changeResistance"
QT_MOC_LITERAL(10, 119, 3), // "res"
QT_MOC_LITERAL(11, 123, 13), // "changeCadence"
QT_MOC_LITERAL(12, 137, 7), // "int16_t"
QT_MOC_LITERAL(13, 145, 3), // "cad"
QT_MOC_LITERAL(14, 149, 11), // "changePower"
QT_MOC_LITERAL(15, 161, 7), // "int32_t"
QT_MOC_LITERAL(16, 169, 5), // "power"
QT_MOC_LITERAL(17, 175, 32), // "changeRequestedPelotonResistance"
QT_MOC_LITERAL(18, 208, 6), // "int8_t"
QT_MOC_LITERAL(19, 215, 13), // "cadenceSensor"
QT_MOC_LITERAL(20, 229, 7), // "uint8_t"
QT_MOC_LITERAL(21, 237, 7), // "cadence"
QT_MOC_LITERAL(22, 245, 11), // "powerSensor"
QT_MOC_LITERAL(23, 257, 8), // "uint16_t"
QT_MOC_LITERAL(24, 266, 17), // "changeInclination"
QT_MOC_LITERAL(25, 284, 5), // "grade"
QT_MOC_LITERAL(26, 290, 10), // "percentage"
QT_MOC_LITERAL(27, 301, 19), // "changeSteeringAngle"
QT_MOC_LITERAL(28, 321, 27) // "resistanceFromFTMSAccessory"

    },
    "bike\0bikeStarted\0\0resistanceChanged\0"
    "resistance_t\0resistance\0resistanceRead\0"
    "steeringAngleChanged\0angle\0changeResistance\0"
    "res\0changeCadence\0int16_t\0cad\0changePower\0"
    "int32_t\0power\0changeRequestedPelotonResistance\0"
    "int8_t\0cadenceSensor\0uint8_t\0cadence\0"
    "powerSensor\0uint16_t\0changeInclination\0"
    "grade\0percentage\0changeSteeringAngle\0"
    "resistanceFromFTMSAccessory"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_bike[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x06 /* Public */,
       3,    1,   80,    2, 0x06 /* Public */,
       6,    1,   83,    2, 0x06 /* Public */,
       7,    1,   86,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    1,   89,    2, 0x0a /* Public */,
      11,    1,   92,    2, 0x0a /* Public */,
      14,    1,   95,    2, 0x0a /* Public */,
      17,    1,   98,    2, 0x0a /* Public */,
      19,    1,  101,    2, 0x0a /* Public */,
      22,    1,  104,    2, 0x0a /* Public */,
      24,    2,  107,    2, 0x0a /* Public */,
      27,    1,  112,    2, 0x0a /* Public */,
      28,    1,  115,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::Double,    8,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4,   10,
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, 0x80000000 | 18,    5,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void, 0x80000000 | 23,   16,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   25,   26,
    QMetaType::Void, QMetaType::Double,    8,
    QMetaType::Void, 0x80000000 | 4,   10,

       0        // eod
};

void bike::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<bike *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->bikeStarted(); break;
        case 1: _t->resistanceChanged((*reinterpret_cast< resistance_t(*)>(_a[1]))); break;
        case 2: _t->resistanceRead((*reinterpret_cast< resistance_t(*)>(_a[1]))); break;
        case 3: _t->steeringAngleChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: _t->changeResistance((*reinterpret_cast< resistance_t(*)>(_a[1]))); break;
        case 5: _t->changeCadence((*reinterpret_cast< int16_t(*)>(_a[1]))); break;
        case 6: _t->changePower((*reinterpret_cast< int32_t(*)>(_a[1]))); break;
        case 7: _t->changeRequestedPelotonResistance((*reinterpret_cast< int8_t(*)>(_a[1]))); break;
        case 8: _t->cadenceSensor((*reinterpret_cast< uint8_t(*)>(_a[1]))); break;
        case 9: _t->powerSensor((*reinterpret_cast< uint16_t(*)>(_a[1]))); break;
        case 10: _t->changeInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 11: _t->changeSteeringAngle((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 12: _t->resistanceFromFTMSAccessory((*reinterpret_cast< resistance_t(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (bike::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bike::bikeStarted)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (bike::*)(resistance_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bike::resistanceChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (bike::*)(resistance_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bike::resistanceRead)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (bike::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bike::steeringAngleChanged)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject bike::staticMetaObject = { {
    QMetaObject::SuperData::link<bluetoothdevice::staticMetaObject>(),
    qt_meta_stringdata_bike.data,
    qt_meta_data_bike,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *bike::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *bike::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_bike.stringdata0))
        return static_cast<void*>(this);
    return bluetoothdevice::qt_metacast(_clname);
}

int bike::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = bluetoothdevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void bike::bikeStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void bike::resistanceChanged(resistance_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void bike::resistanceRead(resistance_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void bike::steeringAngleChanged(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
