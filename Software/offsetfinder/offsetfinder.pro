#-------------------------------------------------
#
# Project created by QtCreator 2012-06-05T13:03:08
#
#-------------------------------------------------

QT -= core gui

DESTDIR  = ../src/bin
TARGET   = offsetfinder
LIBS += -ldxguid

QMAKE_LIBS_QT_ENTRY =
DEFINES += NO_QT

include(../build-config.prf)

# The offsetfinder is used to get the x86 offsets when running as x64
QMAKE_TARGET.arch = x86

CONFIG(msvc) {
    # This will suppress many MSVC warnings about 'unsecure' CRT functions.
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    # Parallel build
    QMAKE_CXXFLAGS += /MP
} else {
    QMAKE_LFLAGS +=-Wl,--kill-at
}

SOURCES += \
	main.cpp \
    ../common/WinDXUtils.cpp

HEADERS += \
    ../common/D3D10GrabberDefs.hpp \
    ../common/WinDXUtils.hpp \
    ../common/defs.h \
    ../common/msvcstub.h \
    ../common/BufferFormat.h
