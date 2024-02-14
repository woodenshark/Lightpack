#-------------------------------------------------
#
# Project created by QtCreator 2012-06-05T13:03:08
#
#-------------------------------------------------

QT       -= core gui

DESTDIR  = ../bin
TARGET   = prismatik-unhook
TEMPLATE = lib

include(../build-config.prf)

LIBS += -lshlwapi -ladvapi32 -luser32

#QMAKE_CXXFLAGS_WARN_ON += -Wno-unknown-pragmas
QMAKE_LFLAGS_EXCEPTIONS_ON -= -mthreads
QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads

CONFIG += c++17
CONFIG -= rtti

DEFINES += HOOKSDLL_EXPORTS UNICODE

CONFIG(msvc) {
    # This will suppress many MSVC warnings about 'unsecure' CRT functions.
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    # Parallel build
    QMAKE_CXXFLAGS += /MP
    # Place *.lib and *.exp files in ../lib
    QMAKE_LFLAGS += /IMPLIB:..\\lib\\$(TargetName).lib
} else {
    QMAKE_LFLAGS += -static
}

SOURCES += \
    main.cpp

HEADERS +=
