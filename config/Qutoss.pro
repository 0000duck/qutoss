QT += core
QT += gui
QT += widgets
QT += concurrent

ICON = circle2.icns

QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.8

#QMAKE_CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
#QMAKE_CFLAGS += -fsanitize=address -fno-omit-frame-pointer
#QMAKE_LFLAGS += -fsanitize=address

CONFIG += c++11

TARGET = Qutoss
CONFIG += console

TEMPLATE = app

SOURCES += main.cpp \
    glwidget.cpp \
    data.cpp \
    window.cpp \
    camera.cpp \
    ae/lmathx.c \
    ae/ae.c \
    equationparser.cpp \
    widgetaddons.cpp \
    helpers.cpp \
    tutorial.cpp \
    lua-5.3.3/src/lapi.c \
    lua-5.3.3/src/lauxlib.c \
    lua-5.3.3/src/lbaselib.c \
    lua-5.3.3/src/lbitlib.c \
    lua-5.3.3/src/lcode.c \
    lua-5.3.3/src/lcorolib.c \
    lua-5.3.3/src/lctype.c \
    lua-5.3.3/src/ldblib.c \
    lua-5.3.3/src/ldebug.c \
    lua-5.3.3/src/ldo.c \
    lua-5.3.3/src/ldump.c \
    lua-5.3.3/src/lfunc.c \
    lua-5.3.3/src/lgc.c \
    lua-5.3.3/src/linit.c \
    lua-5.3.3/src/liolib.c \
    lua-5.3.3/src/llex.c \
    lua-5.3.3/src/lmathlib.c \
    lua-5.3.3/src/lmem.c \
    lua-5.3.3/src/loadlib.c \
    lua-5.3.3/src/lobject.c \
    lua-5.3.3/src/lopcodes.c \
    lua-5.3.3/src/loslib.c \
    lua-5.3.3/src/lparser.c \
    lua-5.3.3/src/lstate.c \
    lua-5.3.3/src/lstring.c \
    lua-5.3.3/src/lstrlib.c \
    lua-5.3.3/src/ltable.c \
    lua-5.3.3/src/ltablib.c \
    lua-5.3.3/src/ltm.c \
    lua-5.3.3/src/lundump.c \
    lua-5.3.3/src/lutf8lib.c \
    lua-5.3.3/src/lvm.c \
    lua-5.3.3/src/lzio.c \

HEADERS += \
    window.h \
    glwidget.h \
    data.h \
    camera.h \
    ae/ae.h \
    equationparser.h \
    widgetaddons.h \
    helpers.h \
    tutorial.h \
    lua-5.3.3/install/include/lauxlib.h \
    lua-5.3.3/install/include/lua.h \
    lua-5.3.3/install/include/lua.hpp \
    lua-5.3.3/install/include/luaconf.h \
    lua-5.3.3/install/include/lualib.h \
    lua-5.3.3/src/lapi.h \
    lua-5.3.3/src/lauxlib.h \
    lua-5.3.3/src/lcode.h \
    lua-5.3.3/src/lctype.h \
    lua-5.3.3/src/ldebug.h \
    lua-5.3.3/src/ldo.h \
    lua-5.3.3/src/lfunc.h \
    lua-5.3.3/src/lgc.h \
    lua-5.3.3/src/llex.h \
    lua-5.3.3/src/llimits.h \
    lua-5.3.3/src/lmem.h \
    lua-5.3.3/src/lobject.h \
    lua-5.3.3/src/lopcodes.h \
    lua-5.3.3/src/lparser.h \
    lua-5.3.3/src/lprefix.h \
    lua-5.3.3/src/lstate.h \
    lua-5.3.3/src/lstring.h \
    lua-5.3.3/src/ltable.h \
    lua-5.3.3/src/ltm.h \
    lua-5.3.3/src/lua.h \
    lua-5.3.3/src/lua.hpp \
    lua-5.3.3/src/luaconf.h \
    lua-5.3.3/src/lualib.h \
    lua-5.3.3/src/lundump.h \
    lua-5.3.3/src/lvm.h \
    lua-5.3.3/src/lzio.h \

RESOURCES += \
    resources.qrc

#QMAKE_LFLAGS += -framework OpenCL

DISTFILES +=
