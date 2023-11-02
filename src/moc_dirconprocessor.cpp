/****************************************************************************
** Meta object code from reading C++ file 'dirconprocessor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "dirconprocessor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'dirconprocessor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_DirconProcessor_t {
    QByteArrayData data[11];
    char stringdata0[165];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DirconProcessor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DirconProcessor_t qt_meta_stringdata_DirconProcessor = {
    {
QT_MOC_LITERAL(0, 0, 15), // "DirconProcessor"
QT_MOC_LITERAL(1, 16, 20), // "onCharacteristicRead"
QT_MOC_LITERAL(2, 37, 0), // ""
QT_MOC_LITERAL(3, 38, 4), // "uuid"
QT_MOC_LITERAL(4, 43, 21), // "onCharacteristicWrite"
QT_MOC_LITERAL(5, 65, 4), // "data"
QT_MOC_LITERAL(6, 70, 34), // "onCharacteristicNotificationS..."
QT_MOC_LITERAL(7, 105, 9), // "switchval"
QT_MOC_LITERAL(8, 115, 16), // "tcpDataAvailable"
QT_MOC_LITERAL(9, 132, 15), // "tcpDisconnected"
QT_MOC_LITERAL(10, 148, 16) // "tcpNewConnection"

    },
    "DirconProcessor\0onCharacteristicRead\0"
    "\0uuid\0onCharacteristicWrite\0data\0"
    "onCharacteristicNotificationSwitch\0"
    "switchval\0tcpDataAvailable\0tcpDisconnected\0"
    "tcpNewConnection"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DirconProcessor[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,    2,   47,    2, 0x06 /* Public */,
       6,    2,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    0,   57,    2, 0x08 /* Private */,
       9,    0,   58,    2, 0x08 /* Private */,
      10,    0,   59,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::UShort,    3,
    QMetaType::Void, QMetaType::UShort, QMetaType::QByteArray,    3,    5,
    QMetaType::Void, QMetaType::UShort, QMetaType::Char,    3,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void DirconProcessor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DirconProcessor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onCharacteristicRead((*reinterpret_cast< quint16(*)>(_a[1]))); break;
        case 1: _t->onCharacteristicWrite((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< QByteArray(*)>(_a[2]))); break;
        case 2: _t->onCharacteristicNotificationSwitch((*reinterpret_cast< quint16(*)>(_a[1])),(*reinterpret_cast< char(*)>(_a[2]))); break;
        case 3: _t->tcpDataAvailable(); break;
        case 4: _t->tcpDisconnected(); break;
        case 5: _t->tcpNewConnection(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DirconProcessor::*)(quint16 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DirconProcessor::onCharacteristicRead)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DirconProcessor::*)(quint16 , QByteArray );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DirconProcessor::onCharacteristicWrite)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DirconProcessor::*)(quint16 , char );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&DirconProcessor::onCharacteristicNotificationSwitch)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject DirconProcessor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_DirconProcessor.data,
    qt_meta_data_DirconProcessor,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *DirconProcessor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DirconProcessor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_DirconProcessor.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DirconProcessor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void DirconProcessor::onCharacteristicRead(quint16 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DirconProcessor::onCharacteristicWrite(quint16 _t1, QByteArray _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DirconProcessor::onCharacteristicNotificationSwitch(quint16 _t1, char _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
