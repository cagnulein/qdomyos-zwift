/****************************************************************************
** Meta object code from reading C++ file 'proformwifitreadmill.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "proformwifitreadmill.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'proformwifitreadmill.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_proformwifitreadmill_t {
    QByteArrayData data[18];
    char stringdata0[234];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_proformwifitreadmill_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_proformwifitreadmill_t qt_meta_stringdata_proformwifitreadmill = {
    {
QT_MOC_LITERAL(0, 0, 20), // "proformwifitreadmill"
QT_MOC_LITERAL(1, 21, 12), // "disconnected"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 5), // "debug"
QT_MOC_LITERAL(4, 41, 6), // "string"
QT_MOC_LITERAL(5, 48, 16), // "deviceDiscovered"
QT_MOC_LITERAL(6, 65, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(7, 86, 6), // "device"
QT_MOC_LITERAL(8, 93, 21), // "characteristicChanged"
QT_MOC_LITERAL(9, 115, 7), // "message"
QT_MOC_LITERAL(10, 123, 21), // "binaryMessageReceived"
QT_MOC_LITERAL(11, 145, 26), // "changeInclinationRequested"
QT_MOC_LITERAL(12, 172, 5), // "grade"
QT_MOC_LITERAL(13, 178, 10), // "percentage"
QT_MOC_LITERAL(14, 189, 17), // "serviceDiscovered"
QT_MOC_LITERAL(15, 207, 14), // "QBluetoothUuid"
QT_MOC_LITERAL(16, 222, 4), // "gatt"
QT_MOC_LITERAL(17, 227, 6) // "update"

    },
    "proformwifitreadmill\0disconnected\0\0"
    "debug\0string\0deviceDiscovered\0"
    "QBluetoothDeviceInfo\0device\0"
    "characteristicChanged\0message\0"
    "binaryMessageReceived\0changeInclinationRequested\0"
    "grade\0percentage\0serviceDiscovered\0"
    "QBluetoothUuid\0gatt\0update"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_proformwifitreadmill[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x06 /* Public */,
       3,    1,   55,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   58,    2, 0x0a /* Public */,
       8,    1,   61,    2, 0x08 /* Private */,
      10,    1,   64,    2, 0x08 /* Private */,
      11,    2,   67,    2, 0x08 /* Private */,
      14,    1,   72,    2, 0x08 /* Private */,
      17,    0,   75,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::QString,    9,
    QMetaType::Void, QMetaType::QByteArray,    9,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   12,   13,
    QMetaType::Void, 0x80000000 | 15,   16,
    QMetaType::Void,

       0        // eod
};

void proformwifitreadmill::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<proformwifitreadmill *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->deviceDiscovered((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 3: _t->characteristicChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 4: _t->binaryMessageReceived((*reinterpret_cast< const QByteArray(*)>(_a[1]))); break;
        case 5: _t->changeInclinationRequested((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 6: _t->serviceDiscovered((*reinterpret_cast< const QBluetoothUuid(*)>(_a[1]))); break;
        case 7: _t->update(); break;
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
        case 6:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QBluetoothUuid >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (proformwifitreadmill::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&proformwifitreadmill::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (proformwifitreadmill::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&proformwifitreadmill::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject proformwifitreadmill::staticMetaObject = { {
    QMetaObject::SuperData::link<treadmill::staticMetaObject>(),
    qt_meta_stringdata_proformwifitreadmill.data,
    qt_meta_data_proformwifitreadmill,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *proformwifitreadmill::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *proformwifitreadmill::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_proformwifitreadmill.stringdata0))
        return static_cast<void*>(this);
    return treadmill::qt_metacast(_clname);
}

int proformwifitreadmill::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = treadmill::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void proformwifitreadmill::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void proformwifitreadmill::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
