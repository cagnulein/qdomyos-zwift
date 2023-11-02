/****************************************************************************
** Meta object code from reading C++ file 'elliptical.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "elliptical.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'elliptical.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_elliptical_t {
    QByteArrayData data[17];
    char stringdata0[189];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_elliptical_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_elliptical_t qt_meta_stringdata_elliptical = {
    {
QT_MOC_LITERAL(0, 0, 10), // "elliptical"
QT_MOC_LITERAL(1, 11, 11), // "bikeStarted"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 11), // "changeSpeed"
QT_MOC_LITERAL(4, 36, 5), // "speed"
QT_MOC_LITERAL(5, 42, 16), // "changeResistance"
QT_MOC_LITERAL(6, 59, 12), // "resistance_t"
QT_MOC_LITERAL(7, 72, 3), // "res"
QT_MOC_LITERAL(8, 76, 17), // "changeInclination"
QT_MOC_LITERAL(9, 94, 5), // "grade"
QT_MOC_LITERAL(10, 100, 11), // "inclination"
QT_MOC_LITERAL(11, 112, 13), // "changeCadence"
QT_MOC_LITERAL(12, 126, 7), // "int16_t"
QT_MOC_LITERAL(13, 134, 3), // "cad"
QT_MOC_LITERAL(14, 138, 32), // "changeRequestedPelotonResistance"
QT_MOC_LITERAL(15, 171, 6), // "int8_t"
QT_MOC_LITERAL(16, 178, 10) // "resistance"

    },
    "elliptical\0bikeStarted\0\0changeSpeed\0"
    "speed\0changeResistance\0resistance_t\0"
    "res\0changeInclination\0grade\0inclination\0"
    "changeCadence\0int16_t\0cad\0"
    "changeRequestedPelotonResistance\0"
    "int8_t\0resistance"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_elliptical[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   45,    2, 0x0a /* Public */,
       5,    1,   48,    2, 0x0a /* Public */,
       8,    2,   51,    2, 0x0a /* Public */,
      11,    1,   56,    2, 0x0a /* Public */,
      14,    1,   59,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    4,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    9,   10,
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void, 0x80000000 | 15,   16,

       0        // eod
};

void elliptical::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<elliptical *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->bikeStarted(); break;
        case 1: _t->changeSpeed((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 2: _t->changeResistance((*reinterpret_cast< resistance_t(*)>(_a[1]))); break;
        case 3: _t->changeInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 4: _t->changeCadence((*reinterpret_cast< int16_t(*)>(_a[1]))); break;
        case 5: _t->changeRequestedPelotonResistance((*reinterpret_cast< int8_t(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (elliptical::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&elliptical::bikeStarted)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject elliptical::staticMetaObject = { {
    QMetaObject::SuperData::link<bluetoothdevice::staticMetaObject>(),
    qt_meta_stringdata_elliptical.data,
    qt_meta_data_elliptical,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *elliptical::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *elliptical::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_elliptical.stringdata0))
        return static_cast<void*>(this);
    return bluetoothdevice::qt_metacast(_clname);
}

int elliptical::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = bluetoothdevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void elliptical::bikeStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
