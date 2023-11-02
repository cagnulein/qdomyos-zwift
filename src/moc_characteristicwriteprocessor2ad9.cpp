/****************************************************************************
** Meta object code from reading C++ file 'characteristicwriteprocessor2ad9.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "characteristicwriteprocessor2ad9.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'characteristicwriteprocessor2ad9.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_CharacteristicWriteProcessor2AD9_t {
    QByteArrayData data[6];
    char stringdata0[109];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CharacteristicWriteProcessor2AD9_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CharacteristicWriteProcessor2AD9_t qt_meta_stringdata_CharacteristicWriteProcessor2AD9 = {
    {
QT_MOC_LITERAL(0, 0, 32), // "CharacteristicWriteProcessor2AD9"
QT_MOC_LITERAL(1, 33, 25), // "ftmsCharacteristicChanged"
QT_MOC_LITERAL(2, 59, 0), // ""
QT_MOC_LITERAL(3, 60, 24), // "QLowEnergyCharacteristic"
QT_MOC_LITERAL(4, 85, 14), // "characteristic"
QT_MOC_LITERAL(5, 100, 8) // "newValue"

    },
    "CharacteristicWriteProcessor2AD9\0"
    "ftmsCharacteristicChanged\0\0"
    "QLowEnergyCharacteristic\0characteristic\0"
    "newValue"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CharacteristicWriteProcessor2AD9[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,

       0        // eod
};

void CharacteristicWriteProcessor2AD9::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CharacteristicWriteProcessor2AD9 *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ftmsCharacteristicChanged((*reinterpret_cast< const QLowEnergyCharacteristic(*)>(_a[1])),(*reinterpret_cast< const QByteArray(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QLowEnergyCharacteristic >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CharacteristicWriteProcessor2AD9::*)(const QLowEnergyCharacteristic & , const QByteArray & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&CharacteristicWriteProcessor2AD9::ftmsCharacteristicChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject CharacteristicWriteProcessor2AD9::staticMetaObject = { {
    QMetaObject::SuperData::link<CharacteristicWriteProcessor::staticMetaObject>(),
    qt_meta_stringdata_CharacteristicWriteProcessor2AD9.data,
    qt_meta_data_CharacteristicWriteProcessor2AD9,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *CharacteristicWriteProcessor2AD9::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CharacteristicWriteProcessor2AD9::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CharacteristicWriteProcessor2AD9.stringdata0))
        return static_cast<void*>(this);
    return CharacteristicWriteProcessor::qt_metacast(_clname);
}

int CharacteristicWriteProcessor2AD9::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CharacteristicWriteProcessor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void CharacteristicWriteProcessor2AD9::ftmsCharacteristicChanged(const QLowEnergyCharacteristic & _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
