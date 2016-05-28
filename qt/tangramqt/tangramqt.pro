#-------------------------------------------------
#
# Project created by QtCreator 2016-05-25T11:29:18
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = tangramqt
TEMPLATE = app

INCLUDEPATH += ../../core/include
INCLUDEPATH += ../../core/include/glm
INCLUDEPATH += ../../external/yaml-cpp/include
INCLUDEPATH += ../../core/src
INCLUDEPATH += ../../core/include/isect2d/include
INCLUDEPATH += ../../core/include/variant
INCLUDEPATH += ../../core/include/mapbox
INCLUDEPATH += ../../core/include/sdf
INCLUDEPATH += ../../core/include/stb
INCLUDEPATH += ../../core/include/sdf
INCLUDEPATH += ../../core/include/stb
INCLUDEPATH += ../../core/include/pbf
INCLUDEPATH += ../../core/include/rapidjson
INCLUDEPATH += ../../external/alfons/src
INCLUDEPATH += ../../external/duktape
INCLUDEPATH += ../../external/css-color-parser-cpp
INCLUDEPATH += ../../core/include/fontstash-es/fontstash/lib/include/freetype2
INCLUDEPATH += ../../core/include/fontstash-es/fontstash/lib/include/harfbuzz
INCLUDEPATH += ../../core/data


LIBS += ../../build/linux/lib/libcore.a
LIBS += -lcurl
LIBS += ../../build/linux/lib/libglfw3.a
LIBS += -ldl -lrt -lm -ldl -lpthread -lGLU -lGL

LIBS += ../../build/linux/lib/libduktape.a
LIBS += ../../build/linux/lib/libcss-color-parser-cpp.a
LIBS += ../../build/linux/lib/libgeojson-vt-cpp.a
LIBS += ../../build/linux/lib/libyaml-cpp.a
LIBS += ../../build/linux/lib/libalfons.a
LIBS += ../../build/linux/lib/liblinebreak.a
LIBS += ../../build/linux/lib/libharfbuzz.a
LIBS += ../../build/linux/lib/libfreetype.a
LIBS += -lz -lpng -lz -lpng
LIBS += ../../build/linux/lib/libicucommon.a

SOURCES += main.cpp \
        mainwindow.cpp \
    tangramwidget.cpp \
    platform_qt.cpp \
    urlWorker.cpp \

HEADERS  += mainwindow.h \
    tangramwidget.h \
    platform_qt.h \
    urlWorker.h \

FORMS    += mainwindow.ui
