#-------------------------------------------------
#
# Project created by QtCreator 2012-06-05T13:03:08
#
#-------------------------------------------------

QT       -= core gui

DESTDIR  = ../lib
TARGET   = prismatik-hooks32
TEMPLATE = lib

include(../build-config.prf)

LIBS += -lshlwapi -ladvapi32 -luser32

# The hooks32 is injected into x86 processes when running as x64
QMAKE_TARGET.arch = x86
DEFINES += HOOKS_SYSWOW64
Release:OBJECTS_DIR = release/32
Debug:OBJECTS_DIR = debug/32

!isEmpty( DIRECTX_SDK_DIR ) {
    # This will suppress gcc warnings in DX headers.
    CONFIG(gcc) {
        QMAKE_CXXFLAGS += -isystem "\"$${DIRECTX_SDK_DIR}/Include\""
    } else {
        INCLUDEPATH += "\"$${DIRECTX_SDK_DIR}/Include\""
    }
    LIBS += "-L\"$${DIRECTX_SDK_DIR}/Lib/x86\""
}
LIBS += -ldxguid

#QMAKE_CXXFLAGS_WARN_ON += -Wno-unknown-pragmas
QMAKE_LFLAGS_EXCEPTIONS_ON -= -mthreads
QMAKE_CXXFLAGS_EXCEPTIONS_ON -= -mthreads

CONFIG -= rtti

DEFINES += HOOKSDLL_EXPORTS UNICODE

CONFIG(msvc) {
    # This will suppress many MSVC warnings about 'unsecure' CRT functions.
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    # Parallel build
    QMAKE_CXXFLAGS += /MP
    # Layout changes in the injected dll are potentially harmful (fraps)
    QMAKE_LFLAGS += /INCREMENTAL:NO
} else {
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_LFLAGS += -static
}
CONFIG(msvc) {
    QMAKE_POST_LINK = cd $(TargetDir) && \
            cp -f prismatik-hooks32.dll ../src/bin/
} else {
    QMAKE_POST_LINK = cd $(DESTDIR) && \
            cp -f prismatik-hooks32.dll ../src/bin/
}

SOURCES += \
    hooks.cpp \
    ProxyFuncJmp.cpp \
    ProxyFuncVFTable.cpp \
    ProxyFuncJmpToVFTable.cpp \
    hooksutils.cpp \
    IPCContext.cpp \
    GAPIProxyFrameGrabber.cpp \
    DxgiFrameGrabber.cpp \
    Logger.cpp \
    D3D9FrameGrabber.cpp

HEADERS += \
    hooks.h \
    ../common/D3D10GrabberDefs.hpp \
    ../common/defs.h \
    ../common/msvcstub.h \
    ProxyFunc.hpp \
    ProxyFuncJmp.hpp \
    ProxyFuncJmpToVFTable.hpp \
    ProxyFuncVFTable.hpp \
    hooksutils.h \
    IPCContext.hpp \
    GAPISubstFunctions.hpp \
    res/logmessages.h \
    GAPIProxyFrameGrabber.hpp \
    DxgiFrameGrabber.hpp \
    Logger.hpp \
    LoggableTrait.hpp \
    ../common/BufferFormat.h \
    D3D9FrameGrabber.hpp
