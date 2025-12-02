/****************************************************************************
** Meta object code from reading C++ file 'bluetoothsignalreceiver.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "Devices/bluetoothsignalreceiver.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'bluetoothsignalreceiver.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_BluetoothSignalReceiver_t {
    QByteArrayData data[6];
    char stringdata0[97];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_BluetoothSignalReceiver_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_BluetoothSignalReceiver_t qt_meta_stringdata_BluetoothSignalReceiver = {
    {
QT_MOC_LITERAL(0, 0, 23), // "BluetoothSignalReceiver"
QT_MOC_LITERAL(1, 24, 24), // "bluetoothDeviceConnected"
QT_MOC_LITERAL(2, 49, 0), // ""
QT_MOC_LITERAL(3, 50, 16), // "bluetoothdevice*"
QT_MOC_LITERAL(4, 67, 1), // "b"
QT_MOC_LITERAL(5, 69, 27) // "bluetoothDeviceDisconnected"

    },
    "BluetoothSignalReceiver\0"
    "bluetoothDeviceConnected\0\0bluetoothdevice*\0"
    "b\0bluetoothDeviceDisconnected"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_BluetoothSignalReceiver[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x0a /* Public */,
       5,    0,   27,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

       0        // eod
};

void BluetoothSignalReceiver::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<BluetoothSignalReceiver *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->bluetoothDeviceConnected((*reinterpret_cast< bluetoothdevice*(*)>(_a[1]))); break;
        case 1: _t->bluetoothDeviceDisconnected(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< bluetoothdevice* >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject BluetoothSignalReceiver::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_BluetoothSignalReceiver.data,
    qt_meta_data_BluetoothSignalReceiver,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *BluetoothSignalReceiver::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BluetoothSignalReceiver::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_BluetoothSignalReceiver.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BluetoothSignalReceiver::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
