/****************************************************************************
** Meta object code from reading C++ file 'iconceptbike.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "iconceptbike.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'iconceptbike.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_iconceptbike_t {
    QByteArrayData data[17];
    char stringdata0[235];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_iconceptbike_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_iconceptbike_t qt_meta_stringdata_iconceptbike = {
    {
QT_MOC_LITERAL(0, 0, 12), // "iconceptbike"
QT_MOC_LITERAL(1, 13, 12), // "disconnected"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 5), // "debug"
QT_MOC_LITERAL(4, 33, 6), // "string"
QT_MOC_LITERAL(5, 40, 16), // "deviceDiscovered"
QT_MOC_LITERAL(6, 57, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(7, 78, 6), // "device"
QT_MOC_LITERAL(8, 85, 17), // "serviceDiscovered"
QT_MOC_LITERAL(9, 103, 21), // "QBluetoothServiceInfo"
QT_MOC_LITERAL(10, 125, 7), // "service"
QT_MOC_LITERAL(11, 133, 15), // "serviceFinished"
QT_MOC_LITERAL(12, 149, 10), // "readSocket"
QT_MOC_LITERAL(13, 160, 15), // "rfCommConnected"
QT_MOC_LITERAL(14, 176, 21), // "onSocketErrorOccurred"
QT_MOC_LITERAL(15, 198, 29), // "QBluetoothSocket::SocketError"
QT_MOC_LITERAL(16, 228, 6) // "update"

    },
    "iconceptbike\0disconnected\0\0debug\0"
    "string\0deviceDiscovered\0QBluetoothDeviceInfo\0"
    "device\0serviceDiscovered\0QBluetoothServiceInfo\0"
    "service\0serviceFinished\0readSocket\0"
    "rfCommConnected\0onSocketErrorOccurred\0"
    "QBluetoothSocket::SocketError\0update"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_iconceptbike[] = {

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
       5,    1,   63,    2, 0x0a /* Public */,
       8,    1,   66,    2, 0x08 /* Private */,
      11,    0,   69,    2, 0x08 /* Private */,
      12,    0,   70,    2, 0x08 /* Private */,
      13,    0,   71,    2, 0x08 /* Private */,
      14,    1,   72,    2, 0x08 /* Private */,
      16,    0,   75,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 15,    2,
    QMetaType::Void,

       0        // eod
};

void iconceptbike::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<iconceptbike *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->deviceDiscovered((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 3: _t->serviceDiscovered((*reinterpret_cast< const QBluetoothServiceInfo(*)>(_a[1]))); break;
        case 4: _t->serviceFinished(); break;
        case 5: _t->readSocket(); break;
        case 6: _t->rfCommConnected(); break;
        case 7: _t->onSocketErrorOccurred((*reinterpret_cast< QBluetoothSocket::SocketError(*)>(_a[1]))); break;
        case 8: _t->update(); break;
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
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothServiceInfo >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (iconceptbike::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&iconceptbike::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (iconceptbike::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&iconceptbike::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject iconceptbike::staticMetaObject = { {
    QMetaObject::SuperData::link<bike::staticMetaObject>(),
    qt_meta_stringdata_iconceptbike.data,
    qt_meta_data_iconceptbike,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *iconceptbike::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *iconceptbike::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_iconceptbike.stringdata0))
        return static_cast<void*>(this);
    return bike::qt_metacast(_clname);
}

int iconceptbike::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void iconceptbike::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void iconceptbike::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
