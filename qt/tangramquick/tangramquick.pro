TEMPLATE = lib
TARGET = tangramquick
QT += qml quick
CONFIG += qt plugin c++14

TARGET = $$qtLibraryTarget($$TARGET)
uri = com.mapzen.tangram

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
LIBS += -lz -lpng -lz -lpng -lfreetype
LIBS += ../../build/linux/lib/libicucommon.a

# Input
SOURCES += \
    tangramquick_plugin.cpp \
    tangramquick.cpp \
    urlWorker.cpp \
    platform_qt.cpp

HEADERS += \
    tangramquick_plugin.h \
    tangramquick.h \
    urlWorker.h \
    platform_qt.h

DISTFILES = qmldir

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}

