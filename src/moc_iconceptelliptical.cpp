/****************************************************************************
** Meta object code from reading C++ file 'iconceptelliptical.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "iconceptelliptical.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'iconceptelliptical.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_iconceptelliptical_t {
    QByteArrayData data[20];
    char stringdata0[285];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_iconceptelliptical_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_iconceptelliptical_t qt_meta_stringdata_iconceptelliptical = {
    {
QT_MOC_LITERAL(0, 0, 18), // "iconceptelliptical"
QT_MOC_LITERAL(1, 19, 12), // "disconnected"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 5), // "debug"
QT_MOC_LITERAL(4, 39, 6), // "string"
QT_MOC_LITERAL(5, 46, 16), // "deviceDiscovered"
QT_MOC_LITERAL(6, 63, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(7, 84, 6), // "device"
QT_MOC_LITERAL(8, 91, 17), // "serviceDiscovered"
QT_MOC_LITERAL(9, 109, 21), // "QBluetoothServiceInfo"
QT_MOC_LITERAL(10, 131, 7), // "service"
QT_MOC_LITERAL(11, 139, 15), // "serviceFinished"
QT_MOC_LITERAL(12, 155, 10), // "readSocket"
QT_MOC_LITERAL(13, 166, 15), // "rfCommConnected"
QT_MOC_LITERAL(14, 182, 21), // "onSocketErrorOccurred"
QT_MOC_LITERAL(15, 204, 29), // "QBluetoothSocket::SocketError"
QT_MOC_LITERAL(16, 234, 6), // "update"
QT_MOC_LITERAL(17, 241, 26), // "changeInclinationRequested"
QT_MOC_LITERAL(18, 268, 5), // "grade"
QT_MOC_LITERAL(19, 274, 10) // "percentage"

    },
    "iconceptelliptical\0disconnected\0\0debug\0"
    "string\0deviceDiscovered\0QBluetoothDeviceInfo\0"
    "device\0serviceDiscovered\0QBluetoothServiceInfo\0"
    "service\0serviceFinished\0readSocket\0"
    "rfCommConnected\0onSocketErrorOccurred\0"
    "QBluetoothSocket::SocketError\0update\0"
    "changeInclinationRequested\0grade\0"
    "percentage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_iconceptelliptical[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x06 /* Public */,
       3,    1,   65,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   68,    2, 0x0a /* Public */,
       8,    1,   71,    2, 0x08 /* Private */,
      11,    0,   74,    2, 0x08 /* Private */,
      12,    0,   75,    2, 0x08 /* Private */,
      13,    0,   76,    2, 0x08 /* Private */,
      14,    1,   77,    2, 0x08 /* Private */,
      16,    0,   80,    2, 0x08 /* Private */,
      17,    2,   81,    2, 0x08 /* Private */,

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
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   18,   19,

       0        // eod
};

void iconceptelliptical::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<iconceptelliptical *>(_o);
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
        case 9: _t->changeInclinationRequested((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
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
            using _t = void (iconceptelliptical::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&iconceptelliptical::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (iconceptelliptical::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&iconceptelliptical::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject iconceptelliptical::staticMetaObject = { {
    QMetaObject::SuperData::link<elliptical::staticMetaObject>(),
    qt_meta_stringdata_iconceptelliptical.data,
    qt_meta_data_iconceptelliptical,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *iconceptelliptical::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *iconceptelliptical::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_iconceptelliptical.stringdata0))
        return static_cast<void*>(this);
    return elliptical::qt_metacast(_clname);
}

int iconceptelliptical::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = elliptical::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void iconceptelliptical::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void iconceptelliptical::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
