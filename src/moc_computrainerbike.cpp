/****************************************************************************
** Meta object code from reading C++ file 'computrainerbike.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "computrainerbike.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'computrainerbike.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_computrainerbike_t {
    QByteArrayData data[9];
    char stringdata0[96];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_computrainerbike_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_computrainerbike_t qt_meta_stringdata_computrainerbike = {
    {
QT_MOC_LITERAL(0, 0, 16), // "computrainerbike"
QT_MOC_LITERAL(1, 17, 12), // "disconnected"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 5), // "debug"
QT_MOC_LITERAL(4, 37, 6), // "string"
QT_MOC_LITERAL(5, 44, 16), // "deviceDiscovered"
QT_MOC_LITERAL(6, 61, 20), // "QBluetoothDeviceInfo"
QT_MOC_LITERAL(7, 82, 6), // "device"
QT_MOC_LITERAL(8, 89, 6) // "update"

    },
    "computrainerbike\0disconnected\0\0debug\0"
    "string\0deviceDiscovered\0QBluetoothDeviceInfo\0"
    "device\0update"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_computrainerbike[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x06 /* Public */,
       3,    1,   35,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   38,    2, 0x0a /* Public */,
       8,    0,   41,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,

       0        // eod
};

void computrainerbike::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<computrainerbike *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->deviceDiscovered((*reinterpret_cast< const QBluetoothDeviceInfo(*)>(_a[1]))); break;
        case 3: _t->update(); break;
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
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (computrainerbike::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&computrainerbike::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (computrainerbike::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&computrainerbike::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject computrainerbike::staticMetaObject = { {
    QMetaObject::SuperData::link<bike::staticMetaObject>(),
    qt_meta_stringdata_computrainerbike.data,
    qt_meta_data_computrainerbike,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *computrainerbike::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *computrainerbike::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_computrainerbike.stringdata0))
        return static_cast<void*>(this);
    return bike::qt_metacast(_clname);
}

int computrainerbike::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = bike::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void computrainerbike::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void computrainerbike::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
