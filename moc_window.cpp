/****************************************************************************
** Meta object code from reading C++ file 'window.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.7)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "window.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'window.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.7. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Window[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      20,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x0a,
      33,   28,    7,    7, 0x0a,
      58,   51,    7,    7, 0x0a,
      75,    7,    7,    7, 0x0a,
      91,    7,    7,    7, 0x0a,
     107,    7,    7,    7, 0x0a,
     115,    7,    7,    7, 0x0a,
     132,  124,    7,    7, 0x0a,
     153,    7,    7,    7, 0x0a,
     163,    7,    7,    7, 0x0a,
     173,    7,    7,    7, 0x0a,
     190,    7,    7,    7, 0x0a,
     207,    7,    7,    7, 0x0a,
     235,  228,    7,    7, 0x0a,
     272,  266,    7,    7, 0x0a,
     305,  299,    7,    7, 0x0a,
     342,  332,    7,    7, 0x0a,
     373,    7,    7,    7, 0x0a,
     393,    7,    7,    7, 0x0a,
     409,    7,    7,    7, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Window[] = {
    "Window\0\0lineReceivedFocus()\0link\0"
    "linkClicked(QUrl)\0tabNum\0tabChanged(uint)\0"
    "switchToTiles()\0switchToBrush()\0pause()\0"
    "resume()\0message\0displayError(string)\0"
    "setXCen()\0setYCen()\0setPacketSpeed()\0"
    "setPacketAngle()\0setPacketPrecision()\0"
    "center\0packetCenterChanged(QVector3D)\0"
    "speed\0packetSpeedChanged(double)\0angle\0"
    "packetAngleChanged(double)\0precision\0"
    "packetPrecisionChanged(double)\0"
    "simulationStarted()\0resetOccurred()\0"
    "updateInfo()\0"
};

void Window::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Window *_t = static_cast<Window *>(_o);
        switch (_id) {
        case 0: _t->lineReceivedFocus(); break;
        case 1: _t->linkClicked((*reinterpret_cast< const QUrl(*)>(_a[1]))); break;
        case 2: _t->tabChanged((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 3: _t->switchToTiles(); break;
        case 4: _t->switchToBrush(); break;
        case 5: _t->pause(); break;
        case 6: _t->resume(); break;
        case 7: _t->displayError((*reinterpret_cast< string(*)>(_a[1]))); break;
        case 8: _t->setXCen(); break;
        case 9: _t->setYCen(); break;
        case 10: _t->setPacketSpeed(); break;
        case 11: _t->setPacketAngle(); break;
        case 12: _t->setPacketPrecision(); break;
        case 13: _t->packetCenterChanged((*reinterpret_cast< QVector3D(*)>(_a[1]))); break;
        case 14: _t->packetSpeedChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 15: _t->packetAngleChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 16: _t->packetPrecisionChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 17: _t->simulationStarted(); break;
        case 18: _t->resetOccurred(); break;
        case 19: _t->updateInfo(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Window::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Window::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Window,
      qt_meta_data_Window, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Window::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Window::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Window::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Window))
        return static_cast<void*>(const_cast< Window*>(this));
    return QWidget::qt_metacast(_clname);
}

int Window::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 20)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 20;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
