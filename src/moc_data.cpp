/****************************************************************************
** Meta object code from reading C++ file 'data.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "data.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'data.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Data[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: signature, parameters, type, tag, flags
       6,    5,    5,    5, 0x05,
      26,    5,    5,    5, 0x05,
      42,    5,    5,    5, 0x05,
      62,    5,    5,    5, 0x05,
      87,    5,    5,    5, 0x05,
     111,    5,    5,    5, 0x05,
     130,    5,    5,    5, 0x05,

       0        // eod
};

static const char qt_meta_stringdata_Data[] = {
    "Data\0\0updateGridBuffers()\0updateTileVbo()\0"
    "updateMeshBuffers()\0updateIndicatorBuffers()\0"
    "updateParticleBuffers()\0updateNetBuffers()\0"
    "updateTileEbo()\0"
};

void Data::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Data *_t = static_cast<Data *>(_o);
        switch (_id) {
        case 0: _t->updateGridBuffers(); break;
        case 1: _t->updateTileVbo(); break;
        case 2: _t->updateMeshBuffers(); break;
        case 3: _t->updateIndicatorBuffers(); break;
        case 4: _t->updateParticleBuffers(); break;
        case 5: _t->updateNetBuffers(); break;
        case 6: _t->updateTileEbo(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Data::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Data::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Data,
      qt_meta_data_Data, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Data::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Data::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Data::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Data))
        return static_cast<void*>(const_cast< Data*>(this));
    return QObject::qt_metacast(_clname);
}

int Data::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void Data::updateGridBuffers()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void Data::updateTileVbo()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void Data::updateMeshBuffers()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}

// SIGNAL 3
void Data::updateIndicatorBuffers()
{
    QMetaObject::activate(this, &staticMetaObject, 3, 0);
}

// SIGNAL 4
void Data::updateParticleBuffers()
{
    QMetaObject::activate(this, &staticMetaObject, 4, 0);
}

// SIGNAL 5
void Data::updateNetBuffers()
{
    QMetaObject::activate(this, &staticMetaObject, 5, 0);
}

// SIGNAL 6
void Data::updateTileEbo()
{
    QMetaObject::activate(this, &staticMetaObject, 6, 0);
}
QT_END_MOC_NAMESPACE
