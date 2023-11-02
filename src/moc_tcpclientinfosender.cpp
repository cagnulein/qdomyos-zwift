/****************************************************************************
** Meta object code from reading C++ file 'tcpclientinfosender.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "tcpclientinfosender.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tcpclientinfosender.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_TcpClientInfoSender_t {
    QByteArrayData data[9];
    char stringdata0[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TcpClientInfoSender_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TcpClientInfoSender_t qt_meta_stringdata_TcpClientInfoSender = {
    {
QT_MOC_LITERAL(0, 0, 19), // "TcpClientInfoSender"
QT_MOC_LITERAL(1, 20, 9), // "readyRead"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 14), // "debugConnected"
QT_MOC_LITERAL(4, 46, 11), // "socketError"
QT_MOC_LITERAL(5, 58, 3), // "err"
QT_MOC_LITERAL(6, 62, 12), // "stateChanged"
QT_MOC_LITERAL(7, 75, 28), // "QAbstractSocket::SocketState"
QT_MOC_LITERAL(8, 104, 11) // "socketState"

    },
    "TcpClientInfoSender\0readyRead\0\0"
    "debugConnected\0socketError\0err\0"
    "stateChanged\0QAbstractSocket::SocketState\0"
    "socketState"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TcpClientInfoSender[] = {

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
       1,    0,   34,    2, 0x08 /* Private */,
       3,    0,   35,    2, 0x08 /* Private */,
       4,    1,   36,    2, 0x08 /* Private */,
       6,    1,   39,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, 0x80000000 | 7,    8,

       0        // eod
};

void TcpClientInfoSender::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TcpClientInfoSender *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->readyRead(); break;
        case 1: _t->debugConnected(); break;
        case 2: _t->socketError((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->stateChanged((*reinterpret_cast< QAbstractSocket::SocketState(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractSocket::SocketState >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject TcpClientInfoSender::staticMetaObject = { {
    QMetaObject::SuperData::link<TemplateInfoSender::staticMetaObject>(),
    qt_meta_stringdata_TcpClientInfoSender.data,
    qt_meta_data_TcpClientInfoSender,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *TcpClientInfoSender::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TcpClientInfoSender::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TcpClientInfoSender.stringdata0))
        return static_cast<void*>(this);
    return TemplateInfoSender::qt_metacast(_clname);
}

int TcpClientInfoSender::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = TemplateInfoSender::qt_metacall(_c, _id, _a);
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
