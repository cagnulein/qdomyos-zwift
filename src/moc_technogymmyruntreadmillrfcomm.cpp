/****************************************************************************
** Meta object code from reading C++ file 'technogymmyruntreadmillrfcomm.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "technogymmyruntreadmillrfcomm.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'technogymmyruntreadmillrfcomm.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_technogymmyruntreadmillrfcomm_t {
    QByteArrayData data[22];
    char stringdata0[327];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_technogymmyruntreadmillrfcomm_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_technogymmyruntreadmillrfcomm_t qt_meta_stringdata_technogymmyruntreadmillrfcomm = {
    {
QT_MOC_LITERAL(0, 0, 29), // "technogymmyruntreadmillrfcomm"
QT_MOC_LITERAL(1, 30, 12), // "disconnected"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 5), // "debug"
QT_MOC_LITERAL(4, 50, 6), // "string"
QT_MOC_LITERAL(5, 57, 14), // "packetReceived"
QT_MOC_LITERAL(6, 72, 16), // "deviceDiscovered"
QT_MOC_LITERAL(7, 89, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(8, 110, 6), // "device"
QT_MOC_LITERAL(9, 117, 15), // "serviceCanceled"
QT_MOC_LITERAL(10, 133, 17), // "serviceDiscovered"
QT_MOC_LITERAL(11, 151, 21), // "QBluetoothServiceInfo"
QT_MOC_LITERAL(12, 173, 7), // "service"
QT_MOC_LITERAL(13, 181, 15), // "serviceFinished"
QT_MOC_LITERAL(14, 197, 10), // "readSocket"
QT_MOC_LITERAL(15, 208, 15), // "rfCommConnected"
QT_MOC_LITERAL(16, 224, 21), // "onSocketErrorOccurred"
QT_MOC_LITERAL(17, 246, 29), // "QBluetoothSocket::SocketError"
QT_MOC_LITERAL(18, 276, 6), // "update"
QT_MOC_LITERAL(19, 283, 26), // "changeInclinationRequested"
QT_MOC_LITERAL(20, 310, 5), // "grade"
QT_MOC_LITERAL(21, 316, 10) // "percentage"

    },
    "technogymmyruntreadmillrfcomm\0"
    "disconnected\0\0debug\0string\0packetReceived\0"
    "deviceDiscovered\0QBluetoothDeviceInfo\0"
    "device\0serviceCanceled\0serviceDiscovered\0"
    "QBluetoothServiceInfo\0service\0"
    "serviceFinished\0readSocket\0rfCommConnected\0"
    "onSocketErrorOccurred\0"
    "QBluetoothSocket::SocketError\0update\0"
    "changeInclinationRequested\0grade\0"
    "percentage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_technogymmyruntreadmillrfcomm[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x06 /* Public */,
       3,    1,   75,    2, 0x06 /* Public */,
       5,    0,   78,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   79,    2, 0x0a /* Public */,
       9,    0,   82,    2, 0x08 /* Private */,
      10,    1,   83,    2, 0x08 /* Private */,
      13,    0,   86,    2, 0x08 /* Private */,
      14,    0,   87,    2, 0x08 /* Private */,
      15,    0,   88,    2, 0x08 /* Private */,
      16,    1,   89,    2, 0x08 /* Private */,
      18,    0,   92,    2, 0x08 /* Private */,
      19,    2,   93,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 17,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   20,   21,

       0        // eod
};

void technogymmyruntreadmillrfcomm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<technogymmyruntreadmillrfcomm *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->packetReceived(); break;
        case 3: _t->deviceDiscovered((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 4: _t->serviceCanceled(); break;
        case 5: _t->serviceDiscovered((*reinterpret_cast< const QBluetoothServiceInfo(*)>(_a[1]))); break;
        case 6: _t->serviceFinished(); break;
        case 7: _t->readSocket(); break;
        case 8: _t->rfCommConnected(); break;
        case 9: _t->onSocketErrorOccurred((*reinterpret_cast< QBluetoothSocket::SocketError(*)>(_a[1]))); break;
        case 10: _t->update(); break;
        case 11: _t->changeInclinationRequested((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
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
        case 5:
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
            using _t = void (technogymmyruntreadmillrfcomm::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&technogymmyruntreadmillrfcomm::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (technogymmyruntreadmillrfcomm::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&technogymmyruntreadmillrfcomm::debug)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (technogymmyruntreadmillrfcomm::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&technogymmyruntreadmillrfcomm::packetReceived)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject technogymmyruntreadmillrfcomm::staticMetaObject = { {
    QMetaObject::SuperData::link<treadmill::staticMetaObject>(),
    qt_meta_stringdata_technogymmyruntreadmillrfcomm.data,
    qt_meta_data_technogymmyruntreadmillrfcomm,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *technogymmyruntreadmillrfcomm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *technogymmyruntreadmillrfcomm::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_technogymmyruntreadmillrfcomm.stringdata0))
        return static_cast<void*>(this);
    return treadmill::qt_metacast(_clname);
}

int technogymmyruntreadmillrfcomm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = treadmill::qt_metacall(_c, _id, _a);
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
void technogymmyruntreadmillrfcomm::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void technogymmyruntreadmillrfcomm::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void technogymmyruntreadmillrfcomm::packetReceived()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
