/****************************************************************************
** Meta object code from reading C++ file 'proformbike.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "proformbike.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'proformbike.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_proformbike_t {
    QByteArrayData data[31];
    char stringdata0[483];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_proformbike_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_proformbike_t qt_meta_stringdata_proformbike = {
    {
QT_MOC_LITERAL(0, 0, 11), // "proformbike"
QT_MOC_LITERAL(1, 12, 12), // "disconnected"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 5), // "debug"
QT_MOC_LITERAL(4, 32, 6), // "string"
QT_MOC_LITERAL(5, 39, 16), // "deviceDiscovered"
QT_MOC_LITERAL(6, 56, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(7, 77, 6), // "device"
QT_MOC_LITERAL(8, 84, 21), // "characteristicChanged"
QT_MOC_LITERAL(9, 106, 24), // "QLowEnergyCharacteristic"
QT_MOC_LITERAL(10, 131, 14), // "characteristic"
QT_MOC_LITERAL(11, 146, 8), // "newValue"
QT_MOC_LITERAL(12, 155, 21), // "characteristicWritten"
QT_MOC_LITERAL(13, 177, 17), // "descriptorWritten"
QT_MOC_LITERAL(14, 195, 20), // "QLowEnergyDescriptor"
QT_MOC_LITERAL(15, 216, 10), // "descriptor"
QT_MOC_LITERAL(16, 227, 12), // "stateChanged"
QT_MOC_LITERAL(17, 240, 31), // "QLowEnergyService::ServiceState"
QT_MOC_LITERAL(18, 272, 5), // "state"
QT_MOC_LITERAL(19, 278, 22), // "controllerStateChanged"
QT_MOC_LITERAL(20, 301, 37), // "QLowEnergyController::Control..."
QT_MOC_LITERAL(21, 339, 17), // "serviceDiscovered"
QT_MOC_LITERAL(22, 357, 14), // "QBluetoothUuid"
QT_MOC_LITERAL(23, 372, 4), // "gatt"
QT_MOC_LITERAL(24, 377, 15), // "serviceScanDone"
QT_MOC_LITERAL(25, 393, 6), // "update"
QT_MOC_LITERAL(26, 400, 5), // "error"
QT_MOC_LITERAL(27, 406, 27), // "QLowEnergyController::Error"
QT_MOC_LITERAL(28, 434, 3), // "err"
QT_MOC_LITERAL(29, 438, 12), // "errorService"
QT_MOC_LITERAL(30, 451, 31) // "QLowEnergyService::ServiceError"

    },
    "proformbike\0disconnected\0\0debug\0string\0"
    "deviceDiscovered\0QBluetoothDeviceInfo\0"
    "device\0characteristicChanged\0"
    "QLowEnergyCharacteristic\0characteristic\0"
    "newValue\0characteristicWritten\0"
    "descriptorWritten\0QLowEnergyDescriptor\0"
    "descriptor\0stateChanged\0"
    "QLowEnergyService::ServiceState\0state\0"
    "controllerStateChanged\0"
    "QLowEnergyController::ControllerState\0"
    "serviceDiscovered\0QBluetoothUuid\0gatt\0"
    "serviceScanDone\0update\0error\0"
    "QLowEnergyController::Error\0err\0"
    "errorService\0QLowEnergyService::ServiceError"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_proformbike[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x06 /* Public */,
       3,    1,   80,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   83,    2, 0x0a /* Public */,
       8,    2,   86,    2, 0x08 /* Private */,
      12,    2,   91,    2, 0x08 /* Private */,
      13,    2,   96,    2, 0x08 /* Private */,
      16,    1,  101,    2, 0x08 /* Private */,
      19,    1,  104,    2, 0x08 /* Private */,
      21,    1,  107,    2, 0x08 /* Private */,
      24,    0,  110,    2, 0x08 /* Private */,
      25,    0,  111,    2, 0x08 /* Private */,
      26,    1,  112,    2, 0x08 /* Private */,
      29,    1,  115,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 9, QMetaType::QByteArray,   10,   11,
    QMetaType::Void, 0x80000000 | 9, QMetaType::QByteArray,   10,   11,
    QMetaType::Void, 0x80000000 | 14, QMetaType::QByteArray,   15,   11,
    QMetaType::Void, 0x80000000 | 17,   18,
    QMetaType::Void, 0x80000000 | 20,   18,
    QMetaType::Void, 0x80000000 | 22,   23,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 27,   28,
    QMetaType::Void, 0x80000000 | 30,    2,

       0        // eod
};

void proformbike::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<proformbike *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->deviceDiscovered((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 3: _t->characteristicChanged((*reinterpret_cast< const QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 4: _t->characteristicWritten((*reinterpret_cast< const QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 5: _t->descriptorWritten((*reinterpret_cast< const QLowEnergyDescriptor(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 6: _t->stateChanged((*reinterpret_cast< QLowEnergyService::ServiceState(*)>(_a[1]))); break;
        case 7: _t->controllerStateChanged((*reinterpret_cast< QLowEnergyController::ControllerState(*)>(_a[1]))); break;
        case 8: _t->serviceDiscovered((*reinterpret_cast< const QBluetoothUuid(*)>(_a[1]))); break;
        case 9: _t->serviceScanDone(); break;
        case 10: _t->update(); break;
        case 11: _t->error((*reinterpret_cast< QLowEnergyController::Error(*)>(_a[1]))); break;
        case 12: _t->errorService((*reinterpret_cast< QLowEnergyService::ServiceError(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothDeviceInfo >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyCharacteristic >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyCharacteristic >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyDescriptor >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyService::ServiceState >(); break;
            }
            break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyController::ControllerState >(); break;
            }
            break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothUuid >(); break;
            }
            break;
        case 11:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyController::Error >(); break;
            }
            break;
        case 12:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyService::ServiceError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (proformbike::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&proformbike::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (proformbike::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&proformbike::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject proformbike::staticMetaObject = { {
    QMetaObject::SuperData::link<bike::staticMetaObject>(),
    qt_meta_stringdata_proformbike.data,
    qt_meta_data_proformbike,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *proformbike::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proformbike::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proformbike.stringdata0))
        return static_cast<void*>(this);
    return bike::qt_metacast(_clname);
}

int proformbike::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = bike::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void proformbike::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void proformbike::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
