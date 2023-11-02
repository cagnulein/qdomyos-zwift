/****************************************************************************
** Meta object code from reading C++ file 'inappstoreqmltype.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "purchasing/qmltypes/inappstoreqmltype.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'inappstoreqmltype.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_InAppStoreQmlType_t {
    QByteArrayData data[5];
    char stringdata0[54];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_InAppStoreQmlType_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_InAppStoreQmlType_t qt_meta_stringdata_InAppStoreQmlType = {
    {
QT_MOC_LITERAL(0, 0, 17), // "InAppStoreQmlType"
QT_MOC_LITERAL(1, 18, 11), // "QML.Element"
QT_MOC_LITERAL(2, 30, 5), // "Store"
QT_MOC_LITERAL(3, 36, 16), // "restorePurchases"
QT_MOC_LITERAL(4, 53, 0) // ""

    },
    "InAppStoreQmlType\0QML.Element\0Store\0"
    "restorePurchases\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_InAppStoreQmlType[] = {

 // content:
       8,       // revision
       0,       // classname
       1,   14, // classinfo
       1,   16, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // classinfo: key, value
       1,    2,

 // methods: name, argc, parameters, tag, flags
       3,    0,   21,    4, 0x02 /* Public */,

 // methods: parameters
    QMetaType::Void,

       0        // eod
};

void InAppStoreQmlType::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<InAppStoreQmlType *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->restorePurchases(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

QT_INIT_METAOBJECT const QMetaObject InAppStoreQmlType::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_InAppStoreQmlType.data,
    qt_meta_data_InAppStoreQmlType,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *InAppStoreQmlType::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *InAppStoreQmlType::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_InAppStoreQmlType.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int InAppStoreQmlType::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
