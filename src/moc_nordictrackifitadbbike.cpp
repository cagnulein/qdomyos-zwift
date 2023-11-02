/****************************************************************************
** Meta object code from reading C++ file 'nordictrackifitadbbike.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "nordictrackifitadbbike.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'nordictrackifitadbbike.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_nordictrackifitadbbikeLogcatAdbThread_t {
    QByteArrayData data[11];
    char stringdata0[112];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_nordictrackifitadbbikeLogcatAdbThread_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_nordictrackifitadbbikeLogcatAdbThread_t qt_meta_stringdata_nordictrackifitadbbikeLogcatAdbThread = {
    {
QT_MOC_LITERAL(0, 0, 37), // "nordictrackifitadbbikeLogcatA..."
QT_MOC_LITERAL(1, 38, 18), // "onSpeedInclination"
QT_MOC_LITERAL(2, 57, 0), // ""
QT_MOC_LITERAL(3, 58, 5), // "speed"
QT_MOC_LITERAL(4, 64, 11), // "inclination"
QT_MOC_LITERAL(5, 76, 5), // "debug"
QT_MOC_LITERAL(6, 82, 7), // "message"
QT_MOC_LITERAL(7, 90, 6), // "onWatt"
QT_MOC_LITERAL(8, 97, 4), // "watt"
QT_MOC_LITERAL(9, 102, 5), // "onHRM"
QT_MOC_LITERAL(10, 108, 3) // "hrm"

    },
    "nordictrackifitadbbikeLogcatAdbThread\0"
    "onSpeedInclination\0\0speed\0inclination\0"
    "debug\0message\0onWatt\0watt\0onHRM\0hrm"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_nordictrackifitadbbikeLogcatAdbThread[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   34,    2, 0x06 /* Public */,
       5,    1,   39,    2, 0x06 /* Public */,
       7,    1,   42,    2, 0x06 /* Public */,
       9,    1,   45,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::Double,    8,
    QMetaType::Void, QMetaType::Int,   10,

       0        // eod
};

void nordictrackifitadbbikeLogcatAdbThread::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<nordictrackifitadbbikeLogcatAdbThread *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->onSpeedInclination((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->onWatt((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 3: _t->onHRM((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (nordictrackifitadbbikeLogcatAdbThread::*)(double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&nordictrackifitadbbikeLogcatAdbThread::onSpeedInclination)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (nordictrackifitadbbikeLogcatAdbThread::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&nordictrackifitadbbikeLogcatAdbThread::debug)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (nordictrackifitadbbikeLogcatAdbThread::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&nordictrackifitadbbikeLogcatAdbThread::onWatt)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (nordictrackifitadbbikeLogcatAdbThread::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&nordictrackifitadbbikeLogcatAdbThread::onHRM)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject nordictrackifitadbbikeLogcatAdbThread::staticMetaObject = { {
    QMetaObject::SuperData::link<QThread::staticMetaObject>(),
    qt_meta_stringdata_nordictrackifitadbbikeLogcatAdbThread.data,
    qt_meta_data_nordictrackifitadbbikeLogcatAdbThread,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *nordictrackifitadbbikeLogcatAdbThread::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *nordictrackifitadbbikeLogcatAdbThread::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_nordictrackifitadbbikeLogcatAdbThread.stringdata0))
        return static_cast<void*>(this);
    return QThread::qt_metacast(_clname);
}

int nordictrackifitadbbikeLogcatAdbThread::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void nordictrackifitadbbikeLogcatAdbThread::onSpeedInclination(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void nordictrackifitadbbikeLogcatAdbThread::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void nordictrackifitadbbikeLogcatAdbThread::onWatt(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void nordictrackifitadbbikeLogcatAdbThread::onHRM(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
struct qt_meta_stringdata_nordictrackifitadbbike_t {
    QByteArrayData data[12];
    char stringdata0[135];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_nordictrackifitadbbike_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_nordictrackifitadbbike_t qt_meta_stringdata_nordictrackifitadbbike = {
    {
QT_MOC_LITERAL(0, 0, 22), // "nordictrackifitadbbike"
QT_MOC_LITERAL(1, 23, 12), // "disconnected"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 5), // "debug"
QT_MOC_LITERAL(4, 43, 6), // "string"
QT_MOC_LITERAL(5, 50, 23), // "processPendingDatagrams"
QT_MOC_LITERAL(6, 74, 26), // "changeInclinationRequested"
QT_MOC_LITERAL(7, 101, 5), // "grade"
QT_MOC_LITERAL(8, 107, 10), // "percentage"
QT_MOC_LITERAL(9, 118, 5), // "onHRM"
QT_MOC_LITERAL(10, 124, 3), // "hrm"
QT_MOC_LITERAL(11, 128, 6) // "update"

    },
    "nordictrackifitadbbike\0disconnected\0"
    "\0debug\0string\0processPendingDatagrams\0"
    "changeInclinationRequested\0grade\0"
    "percentage\0onHRM\0hrm\0update"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_nordictrackifitadbbike[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    1,   45,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   48,    2, 0x08 /* Private */,
       6,    2,   49,    2, 0x08 /* Private */,
       9,    1,   54,    2, 0x08 /* Private */,
      11,    0,   57,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    7,    8,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void,

       0        // eod
};

void nordictrackifitadbbike::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<nordictrackifitadbbike *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->disconnected(); break;
        case 1: _t->debug((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->processPendingDatagrams(); break;
        case 3: _t->changeInclinationRequested((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 4: _t->onHRM((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->update(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (nordictrackifitadbbike::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&nordictrackifitadbbike::disconnected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (nordictrackifitadbbike::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&nordictrackifitadbbike::debug)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject nordictrackifitadbbike::staticMetaObject = { {
    QMetaObject::SuperData::link<bike::staticMetaObject>(),
    qt_meta_stringdata_nordictrackifitadbbike.data,
    qt_meta_data_nordictrackifitadbbike,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *nordictrackifitadbbike::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *nordictrackifitadbbike::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_nordictrackifitadbbike.stringdata0))
        return static_cast<void*>(this);
    return bike::qt_metacast(_clname);
}

int nordictrackifitadbbike::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = bike::qt_metacall(_c, _id, _a);
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
void nordictrackifitadbbike::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void nordictrackifitadbbike::debug(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
