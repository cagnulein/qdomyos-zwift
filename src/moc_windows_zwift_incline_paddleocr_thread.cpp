/****************************************************************************
** Meta object code from reading C++ file 'windows_zwift_incline_paddleocr_thread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "windows_zwift_incline_paddleocr_thread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'windows_zwift_incline_paddleocr_thread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_windows_zwift_incline_paddleocr_thread_t {
    QByteArrayData data[7];
    char stringdata0[85];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_windows_zwift_incline_paddleocr_thread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_windows_zwift_incline_paddleocr_thread_t qt_meta_stringdata_windows_zwift_incline_paddleocr_thread = {
    {
QT_MOC_LITERAL(0, 0, 38), // "windows_zwift_incline_paddleo..."
QT_MOC_LITERAL(1, 39, 13), // "onInclination"
QT_MOC_LITERAL(2, 53, 0), // ""
QT_MOC_LITERAL(3, 54, 11), // "inclination"
QT_MOC_LITERAL(4, 66, 5), // "grade"
QT_MOC_LITERAL(5, 72, 5), // "debug"
QT_MOC_LITERAL(6, 78, 6) // "string"

    },
    "windows_zwift_incline_paddleocr_thread\0"
    "onInclination\0\0inclination\0grade\0debug\0"
    "string"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_windows_zwift_incline_paddleocr_thread[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x06 /* Public */,
       5,    1,   29,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    3,    4,
    QMetaType::Void, QMetaType::QString,    6,

       0        // eod
};

void windows_zwift_incline_paddleocr_thread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<windows_zwift_incline_paddleocr_thread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (windows_zwift_incline_paddleocr_thread::*)(double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&windows_zwift_incline_paddleocr_thread::onInclination)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (windows_zwift_incline_paddleocr_thread::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&windows_zwift_incline_paddleocr_thread::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject windows_zwift_incline_paddleocr_thread::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_windows_zwift_incline_paddleocr_thread.data,
    qt_meta_data_windows_zwift_incline_paddleocr_thread,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *windows_zwift_incline_paddleocr_thread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *windows_zwift_incline_paddleocr_thread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_windows_zwift_incline_paddleocr_thread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int windows_zwift_incline_paddleocr_thread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
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
    return _id;
}

// SIGNAL 0
void windows_zwift_incline_paddleocr_thread::onInclination(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void windows_zwift_incline_paddleocr_thread::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
