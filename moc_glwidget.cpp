/****************************************************************************
** Meta object code from reading C++ file 'glwidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "glwidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'glwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GLWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      38,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x05,
      31,    9,    9,    9, 0x05,
      55,   47,    9,    9, 0x05,
      76,   69,    9,    9, 0x05,
     107,  101,    9,    9, 0x05,
     134,  128,    9,    9, 0x05,
     165,  155,    9,    9, 0x05,
     190,    9,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
     214,    9,    9,    9, 0x0a,
     224,    9,    9,    9, 0x0a,
     242,    9,    9,    9, 0x0a,
     256,    9,    9,    9, 0x0a,
     283,  278,    9,    9, 0x0a,
     308,  301,    9,    9, 0x0a,
     328,  101,    9,    9, 0x0a,
     358,  346,    9,    9, 0x0a,
     392,  379,    9,    9, 0x0a,
     418,  413,    9,    9, 0x2a,
     447,  434,    9,    9, 0x0a,
     473,  468,    9,    9, 0x2a,
     503,  489,    9,    9, 0x0a,
     531,  101,    9,    9, 0x2a,
     568,  554,    9,    9, 0x0a,
     596,  128,    9,    9, 0x2a,
     637,  619,    9,    9, 0x0a,
     669,  155,    9,    9, 0x2a,
     703,  696,    9,    9, 0x0a,
     723,    9,    9,    9, 0x0a,
     743,  737,    9,    9, 0x0a,
     768,    9,    9,    9, 0x0a,
     785,    9,    9,    9, 0x0a,
     800,    9,    9,    9, 0x0a,
     820,    9,    9,    9, 0x0a,
     836,    9,    9,    9, 0x0a,
     861,    9,    9,    9, 0x0a,
     885,    9,    9,    9, 0x0a,
     904,    9,    9,    9, 0x0a,
     924,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GLWidget[] = {
    "GLWidget\0\0startingSimulation()\0"
    "resetOccurred()\0message\0error(string)\0"
    "center\0updatedCenter(QVector3D)\0speed\0"
    "updatedSpeed(double)\0angle\0"
    "updatedAngle(double)\0precision\0"
    "updatedPrecision(double)\0"
    "observingWavefunction()\0observe()\0"
    "resetSimulation()\0toggleBrush()\0"
    "toggleDrawClassical()\0size\0setBrushSize(int)\0"
    "height\0setBrushHeight(int)\0setSimSpeed(uint)\0"
    "sensitivity\0setSensitivity(uint)\0"
    "xCen,preview\0setXCen(double,bool)\0"
    "xCen\0setXCen(double)\0yCen,preview\0"
    "setYCen(double,bool)\0yCen\0setYCen(double)\0"
    "speed,preview\0setPacketSpeed(double,bool)\0"
    "setPacketSpeed(double)\0angle,preview\0"
    "setPacketAngle(double,bool)\0"
    "setPacketAngle(double)\0precision,preview\0"
    "setPacketPrecision(double,bool)\0"
    "setPacketPrecision(double)\0tabNum\0"
    "setHoverState(uint)\0togglePause()\0"
    "pause\0setSimulationState(bool)\0"
    "refreshRequest()\0brushRequest()\0"
    "updateGridBuffers()\0updateTileVbo()\0"
    "updateIndicatorBuffers()\0"
    "updateParticleBuffers()\0updateNetBuffers()\0"
    "updateMeshBuffers()\0updateTileEbo()\0"
};

void GLWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GLWidget *_t = static_cast<GLWidget *>(_o);
        switch (_id) {
        case 0: _t->startingSimulation(); break;
        case 1: _t->resetOccurred(); break;
        case 2: _t->error((*reinterpret_cast< string(*)>(_a[1]))); break;
        case 3: _t->updatedCenter((*reinterpret_cast< QVector3D(*)>(_a[1]))); break;
        case 4: _t->updatedSpeed((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->updatedAngle((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->updatedPrecision((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->observingWavefunction(); break;
        case 8: _t->observe(); break;
        case 9: _t->resetSimulation(); break;
        case 10: _t->toggleBrush(); break;
        case 11: _t->toggleDrawClassical(); break;
        case 12: _t->setBrushSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->setBrushHeight((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->setSimSpeed((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 15: _t->setSensitivity((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 16: _t->setXCen((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 17: _t->setXCen((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 18: _t->setYCen((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 19: _t->setYCen((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 20: _t->setPacketSpeed((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 21: _t->setPacketSpeed((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 22: _t->setPacketAngle((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 23: _t->setPacketAngle((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 24: _t->setPacketPrecision((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 25: _t->setPacketPrecision((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 26: _t->setHoverState((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 27: _t->togglePause(); break;
        case 28: _t->setSimulationState((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 29: _t->refreshRequest(); break;
        case 30: _t->brushRequest(); break;
        case 31: _t->updateGridBuffers(); break;
        case 32: _t->updateTileVbo(); break;
        case 33: _t->updateIndicatorBuffers(); break;
        case 34: _t->updateParticleBuffers(); break;
        case 35: _t->updateNetBuffers(); break;
        case 36: _t->updateMeshBuffers(); break;
        case 37: _t->updateTileEbo(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GLWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GLWidget::staticMetaObject = {
    { &QOpenGLWidget::staticMetaObject, qt_meta_stringdata_GLWidget,
      qt_meta_data_GLWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GLWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GLWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GLWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GLWidget))
        return static_cast<void*>(const_cast< GLWidget*>(this));
    if (!strcmp(_clname, "QOpenGLFunctions"))
        return static_cast< QOpenGLFunctions*>(const_cast< GLWidget*>(this));
    return QOpenGLWidget::qt_metacast(_clname);
}

int GLWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QOpenGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 38)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 38;
    }
    return _id;
}

// SIGNAL 0
void GLWidget::startingSimulation()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void GLWidget::resetOccurred()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void GLWidget::error(string _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void GLWidget::updatedCenter(QVector3D _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void GLWidget::updatedSpeed(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void GLWidget::updatedAngle(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void GLWidget::updatedPrecision(double _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void GLWidget::observingWavefunction()
{
    QMetaObject::activate(this, &staticMetaObject, 7, 0);
}
QT_END_MOC_NAMESPACE
