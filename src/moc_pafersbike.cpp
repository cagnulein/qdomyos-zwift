/****************************************************************************
** Meta object code from reading C++ file 'pafersbike.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "pafersbike.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'pafersbike.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_pafersbike_t {
    QByteArrayData data[29];
    char stringdata0[469];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_pafersbike_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_pafersbike_t qt_meta_stringdata_pafersbike = {
    {
QT_MOC_LITERAL(0, 0, 10), // "pafersbike"
QT_MOC_LITERAL(1, 11, 12), // "disconnected"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 16), // "deviceDiscovered"
QT_MOC_LITERAL(4, 42, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(5, 63, 6), // "device"
QT_MOC_LITERAL(6, 70, 21), // "characteristicChanged"
QT_MOC_LITERAL(7, 92, 24), // "QLowEnergyCharacteristic"
QT_MOC_LITERAL(8, 117, 14), // "characteristic"
QT_MOC_LITERAL(9, 132, 8), // "newValue"
QT_MOC_LITERAL(10, 141, 21), // "characteristicWritten"
QT_MOC_LITERAL(11, 163, 17), // "descriptorWritten"
QT_MOC_LITERAL(12, 181, 20), // "QLowEnergyDescriptor"
QT_MOC_LITERAL(13, 202, 10), // "descriptor"
QT_MOC_LITERAL(14, 213, 12), // "stateChanged"
QT_MOC_LITERAL(15, 226, 31), // "QLowEnergyService::ServiceState"
QT_MOC_LITERAL(16, 258, 5), // "state"
QT_MOC_LITERAL(17, 264, 22), // "controllerStateChanged"
QT_MOC_LITERAL(18, 287, 37), // "QLowEnergyController::Control..."
QT_MOC_LITERAL(19, 325, 17), // "serviceDiscovered"
QT_MOC_LITERAL(20, 343, 14), // "QBluetoothUuid"
QT_MOC_LITERAL(21, 358, 4), // "gatt"
QT_MOC_LITERAL(22, 363, 15), // "serviceScanDone"
QT_MOC_LITERAL(23, 379, 6), // "update"
QT_MOC_LITERAL(24, 386, 5), // "error"
QT_MOC_LITERAL(25, 392, 27), // "QLowEnergyController::Error"
QT_MOC_LITERAL(26, 420, 3), // "err"
QT_MOC_LITERAL(27, 424, 12), // "errorService"
QT_MOC_LITERAL(28, 437, 31) // "QLowEnergyService::ServiceError"

    },
    "pafersbike\0disconnected\0\0deviceDiscovered\0"
    "QBluetoothDeviceInfo\0device\0"
    "characteristicChanged\0QLowEnergyCharacteristic\0"
    "characteristic\0newValue\0characteristicWritten\0"
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

static const uint qt_meta_data_pafersbike[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   75,    2, 0x0a /* Public */,
       6,    2,   78,    2, 0x08 /* Private */,
      10,    2,   83,    2, 0x08 /* Private */,
      11,    2,   88,    2, 0x08 /* Private */,
      14,    1,   93,    2, 0x08 /* Private */,
      17,    1,   96,    2, 0x08 /* Private */,
      19,    1,   99,    2, 0x08 /* Private */,
      22,    0,  102,    2, 0x08 /* Private */,
      23,    0,  103,    2, 0x08 /* Private */,
      24,    1,  104,    2, 0x08 /* Private */,
      27,    1,  107,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, 0x80000000 | 7, QMetaType::QByteArray,    8,    9,
    QMetaType::Void, 0x80000000 | 7, QMetaType::QByteArray,    8,    9,
    QMetaType::Void, 0x80000000 | 12, QMetaType::QByteArray,   13,    9,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void, 0x80000000 | 18,   16,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 25,   26,
    QMetaType::Void, 0x80000000 | 28,    2,

       0        // eod
};

void pafersbike::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<pafersbike *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->deviceDiscovered((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 2: _t->characteristicChanged((*reinterpret_cast< const QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 3: _t->characteristicWritten((*reinterpret_cast< const QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 4: _t->descriptorWritten((*reinterpret_cast< const QLowEnergyDescriptor(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        case 5: _t->stateChanged((*reinterpret_cast< QLowEnergyService::ServiceState(*)>(_a[1]))); break;
        case 6: _t->controllerStateChanged((*reinterpret_cast< QLowEnergyController::ControllerState(*)>(_a[1]))); break;
        case 7: _t->serviceDiscovered((*reinterpret_cast< const QBluetoothUuid(*)>(_a[1]))); break;
        case 8: _t->serviceScanDone(); break;
        case 9: _t->update(); break;
        case 10: _t->error((*reinterpret_cast< QLowEnergyController::Error(*)>(_a[1]))); break;
        case 11: _t->errorService((*reinterpret_cast< QLowEnergyService::ServiceError(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothDeviceInfo >(); break;
            }
            break;
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
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyDescriptor >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyService::ServiceState >(); break;
            }
            break;
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyController::ControllerState >(); break;
            }
            break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothUuid >(); break;
            }
            break;
        case 10:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyController::Error >(); break;
            }
            break;
        case 11:
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
            using _t = void (pafersbike::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&pafersbike::disconnected)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject pafersbike::staticMetaObject = { {
    QMetaObject::SuperData::link<bike::staticMetaObject>(),
    qt_meta_stringdata_pafersbike.data,
    qt_meta_data_pafersbike,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *pafersbike::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *pafersbike::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_pafersbike.stringdata0))
        return static_cast<void*>(this);
    return bike::qt_metacast(_clname);
}

int pafersbike::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = bike::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void pafersbike::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
