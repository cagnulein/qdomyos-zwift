/****************************************************************************
** Meta object code from reading C++ file 'inapptransaction.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "purchasing/inapp/inapptransaction.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'inapptransaction.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_InAppTransaction_t {
    QByteArrayData data[25];
    char stringdata0[315];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_InAppTransaction_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_InAppTransaction_t qt_meta_stringdata_InAppTransaction = {
    {
QT_MOC_LITERAL(0, 0, 16), // "InAppTransaction"
QT_MOC_LITERAL(1, 17, 11), // "QML.Element"
QT_MOC_LITERAL(2, 29, 11), // "Transaction"
QT_MOC_LITERAL(3, 41, 11), // "QML.Foreign"
QT_MOC_LITERAL(4, 53, 20), // "QQmlTypeNotAvailable"
QT_MOC_LITERAL(5, 74, 8), // "finalize"
QT_MOC_LITERAL(6, 83, 0), // ""
QT_MOC_LITERAL(7, 84, 16), // "platformProperty"
QT_MOC_LITERAL(8, 101, 12), // "propertyName"
QT_MOC_LITERAL(9, 114, 6), // "status"
QT_MOC_LITERAL(10, 121, 17), // "TransactionStatus"
QT_MOC_LITERAL(11, 139, 7), // "product"
QT_MOC_LITERAL(12, 147, 13), // "InAppProduct*"
QT_MOC_LITERAL(13, 161, 7), // "orderId"
QT_MOC_LITERAL(14, 169, 13), // "failureReason"
QT_MOC_LITERAL(15, 183, 13), // "FailureReason"
QT_MOC_LITERAL(16, 197, 11), // "errorString"
QT_MOC_LITERAL(17, 209, 9), // "timestamp"
QT_MOC_LITERAL(18, 219, 7), // "Unknown"
QT_MOC_LITERAL(19, 227, 16), // "PurchaseApproved"
QT_MOC_LITERAL(20, 244, 14), // "PurchaseFailed"
QT_MOC_LITERAL(21, 259, 16), // "PurchaseRestored"
QT_MOC_LITERAL(22, 276, 9), // "NoFailure"
QT_MOC_LITERAL(23, 286, 14), // "CanceledByUser"
QT_MOC_LITERAL(24, 301, 13) // "ErrorOccurred"

    },
    "InAppTransaction\0QML.Element\0Transaction\0"
    "QML.Foreign\0QQmlTypeNotAvailable\0"
    "finalize\0\0platformProperty\0propertyName\0"
    "status\0TransactionStatus\0product\0"
    "InAppProduct*\0orderId\0failureReason\0"
    "FailureReason\0errorString\0timestamp\0"
    "Unknown\0PurchaseApproved\0PurchaseFailed\0"
    "PurchaseRestored\0NoFailure\0CanceledByUser\0"
    "ErrorOccurred"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_InAppTransaction[] = {

 // content:
       8,       // revision
       0,       // classname
       2,   14, // classinfo
       2,   18, // methods
       6,   32, // properties
       2,   50, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // classinfo: key, value
       1,    2,
       3,    4,

 // methods: name, argc, parameters, tag, flags
       5,    0,   28,    6, 0x02 /* Public */,
       7,    1,   29,    6, 0x02 /* Public */,

 // methods: parameters
    QMetaType::Void,
    QMetaType::QString, QMetaType::QString,    8,

 // properties: name, type, flags
       9, 0x80000000 | 10, 0x00095409,
      11, 0x80000000 | 12, 0x00095409,
      13, QMetaType::QString, 0x00095401,
      14, 0x80000000 | 15, 0x00095409,
      16, QMetaType::QString, 0x00095401,
      17, QMetaType::QDateTime, 0x00095401,

 // enums: name, alias, flags, count, data
      10,   10, 0x0,    4,   60,
      15,   15, 0x0,    3,   68,

 // enum data: key, value
      18, uint(InAppTransaction::Unknown),
      19, uint(InAppTransaction::PurchaseApproved),
      20, uint(InAppTransaction::PurchaseFailed),
      21, uint(InAppTransaction::PurchaseRestored),
      22, uint(InAppTransaction::NoFailure),
      23, uint(InAppTransaction::CanceledByUser),
      24, uint(InAppTransaction::ErrorOccurred),

       0        // eod
};

void InAppTransaction::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<InAppTransaction *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->finalize(); break;
        case 1: { QString _r = _t->platformProperty((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterPropertyMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 1:
            *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< InAppProduct* >(); break;
        }
    }

#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<InAppTransaction *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< TransactionStatus*>(_v) = _t->status(); break;
        case 1: *reinterpret_cast< InAppProduct**>(_v) = _t->product(); break;
        case 2: *reinterpret_cast< QString*>(_v) = _t->orderId(); break;
        case 3: *reinterpret_cast< FailureReason*>(_v) = _t->failureReason(); break;
        case 4: *reinterpret_cast< QString*>(_v) = _t->errorString(); break;
        case 5: *reinterpret_cast< QDateTime*>(_v) = _t->timestamp(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject InAppTransaction::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_InAppTransaction.data,
    qt_meta_data_InAppTransaction,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *InAppTransaction::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *InAppTransaction::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_InAppTransaction.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int InAppTransaction::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 6;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
