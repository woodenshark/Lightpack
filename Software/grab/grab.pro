#-------------------------------------------------
#
# Project created by QtCreator 2013-06-11T16:47:52
#
#-------------------------------------------------

QT       += widgets

DESTDIR = ../lib
TARGET = grab
TEMPLATE = lib
CONFIG += staticlib c++17

include(../build-config.prf)

# Grabber types configuration
include(configure-grabbers.prf)

LIBS += -lprismatik-math

INCLUDEPATH += ./include \
               ../src \
               ../math/include \
               ..

QMAKE_CFLAGS = $$(CFLAGS)
QMAKE_CXXFLAGS = $$(CXXFLAGS)
QMAKE_LFLAGS = $$(LDFLAGS)

CONFIG(clang) {
    QMAKE_CXXFLAGS += -stdlib=libc++
    LIBS += -stdlib=libc++
}

DEFINES += $${SUPPORTED_GRABBERS}
# Linux/UNIX platform
unix:!macx {
    contains(DEFINES, X11_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/X11Grabber.hpp
        GRABBERS_SOURCES += X11Grabber.cpp
    }
}

# Mac platform
macx {
    contains(DEFINES, MAC_OS_CG_GRAB_SUPPORT) | contains(DEFINES, MAC_OS_AV_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/MacOSGrabberBase.hpp
        GRABBERS_SOURCES += MacOSGrabberBase.mm
    }

    contains(DEFINES, MAC_OS_CG_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/MacOSCGGrabber.hpp
        GRABBERS_SOURCES += MacOSCGGrabber.mm
    }

    contains(DEFINES, MAC_OS_AV_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/MacOSAVGrabber.h
        GRABBERS_SOURCES += MacOSAVGrabber.mm
    }
}

# Windows platform
win32 {
    contains(DEFINES, WINAPI_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/WinAPIGrabber.hpp
        GRABBERS_SOURCES += WinAPIGrabber.cpp
    }

    contains(DEFINES, DDUPL_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/DDuplGrabber.hpp
        GRABBERS_SOURCES += DDuplGrabber.cpp
    }

    contains(DEFINES, D3D10_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/D3D10Grabber.hpp
        GRABBERS_SOURCES += D3D10Grabber.cpp
    }
}

HEADERS += \
    include/calculations.hpp \
    include/GrabberBase.hpp \
    include/ColorProvider.hpp \
    include/GrabberContext.hpp \
    include/BlueLightReduction.hpp \
    $${GRABBERS_HEADERS}

SOURCES += \
    calculations.cpp \
    GrabberBase.cpp \
    include/ColorProvider.cpp \
    BlueLightReduction.cpp \
    $${GRABBERS_SOURCES}

win32 {
    CONFIG(msvc) {
        # This will suppress many MSVC warnings about 'unsecure' CRT functions.
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
        # Parallel build
        QMAKE_CXXFLAGS += /MP
        # Fix __cplusplus macro as required by Qt
        QMAKE_CXXFLAGS += /Zc:__cplusplus
        # As required by Qt
        QMAKE_CXXFLAGS += /permissive-
        # Create "fake" project dependencies of the libraries used dynamically
        LIBS += -lprismatik-hooks -llibraryinjector -lprismatik-unhook

        # emulate every other compiler, __SSE4_1__ is defined when AVX2 is enabled (and __AVX2__ is also defined)
        DEFINES += __SSE4_1__ __AVX2__ __AVX512F__ __AVX512BW__
        # causes global vectorization, enable if your target CPU has AVX2
        # QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_AVX2 $$QMAKE_CFLAGS_AVX512F $$QMAKE_CFLAGS_AVX512BW
    }

    contains(DEFINES,NIGHTLIGHT_SUPPORT) {
      contains(QMAKE_TARGET.arch, x86_64) {
        Release:INCLUDEPATH += $${NIGHTLIGHT_DIR}/Release/
        Debug:INCLUDEPATH += $${NIGHTLIGHT_DIR}/Debug/
      }
    }

    HEADERS += \
            ../common/msvcstub.h \
            include/WinUtils.hpp \
            ../common/WinDXUtils.hpp
    SOURCES += \
            WinUtils.cpp \
            ../common/WinDXUtils.cpp
}

macx {
    #QMAKE_LFLAGS += -F/System/Library/Frameworks

    INCLUDEPATH += /System/Library/Frameworks

    HEADERS += \
            include/MacUtils.h
    SOURCES += \
            MacUtils.mm

    #LIBS += \
    #        -framework CoreGraphics
    #        -framework CoreFoundation
    #QMAKE_MAC_SDK = macosx10.8

    # ignored by clang when building for arm
    QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_AVX2
}

unix:!macx {
    CXX_TARGET = $$system($$QMAKE_CXX -dumpmachine)
    contains(CXX_TARGET, x86_64.*) {
        QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_AVX2 $$QMAKE_CFLAGS_AVX512F $$QMAKE_CFLAGS_AVX512BW
    }
}

OTHER_FILES += \
    configure-grabbers.prf
