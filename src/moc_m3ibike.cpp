/****************************************************************************
** Meta object code from reading C++ file 'm3ibike.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "m3ibike.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'm3ibike.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_m3ibike_t {
    QByteArrayData data[16];
    char stringdata0[220];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_m3ibike_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_m3ibike_t qt_meta_stringdata_m3ibike = {
    {
QT_MOC_LITERAL(0, 0, 7), // "m3ibike"
QT_MOC_LITERAL(1, 8, 12), // "disconnected"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 5), // "debug"
QT_MOC_LITERAL(4, 28, 6), // "string"
QT_MOC_LITERAL(5, 35, 13), // "searchingStop"
QT_MOC_LITERAL(6, 49, 16), // "deviceDiscovered"
QT_MOC_LITERAL(7, 66, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(8, 87, 6), // "device"
QT_MOC_LITERAL(9, 94, 17), // "deviceUpdatedPriv"
QT_MOC_LITERAL(10, 112, 28), // "QBluetoothDeviceInfo::Fields"
QT_MOC_LITERAL(11, 141, 20), // "deviceDiscoveredPriv"
QT_MOC_LITERAL(12, 162, 21), // "discoveryFinishedPriv"
QT_MOC_LITERAL(13, 184, 18), // "processAdvertising"
QT_MOC_LITERAL(14, 203, 4), // "data"
QT_MOC_LITERAL(15, 208, 11) // "restartScan"

    },
    "m3ibike\0disconnected\0\0debug\0string\0"
    "searchingStop\0deviceDiscovered\0"
    "QBluetoothDeviceInfo\0device\0"
    "deviceUpdatedPriv\0QBluetoothDeviceInfo::Fields\0"
    "deviceDiscoveredPriv\0discoveryFinishedPriv\0"
    "processAdvertising\0data\0restartScan"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_m3ibike[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   59,    2, 0x06 /* Public */,
       3,    1,   60,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   63,    2, 0x0a /* Public */,
       6,    1,   64,    2, 0x0a /* Public */,
       9,    2,   67,    2, 0x08 /* Private */,
      11,    1,   72,    2, 0x08 /* Private */,
      12,    0,   75,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
      13,    1,   76,    2, 0x00 /* Private */,
      15,    0,   79,    2, 0x00 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void, 0x80000000 | 7, 0x80000000 | 10,    2,    2,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void, QMetaType::QByteArray,   14,
    QMetaType::Void,

       0        // eod
};

void m3ibike::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<m3ibike *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->searchingStop(); break;
        case 3: _t->deviceDiscovered((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 4: _t->deviceUpdatedPriv((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1])),(*reinterpret_cast< QBluetoothDeviceInfo::Fields(*)>(_a[2]))); break;
        case 5: _t->deviceDiscoveredPriv((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 6: _t->discoveryFinishedPriv(); break;
        case 7: _t->processAdvertising((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 8: _t->restartScan(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothDeviceInfo >(); break;
            }
            break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothDeviceInfo >(); break;
            }
            break;
        case 5:
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
            using _t = void (m3ibike::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&m3ibike::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (m3ibike::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&m3ibike::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject m3ibike::staticMetaObject = { {
    QMetaObject::SuperData::link<bike::staticMetaObject>(),
    qt_meta_stringdata_m3ibike.data,
    qt_meta_data_m3ibike,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *m3ibike::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *m3ibike::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_m3ibike.stringdata0))
        return static_cast<void*>(this);
    return bike::qt_metacast(_clname);
}

int m3ibike::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = bike::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void m3ibike::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void m3ibike::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
