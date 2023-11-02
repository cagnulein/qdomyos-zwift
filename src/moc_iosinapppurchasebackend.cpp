/****************************************************************************
** Meta object code from reading C++ file 'iosinapppurchasebackend.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "purchasing/ios/iosinapppurchasebackend.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'iosinapppurchasebackend.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_IosInAppPurchaseBackend_t {
    QByteArrayData data[12];
    char stringdata0[192];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IosInAppPurchaseBackend_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IosInAppPurchaseBackend_t qt_meta_stringdata_IosInAppPurchaseBackend = {
    {
QT_MOC_LITERAL(0, 0, 23), // "IosInAppPurchaseBackend"
QT_MOC_LITERAL(1, 24, 18), // "setParentToBackend"
QT_MOC_LITERAL(2, 43, 0), // ""
QT_MOC_LITERAL(3, 44, 6), // "object"
QT_MOC_LITERAL(4, 51, 15), // "registerProduct"
QT_MOC_LITERAL(5, 67, 24), // "IosInAppPurchaseProduct*"
QT_MOC_LITERAL(6, 92, 7), // "product"
QT_MOC_LITERAL(7, 100, 20), // "registerQueryFailure"
QT_MOC_LITERAL(8, 121, 9), // "productId"
QT_MOC_LITERAL(9, 131, 19), // "registerTransaction"
QT_MOC_LITERAL(10, 151, 28), // "IosInAppPurchaseTransaction*"
QT_MOC_LITERAL(11, 180, 11) // "transaction"

    },
    "IosInAppPurchaseBackend\0setParentToBackend\0"
    "\0object\0registerProduct\0"
    "IosInAppPurchaseProduct*\0product\0"
    "registerQueryFailure\0productId\0"
    "registerTransaction\0IosInAppPurchaseTransaction*\0"
    "transaction"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IosInAppPurchaseBackend[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
       4,    1,   37,    2, 0x02 /* Public */,
       7,    1,   40,    2, 0x02 /* Public */,
       9,    1,   43,    2, 0x02 /* Public */,

 // slots: parameters
    QMetaType::Void, QMetaType::QObjectStar,    3,

 // methods: parameters
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, 0x80000000 | 10,   11,

       0        // eod
};

void IosInAppPurchaseBackend::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IosInAppPurchaseBackend *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setParentToBackend((*reinterpret_cast< QObject*(*)>(_a[1]))); break;
        case 1: _t->registerProduct((*reinterpret_cast< IosInAppPurchaseProduct*(*)>(_a[1]))); break;
        case 2: _t->registerQueryFailure((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 3: _t->registerTransaction((*reinterpret_cast< IosInAppPurchaseTransaction*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject IosInAppPurchaseBackend::staticMetaObject = { {
    QMetaObject::SuperData::link<InAppPurchaseBackend::staticMetaObject>(),
    qt_meta_stringdata_IosInAppPurchaseBackend.data,
    qt_meta_data_IosInAppPurchaseBackend,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IosInAppPurchaseBackend::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IosInAppPurchaseBackend::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IosInAppPurchaseBackend.stringdata0))
        return static_cast<void*>(this);
    return InAppPurchaseBackend::qt_metacast(_clname);
}

int IosInAppPurchaseBackend::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = InAppPurchaseBackend::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
