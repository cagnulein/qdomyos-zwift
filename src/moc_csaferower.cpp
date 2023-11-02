/****************************************************************************
** Meta object code from reading C++ file 'csaferower.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "csaferower.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'csaferower.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_csaferowerThread_t {
    QByteArrayData data[16];
    char stringdata0[127];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_csaferowerThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_csaferowerThread_t qt_meta_stringdata_csaferowerThread = {
    {
QT_MOC_LITERAL(0, 0, 16), // "csaferowerThread"
QT_MOC_LITERAL(1, 17, 7), // "onDebug"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 5), // "debug"
QT_MOC_LITERAL(4, 32, 9), // "newPacket"
QT_MOC_LITERAL(5, 42, 1), // "p"
QT_MOC_LITERAL(6, 44, 7), // "onPower"
QT_MOC_LITERAL(7, 52, 5), // "power"
QT_MOC_LITERAL(8, 58, 9), // "onCadence"
QT_MOC_LITERAL(9, 68, 7), // "cadence"
QT_MOC_LITERAL(10, 76, 7), // "onHeart"
QT_MOC_LITERAL(11, 84, 2), // "hr"
QT_MOC_LITERAL(12, 87, 10), // "onCalories"
QT_MOC_LITERAL(13, 98, 8), // "calories"
QT_MOC_LITERAL(14, 107, 10), // "onDistance"
QT_MOC_LITERAL(15, 118, 8) // "distance"

    },
    "csaferowerThread\0onDebug\0\0debug\0"
    "newPacket\0p\0onPower\0power\0onCadence\0"
    "cadence\0onHeart\0hr\0onCalories\0calories\0"
    "onDistance\0distance"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_csaferowerThread[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       4,    1,   52,    2, 0x06 /* Public */,
       6,    1,   55,    2, 0x06 /* Public */,
       8,    1,   58,    2, 0x06 /* Public */,
      10,    1,   61,    2, 0x06 /* Public */,
      12,    1,   64,    2, 0x06 /* Public */,
      14,    1,   67,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QByteArray,    5,
    QMetaType::Void, QMetaType::Double,    7,
    QMetaType::Void, QMetaType::Double,    9,
    QMetaType::Void, QMetaType::Double,   11,
    QMetaType::Void, QMetaType::Double,   13,
    QMetaType::Void, QMetaType::Double,   15,

       0        // eod
};

void csaferowerThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<csaferowerThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onDebug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->newPacket((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 2: _t->onPower((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->onCadence((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: _t->onHeart((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->onCalories((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->onDistance((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (csaferowerThread::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&csaferowerThread::onDebug)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (csaferowerThread::*)(QByteArray );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&csaferowerThread::newPacket)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (csaferowerThread::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&csaferowerThread::onPower)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (csaferowerThread::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&csaferowerThread::onCadence)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (csaferowerThread::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&csaferowerThread::onHeart)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (csaferowerThread::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&csaferowerThread::onCalories)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (csaferowerThread::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&csaferowerThread::onDistance)) {
                *result = 6;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject csaferowerThread::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_csaferowerThread.data,
    qt_meta_data_csaferowerThread,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *csaferowerThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *csaferowerThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_csaferowerThread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int csaferowerThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void csaferowerThread::onDebug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void csaferowerThread::newPacket(QByteArray _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void csaferowerThread::onPower(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void csaferowerThread::onCadence(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void csaferowerThread::onHeart(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void csaferowerThread::onCalories(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void csaferowerThread::onDistance(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
struct qt_meta_stringdata_csaferower_t {
    QByteArrayData data[25];
    char stringdata0[260];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_csaferower_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_csaferower_t qt_meta_stringdata_csaferower = {
    {
QT_MOC_LITERAL(0, 0, 10), // "csaferower"
QT_MOC_LITERAL(1, 11, 12), // "disconnected"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 5), // "debug"
QT_MOC_LITERAL(4, 31, 6), // "string"
QT_MOC_LITERAL(5, 38, 6), // "update"
QT_MOC_LITERAL(6, 45, 9), // "newPacket"
QT_MOC_LITERAL(7, 55, 1), // "p"
QT_MOC_LITERAL(8, 57, 25), // "ftmsCharacteristicChanged"
QT_MOC_LITERAL(9, 83, 24), // "QLowEnergyCharacteristic"
QT_MOC_LITERAL(10, 108, 14), // "characteristic"
QT_MOC_LITERAL(11, 123, 8), // "newValue"
QT_MOC_LITERAL(12, 132, 7), // "onPower"
QT_MOC_LITERAL(13, 140, 5), // "power"
QT_MOC_LITERAL(14, 146, 9), // "onCadence"
QT_MOC_LITERAL(15, 156, 7), // "cadence"
QT_MOC_LITERAL(16, 164, 7), // "onHeart"
QT_MOC_LITERAL(17, 172, 2), // "hr"
QT_MOC_LITERAL(18, 175, 10), // "onCalories"
QT_MOC_LITERAL(19, 186, 8), // "calories"
QT_MOC_LITERAL(20, 195, 10), // "onDistance"
QT_MOC_LITERAL(21, 206, 8), // "distance"
QT_MOC_LITERAL(22, 215, 16), // "deviceDiscovered"
QT_MOC_LITERAL(23, 232, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(24, 253, 6) // "device"

    },
    "csaferower\0disconnected\0\0debug\0string\0"
    "update\0newPacket\0p\0ftmsCharacteristicChanged\0"
    "QLowEnergyCharacteristic\0characteristic\0"
    "newValue\0onPower\0power\0onCadence\0"
    "cadence\0onHeart\0hr\0onCalories\0calories\0"
    "onDistance\0distance\0deviceDiscovered\0"
    "QBluetoothDeviceInfo\0device"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_csaferower[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   69,    2, 0x06 /* Public */,
       3,    1,   70,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   73,    2, 0x08 /* Private */,
       6,    1,   74,    2, 0x08 /* Private */,
       8,    2,   77,    2, 0x08 /* Private */,
      12,    1,   82,    2, 0x08 /* Private */,
      14,    1,   85,    2, 0x08 /* Private */,
      16,    1,   88,    2, 0x08 /* Private */,
      18,    1,   91,    2, 0x08 /* Private */,
      20,    1,   94,    2, 0x08 /* Private */,
      22,    1,   97,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,    7,
    QMetaType::Void, 0x80000000 | 9, QMetaType::QByteArray,   10,   11,
    QMetaType::Void, QMetaType::Double,   13,
    QMetaType::Void, QMetaType::Double,   15,
    QMetaType::Void, QMetaType::Double,   17,
    QMetaType::Void, QMetaType::Double,   19,
    QMetaType::Void, QMetaType::Double,   21,
    QMetaType::Void, 0x80000000 | 23,   24,

       0        // eod
};

void csaferower::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<csaferower *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->update(); break;
        case 3: _t->newPacket((*reinterpret_cast< QByteArray(*)>(_a[1]))); break;
        case 4: _t->ftmsCharacteristicChanged((*reinterpret_cast< const QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 5: _t->onPower((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->onCadence((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->onHeart((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 8: _t->onCalories((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: _t->onDistance((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 10: _t->deviceDiscovered((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
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
        case 10:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothDeviceInfo >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (csaferower::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&csaferower::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (csaferower::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&csaferower::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject csaferower::staticMetaObject = { {
    QMetaObject::SuperData::link<rower::staticMetaObject>(),
    qt_meta_stringdata_csaferower.data,
    qt_meta_data_csaferower,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *csaferower::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *csaferower::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_csaferower.stringdata0))
        return static_cast<void*>(this);
    return rower::qt_metacast(_clname);
}

int csaferower::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = rower::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void csaferower::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void csaferower::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
