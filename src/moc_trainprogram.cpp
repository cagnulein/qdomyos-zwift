/****************************************************************************
** Meta object code from reading C++ file 'trainprogram.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "trainprogram.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'trainprogram.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_trainprogram_t {
    QByteArrayData data[40];
    char stringdata0[510];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_trainprogram_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_trainprogram_t qt_meta_stringdata_trainprogram = {
    {
QT_MOC_LITERAL(0, 0, 12), // "trainprogram"
QT_MOC_LITERAL(1, 13, 5), // "start"
QT_MOC_LITERAL(2, 19, 0), // ""
QT_MOC_LITERAL(3, 20, 4), // "stop"
QT_MOC_LITERAL(4, 25, 6), // "paused"
QT_MOC_LITERAL(5, 32, 3), // "lap"
QT_MOC_LITERAL(6, 36, 11), // "changeSpeed"
QT_MOC_LITERAL(7, 48, 5), // "speed"
QT_MOC_LITERAL(8, 54, 14), // "changeFanSpeed"
QT_MOC_LITERAL(9, 69, 7), // "uint8_t"
QT_MOC_LITERAL(10, 77, 17), // "changeInclination"
QT_MOC_LITERAL(11, 95, 5), // "grade"
QT_MOC_LITERAL(12, 101, 11), // "inclination"
QT_MOC_LITERAL(13, 113, 30), // "changeNextInclination300Meters"
QT_MOC_LITERAL(14, 144, 26), // "QList<MetersByInclination>"
QT_MOC_LITERAL(15, 171, 16), // "changeResistance"
QT_MOC_LITERAL(16, 188, 12), // "resistance_t"
QT_MOC_LITERAL(17, 201, 10), // "resistance"
QT_MOC_LITERAL(18, 212, 32), // "changeRequestedPelotonResistance"
QT_MOC_LITERAL(19, 245, 6), // "int8_t"
QT_MOC_LITERAL(20, 252, 13), // "changeCadence"
QT_MOC_LITERAL(21, 266, 7), // "int16_t"
QT_MOC_LITERAL(22, 274, 7), // "cadence"
QT_MOC_LITERAL(23, 282, 11), // "changePower"
QT_MOC_LITERAL(24, 294, 7), // "int32_t"
QT_MOC_LITERAL(25, 302, 5), // "power"
QT_MOC_LITERAL(26, 308, 25), // "changeSpeedAndInclination"
QT_MOC_LITERAL(27, 334, 17), // "changeGeoPosition"
QT_MOC_LITERAL(28, 352, 14), // "QGeoCoordinate"
QT_MOC_LITERAL(29, 367, 1), // "p"
QT_MOC_LITERAL(30, 369, 7), // "azimuth"
QT_MOC_LITERAL(31, 377, 23), // "avgAzimuthNext300Meters"
QT_MOC_LITERAL(32, 401, 15), // "changeTimestamp"
QT_MOC_LITERAL(33, 417, 6), // "source"
QT_MOC_LITERAL(34, 424, 6), // "actual"
QT_MOC_LITERAL(35, 431, 12), // "toastRequest"
QT_MOC_LITERAL(36, 444, 7), // "message"
QT_MOC_LITERAL(37, 452, 13), // "onTapeStarted"
QT_MOC_LITERAL(38, 466, 9), // "scheduler"
QT_MOC_LITERAL(39, 476, 33) // "pelotonOCRprocessPendingDatag..."

    },
    "trainprogram\0start\0\0stop\0paused\0lap\0"
    "changeSpeed\0speed\0changeFanSpeed\0"
    "uint8_t\0changeInclination\0grade\0"
    "inclination\0changeNextInclination300Meters\0"
    "QList<MetersByInclination>\0changeResistance\0"
    "resistance_t\0resistance\0"
    "changeRequestedPelotonResistance\0"
    "int8_t\0changeCadence\0int16_t\0cadence\0"
    "changePower\0int32_t\0power\0"
    "changeSpeedAndInclination\0changeGeoPosition\0"
    "QGeoCoordinate\0p\0azimuth\0"
    "avgAzimuthNext300Meters\0changeTimestamp\0"
    "source\0actual\0toastRequest\0message\0"
    "onTapeStarted\0scheduler\0"
    "pelotonOCRprocessPendingDatagrams"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_trainprogram[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      15,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  104,    2, 0x06 /* Public */,
       3,    1,  105,    2, 0x06 /* Public */,
       5,    0,  108,    2, 0x06 /* Public */,
       6,    1,  109,    2, 0x06 /* Public */,
       8,    1,  112,    2, 0x06 /* Public */,
      10,    2,  115,    2, 0x06 /* Public */,
      13,    1,  120,    2, 0x06 /* Public */,
      15,    1,  123,    2, 0x06 /* Public */,
      18,    1,  126,    2, 0x06 /* Public */,
      20,    1,  129,    2, 0x06 /* Public */,
      23,    1,  132,    2, 0x06 /* Public */,
      26,    2,  135,    2, 0x06 /* Public */,
      27,    3,  140,    2, 0x06 /* Public */,
      32,    2,  147,    2, 0x06 /* Public */,
      35,    1,  152,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      37,    0,  155,    2, 0x0a /* Public */,
      38,    0,  156,    2, 0x0a /* Public */,
      39,    0,  157,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    7,
    QMetaType::Bool, 0x80000000 | 9,    7,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   11,   12,
    QMetaType::Void, 0x80000000 | 14,    2,
    QMetaType::Void, 0x80000000 | 16,   17,
    QMetaType::Void, 0x80000000 | 19,   17,
    QMetaType::Void, 0x80000000 | 21,   22,
    QMetaType::Void, 0x80000000 | 24,   25,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    7,   12,
    QMetaType::Void, 0x80000000 | 28, QMetaType::Double, QMetaType::Double,   29,   30,   31,
    QMetaType::Void, QMetaType::QTime, QMetaType::QTime,   33,   34,
    QMetaType::Void, QMetaType::QString,   36,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void trainprogram::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<trainprogram *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->start(); break;
        case 1: _t->stop((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->lap(); break;
        case 3: _t->changeSpeed((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: { bool _r = _t->changeFanSpeed((*reinterpret_cast< uint8_t(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 5: _t->changeInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 6: _t->changeNextInclination300Meters((*reinterpret_cast< QList<MetersByInclination>(*)>(_a[1]))); break;
        case 7: _t->changeResistance((*reinterpret_cast< resistance_t(*)>(_a[1]))); break;
        case 8: _t->changeRequestedPelotonResistance((*reinterpret_cast< int8_t(*)>(_a[1]))); break;
        case 9: _t->changeCadence((*reinterpret_cast< int16_t(*)>(_a[1]))); break;
        case 10: _t->changePower((*reinterpret_cast< int32_t(*)>(_a[1]))); break;
        case 11: _t->changeSpeedAndInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 12: _t->changeGeoPosition((*reinterpret_cast< QGeoCoordinate(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 13: _t->changeTimestamp((*reinterpret_cast< QTime(*)>(_a[1])),(*reinterpret_cast< QTime(*)>(_a[2]))); break;
        case 14: _t->toastRequest((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 15: _t->onTapeStarted(); break;
        case 16: _t->scheduler(); break;
        case 17: _t->pelotonOCRprocessPendingDatagrams(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QGeoCoordinate >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (trainprogram::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::start)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::stop)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::lap)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeSpeed)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = bool (trainprogram::*)(uint8_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeFanSpeed)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeInclination)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(QList<MetersByInclination> );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeNextInclination300Meters)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(resistance_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeResistance)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(int8_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeRequestedPelotonResistance)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(int16_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeCadence)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(int32_t );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changePower)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeSpeedAndInclination)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(QGeoCoordinate , double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeGeoPosition)) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(QTime , QTime );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::changeTimestamp)) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (trainprogram::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&trainprogram::toastRequest)) {
                *result = 14;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject trainprogram::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_trainprogram.data,
    qt_meta_data_trainprogram,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *trainprogram::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *trainprogram::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_trainprogram.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int trainprogram::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void trainprogram::start()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void trainprogram::stop(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void trainprogram::lap()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void trainprogram::changeSpeed(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
bool trainprogram::changeFanSpeed(uint8_t _t1)
{
    bool _t0{};
    void *_a[] = { const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t0))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
    return _t0;
}

// SIGNAL 5
void trainprogram::changeInclination(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void trainprogram::changeNextInclination300Meters(QList<MetersByInclination> _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void trainprogram::changeResistance(resistance_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void trainprogram::changeRequestedPelotonResistance(int8_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void trainprogram::changeCadence(int16_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void trainprogram::changePower(int32_t _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void trainprogram::changeSpeedAndInclination(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void trainprogram::changeGeoPosition(QGeoCoordinate _t1, double _t2, double _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void trainprogram::changeTimestamp(QTime _t1, QTime _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void trainprogram::toastRequest(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
