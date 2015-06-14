#-------------------------------------------------
#
# Project created by QtCreator 2013-06-11T16:47:52
#
#-------------------------------------------------

QT       += widgets

DESTDIR = ../lib
TARGET = grab
TEMPLATE = lib
CONFIG += staticlib

include(../build-config.prf)

# Grabber types configuration
include(configure-grabbers.prf)

LIBS += -lprismatik-math

INCLUDEPATH += ./include \
               ../src \
               ../math/include \
               ..

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
    contains(DEFINES, MAC_OS_CG_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/MacOSGrabber.hpp
        GRABBERS_SOURCES += MacOSGrabber.cpp
    }
}

# Windows platform
win32 {
    contains(DEFINES, WINAPI_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/WinAPIGrabber.hpp
        GRABBERS_SOURCES += WinAPIGrabber.cpp
    }

    contains(DEFINES, WINAPI_EACH_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/WinAPIGrabberEachWidget.hpp
        GRABBERS_SOURCES += WinAPIGrabberEachWidget.cpp
    }

    contains(DEFINES, D3D9_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/D3D9Grabber.hpp
        GRABBERS_SOURCES += D3D9Grabber.cpp
    }

    contains(DEFINES, D3D10_GRAB_SUPPORT) {
        GRABBERS_HEADERS += include/D3D10Grabber.hpp
        GRABBERS_SOURCES += D3D10Grabber.cpp
    }
}

# Common Qt grabbers
contains(DEFINES, QT_GRAB_SUPPORT) {
    GRABBERS_HEADERS += \
        include/QtGrabberEachWidget.hpp \
        include/QtGrabber.hpp

    GRABBERS_SOURCES += \
        QtGrabberEachWidget.cpp \
        QtGrabber.cpp
}

HEADERS += \
    include/calculations.hpp \
    include/GrabberBase.hpp \
    include/ColorProvider.hpp \
    include/GrabberContext.hpp \
    $${GRABBERS_HEADERS}

SOURCES += \
    calculations.cpp \
    GrabberBase.cpp \
    include/ColorProvider.cpp \
    $${GRABBERS_SOURCES}

win32 {
    !isEmpty( DIRECTX_SDK_DIR ) {
        # This will suppress gcc warnings in DX headers.
        CONFIG(gcc) {
            QMAKE_CXXFLAGS += -isystem "\"$${DIRECTX_SDK_DIR}/Include\""
        } else {
            INCLUDEPATH += "\"$${DIRECTX_SDK_DIR}/Include\""
        }
    }

    CONFIG(msvc) {
        # This will suppress many MSVC warnings about 'unsecure' CRT functions.
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
        # Parallel build
        QMAKE_CXXFLAGS += /MP
        # Create "fake" project dependencies of the libraries used dynamically
        LIBS += -lprismatik-hooks -llibraryinjector -lprismatik-unhook
        contains(QMAKE_TARGET.arch, x86_64) {
            # workaround for qmake not being able to support win32 projects in a x64 sln
            # this results in "Project not selected to build for this solution configuration"
            # thus build them manually as post-build step of grab
            QMAKE_POST_LINK = MSBuild.exe ..\hooks\prismatik-hooks32.vcxproj /p:Configuration=$(Configuration)\
                                && MSBuild.exe ..\offsetfinder\offsetfinder.vcxproj /p:Configuration=$(Configuration)\
                                && MSBuild.exe ..\unhook\prismatik-unhook32.vcxproj /p:Configuration=$(Configuration)
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

    #LIBS += \
    #        -framework CoreGraphics
    #        -framework CoreFoundation
    #QMAKE_MAC_SDK = macosx10.8
}

OTHER_FILES += \
    configure-grabbers.prf
