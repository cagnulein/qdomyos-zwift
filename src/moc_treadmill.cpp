/****************************************************************************
** Meta object code from reading C++ file 'treadmill.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "treadmill.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'treadmill.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_treadmill_t {
    QByteArrayData data[23];
    char stringdata0[302];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_treadmill_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_treadmill_t qt_meta_stringdata_treadmill = {
    {
QT_MOC_LITERAL(0, 0, 9), // "treadmill"
QT_MOC_LITERAL(1, 10, 11), // "tapeStarted"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 11), // "changeSpeed"
QT_MOC_LITERAL(4, 35, 5), // "speed"
QT_MOC_LITERAL(5, 41, 17), // "changeInclination"
QT_MOC_LITERAL(6, 59, 5), // "grade"
QT_MOC_LITERAL(7, 65, 10), // "percentage"
QT_MOC_LITERAL(8, 76, 25), // "changeSpeedAndInclination"
QT_MOC_LITERAL(9, 102, 11), // "inclination"
QT_MOC_LITERAL(10, 114, 13), // "cadenceSensor"
QT_MOC_LITERAL(11, 128, 7), // "uint8_t"
QT_MOC_LITERAL(12, 136, 7), // "cadence"
QT_MOC_LITERAL(13, 144, 11), // "powerSensor"
QT_MOC_LITERAL(14, 156, 8), // "uint16_t"
QT_MOC_LITERAL(15, 165, 5), // "power"
QT_MOC_LITERAL(16, 171, 11), // "speedSensor"
QT_MOC_LITERAL(17, 183, 31), // "instantaneousStrideLengthSensor"
QT_MOC_LITERAL(18, 215, 6), // "length"
QT_MOC_LITERAL(19, 222, 19), // "groundContactSensor"
QT_MOC_LITERAL(20, 242, 13), // "groundContact"
QT_MOC_LITERAL(21, 256, 25), // "verticalOscillationSensor"
QT_MOC_LITERAL(22, 282, 19) // "verticalOscillation"

    },
    "treadmill\0tapeStarted\0\0changeSpeed\0"
    "speed\0changeInclination\0grade\0percentage\0"
    "changeSpeedAndInclination\0inclination\0"
    "cadenceSensor\0uint8_t\0cadence\0powerSensor\0"
    "uint16_t\0power\0speedSensor\0"
    "instantaneousStrideLengthSensor\0length\0"
    "groundContactSensor\0groundContact\0"
    "verticalOscillationSensor\0verticalOscillation"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_treadmill[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   65,    2, 0x0a /* Public */,
       5,    2,   68,    2, 0x0a /* Public */,
       8,    2,   73,    2, 0x0a /* Public */,
      10,    1,   78,    2, 0x0a /* Public */,
      13,    1,   81,    2, 0x0a /* Public */,
      16,    1,   84,    2, 0x0a /* Public */,
      17,    1,   87,    2, 0x0a /* Public */,
      19,    1,   90,    2, 0x0a /* Public */,
      21,    1,   93,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    4,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    6,    7,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    4,    9,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void, 0x80000000 | 14,   15,
    QMetaType::Void, QMetaType::Double,    4,
    QMetaType::Void, QMetaType::Double,   18,
    QMetaType::Void, QMetaType::Double,   20,
    QMetaType::Void, QMetaType::Double,   22,

       0        // eod
};

void treadmill::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<treadmill *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->tapeStarted(); break;
        case 1: _t->changeSpeed((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->changeInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 3: _t->changeSpeedAndInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 4: _t->cadenceSensor((*reinterpret_cast< uint8_t(*)>(_a[1]))); break;
        case 5: _t->powerSensor((*reinterpret_cast< uint16_t(*)>(_a[1]))); break;
        case 6: _t->speedSensor((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->instantaneousStrideLengthSensor((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 8: _t->groundContactSensor((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: _t->verticalOscillationSensor((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (treadmill::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&treadmill::tapeStarted)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject treadmill::staticMetaObject = { {
    QMetaObject::SuperData::link<bluetoothdevice::staticMetaObject>(),
    qt_meta_stringdata_treadmill.data,
    qt_meta_data_treadmill,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *treadmill::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *treadmill::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_treadmill.stringdata0))
        return static_cast<void*>(this);
    return bluetoothdevice::qt_metacast(_clname);
}

int treadmill::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void treadmill::tapeStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
