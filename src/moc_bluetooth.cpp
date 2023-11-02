/****************************************************************************
** Meta object code from reading C++ file 'bluetooth.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "bluetooth.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'bluetooth.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_bluetooth_t {
    QByteArrayData data[30];
    char stringdata0[387];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_bluetooth_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_bluetooth_t qt_meta_stringdata_bluetooth = {
    {
QT_MOC_LITERAL(0, 0, 9), // "bluetooth"
QT_MOC_LITERAL(1, 10, 15), // "deviceConnected"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(4, 48, 1), // "b"
QT_MOC_LITERAL(5, 50, 11), // "deviceFound"
QT_MOC_LITERAL(6, 62, 4), // "name"
QT_MOC_LITERAL(7, 67, 13), // "searchingStop"
QT_MOC_LITERAL(8, 81, 22), // "ftmsAccessoryConnected"
QT_MOC_LITERAL(9, 104, 12), // "smartspin2k*"
QT_MOC_LITERAL(10, 117, 1), // "d"
QT_MOC_LITERAL(11, 119, 24), // "bluetoothDeviceConnected"
QT_MOC_LITERAL(12, 144, 16), // "bluetoothdevice*"
QT_MOC_LITERAL(13, 161, 27), // "bluetoothDeviceDisconnected"
QT_MOC_LITERAL(14, 189, 7), // "restart"
QT_MOC_LITERAL(15, 197, 5), // "debug"
QT_MOC_LITERAL(16, 203, 6), // "string"
QT_MOC_LITERAL(17, 210, 9), // "heartRate"
QT_MOC_LITERAL(18, 220, 7), // "uint8_t"
QT_MOC_LITERAL(19, 228, 5), // "heart"
QT_MOC_LITERAL(20, 234, 16), // "deviceDiscovered"
QT_MOC_LITERAL(21, 251, 6), // "device"
QT_MOC_LITERAL(22, 258, 13), // "deviceUpdated"
QT_MOC_LITERAL(23, 272, 28), // "QBluetoothDeviceInfo::Fields"
QT_MOC_LITERAL(24, 301, 12), // "updateFields"
QT_MOC_LITERAL(25, 314, 8), // "canceled"
QT_MOC_LITERAL(26, 323, 8), // "finished"
QT_MOC_LITERAL(27, 332, 12), // "speedChanged"
QT_MOC_LITERAL(28, 345, 18), // "inclinationChanged"
QT_MOC_LITERAL(29, 364, 22) // "connectedAndDiscovered"

    },
    "bluetooth\0deviceConnected\0\0"
    "QBluetoothDeviceInfo\0b\0deviceFound\0"
    "name\0searchingStop\0ftmsAccessoryConnected\0"
    "smartspin2k*\0d\0bluetoothDeviceConnected\0"
    "bluetoothdevice*\0bluetoothDeviceDisconnected\0"
    "restart\0debug\0string\0heartRate\0uint8_t\0"
    "heart\0deviceDiscovered\0device\0"
    "deviceUpdated\0QBluetoothDeviceInfo::Fields\0"
    "updateFields\0canceled\0finished\0"
    "speedChanged\0inclinationChanged\0"
    "connectedAndDiscovered"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_bluetooth[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      16,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   94,    2, 0x06 /* Public */,
       5,    1,   97,    2, 0x06 /* Public */,
       7,    0,  100,    2, 0x06 /* Public */,
       8,    1,  101,    2, 0x06 /* Public */,
      11,    1,  104,    2, 0x06 /* Public */,
      13,    0,  107,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      14,    0,  108,    2, 0x0a /* Public */,
      15,    1,  109,    2, 0x0a /* Public */,
      17,    1,  112,    2, 0x0a /* Public */,
      20,    1,  115,    2, 0x0a /* Public */,
      22,    2,  118,    2, 0x08 /* Private */,
      25,    0,  123,    2, 0x08 /* Private */,
      26,    0,  124,    2, 0x08 /* Private */,
      27,    1,  125,    2, 0x08 /* Private */,
      28,    2,  128,    2, 0x08 /* Private */,
      29,    0,  133,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void, 0x80000000 | 12,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   16,
    QMetaType::Void, 0x80000000 | 18,   19,
    QMetaType::Void, 0x80000000 | 3,   21,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 23,   21,   24,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    2,    2,
    QMetaType::Void,

       0        // eod
};

void bluetooth::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<bluetooth *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->deviceConnected((*reinterpret_cast< QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 1: _t->deviceFound((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->searchingStop(); break;
        case 3: _t->ftmsAccessoryConnected((*reinterpret_cast< smartspin2k*(*)>(_a[1]))); break;
        case 4: _t->bluetoothDeviceConnected((*reinterpret_cast< bluetoothdevice*(*)>(_a[1]))); break;
        case 5: _t->bluetoothDeviceDisconnected(); break;
        case 6: _t->restart(); break;
        case 7: _t->debug((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->heartRate((*reinterpret_cast< uint8_t(*)>(_a[1]))); break;
        case 9: _t->deviceDiscovered((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 10: _t->deviceUpdated((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1])),(*reinterpret_cast< QBluetoothDeviceInfo::Fields(*)>(_a[2]))); break;
        case 11: _t->canceled(); break;
        case 12: _t->finished(); break;
        case 13: _t->speedChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 14: _t->inclinationChanged((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 15: _t->connectedAndDiscovered(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
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
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< smartspin2k* >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< bluetoothdevice* >(); break;
            }
            break;
        case 9:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothDeviceInfo >(); break;
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
            using _t = void (bluetooth::*)(QBluetoothDeviceInfo );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bluetooth::deviceConnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (bluetooth::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bluetooth::deviceFound)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (bluetooth::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bluetooth::searchingStop)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (bluetooth::*)(smartspin2k * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bluetooth::ftmsAccessoryConnected)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (bluetooth::*)(bluetoothdevice * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bluetooth::bluetoothDeviceConnected)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (bluetooth::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&bluetooth::bluetoothDeviceDisconnected)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject bluetooth::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_bluetooth.data,
    qt_meta_data_bluetooth,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *bluetooth::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *bluetooth::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_bluetooth.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "SignalHandler"))
        return static_cast< SignalHandler*>(this);
    return QObject::qt_metacast(_clname);
}

int bluetooth::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 16)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 16;
    }
    return _id;
}

// SIGNAL 0
void bluetooth::deviceConnected(QBluetoothDeviceInfo _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void bluetooth::deviceFound(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void bluetooth::searchingStop()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void bluetooth::ftmsAccessoryConnected(smartspin2k * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void bluetooth::bluetoothDeviceConnected(bluetoothdevice * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void bluetooth::bluetoothDeviceDisconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
