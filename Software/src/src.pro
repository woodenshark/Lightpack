# -------------------------------------------------
# src.pro
#
# Copyright (c) 2010,2011 Mike Shatohin, mikeshatohin [at] gmail.com
# http://lightpack.tv https://github.com/woodenshark/Lightpack
# Project created by QtCreator 2010-04-28T19:08:13
# -------------------------------------------------

TARGET      = Prismatik
CONFIG(msvc) {
    PRE_TARGETDEPS += ../lib/grab.lib
} else {
    PRE_TARGETDEPS += ../lib/libgrab.a
}
DESTDIR     = ../bin
TEMPLATE    = app
QT         += network widgets
CONFIG += c++17
win32 {
    QT += serialport
}
macx {
    QT += serialport
}
# QMake and GCC produce a lot of stuff
OBJECTS_DIR = stuff
MOC_DIR     = stuff
UI_DIR      = stuff
RCC_DIR     = stuff

# Find currect git revision
GIT_REVISION = $$system(git show -s --format="%h")

# For update GIT_REVISION use it:
#   $ qmake Lightpack.pro && make clean && make
#
# Or simply edit this file (add space anythere
# for cause to call qmake) and re-build project

isEmpty( GIT_REVISION ){
    # In code uses #ifdef GIT_REVISION ... #endif
    message( "GIT not found, GIT_REVISION will be undefined" )
} else {
    # Define current mercurial revision id
    # It will be show in about dialog and --help output
    DEFINES += GIT_REVISION=\\\"$${GIT_REVISION}\\\"
}

TRANSLATIONS += ../res/translations/en.ts \
       ../res/translations/ru_RU.ts \
       ../res/translations/uk_UA.ts \
       ../res/translations/pl_PL.ts \
       ../res/translations/zh_CN.ts
RESOURCES    = ../res/LightpackResources.qrc
RC_FILE      = ../res/Lightpack.rc

include(../build-config.prf)

# Grabber types configuration
include(../grab/configure-grabbers.prf)
DEFINES += $${SUPPORTED_GRABBERS}

LIBS    += -L../lib -lgrab -lprismatik-math

QMAKE_CFLAGS = $$(CFLAGS)
QMAKE_CXXFLAGS = $$(CXXFLAGS)
QMAKE_LFLAGS = $$(LDFLAGS)

CONFIG(clang) {
    QMAKE_CXXFLAGS += -stdlib=libc++
    LIBS += -stdlib=libc++
}

unix:!macx{
    PKGCONFIG_BIN = $$system(which pkg-config)
    isEmpty(PKGCONFIG_BIN) {
        error("pkg-config not found")
    }
    CONFIG    += link_pkgconfig
    PKGCONFIG += libusb-1.0

    DESKTOP = $$(XDG_CURRENT_DESKTOP)

    equals(DESKTOP, "Unity") {
        DEFINES += UNITY_DESKTOP
        PKGCONFIG += gtk+-2.0 appindicator-0.1 libnotify
    }

    LIBS += -L../qtserialport/lib -lQt5SerialPort
    QMAKE_LFLAGS += -Wl,-rpath=/usr/lib/prismatik
}

win32 {
    CONFIG(msvc) {
        # This will suppress many MSVC warnings about 'unsecure' CRT functions.
        DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
        # Parallel build
        QMAKE_CXXFLAGS += /MP
        # Fix __cplusplus macro as required by Qt
        QMAKE_CXXFLAGS += /Zc:__cplusplus
        # As required by Qt
        QMAKE_CXXFLAGS += -permissive-
        # Place *.lib and *.exp files in ../lib
        QMAKE_LFLAGS += /IMPLIB:..\\lib\\$(TargetName).lib
    }

    # Windows version using WinAPI for HID
    LIBS    += -lsetupapi
    # For QSerialDevice
    LIBS    += -luuid -ladvapi32

    !isEmpty( DIRECTX_SDK_DIR ) {
        LIBS += -L$${DIRECTX_SDK_DIR}/Lib/x86
    }
    LIBS    += -lwsock32 -lshlwapi -lole32 -ldxguid

    SOURCES += hidapi/windows/hid.c

    #DX9 grab
    LIBS    += -lgdi32 -ld3d9

    QMAKE_CFLAGS += -O2 -ggdb
    # Windows version using WinAPI + GDI + DirectX for grab colors

    LIBS    += -lwsock32 -lshlwapi -lole32

    LIBS    += -lpsapi
    LIBS    += -lwtsapi32

    CONFIG(msvc) {
        versionAtLeast(QT_VERSION, 6.0.0) {
            QMAKE_POST_LINK = cd $(TargetDir) $$escape_expand(\r\n)\
                $$[QT_INSTALL_BINS]/windeployqt --no-opengl-sw --no-svg --no-translations --no-compiler-runtime \"$(TargetName)$(TargetExt)\" $$escape_expand(\r\n)
        } else {
            QMAKE_POST_LINK = cd $(TargetDir) $$escape_expand(\r\n)\
                $$[QT_INSTALL_BINS]/windeployqt --no-angle --no-svg --no-translations --no-compiler-runtime \"$(TargetName)$(TargetExt)\" $$escape_expand(\r\n)
        }
        QMAKE_POST_LINK += cd $(TargetDir) $$escape_expand(\r\n)\
            if $(PlatformToolsetVersion) LEQ 140 copy /y \"$(VcInstallDir)redist\\$(PlatformTarget)\\Microsoft.VC$(PlatformToolsetVersion).CRT\\msvcp$(PlatformToolsetVersion).dll\" .\ $$escape_expand(\r\n)\
            if $(PlatformToolsetVersion) GTR 140 copy /y \"$(VcInstallDir)Redist\\MSVC\\$(VCToolsRedistVersion)\\$(PlatformTarget)\\Microsoft.VC$(PlatformToolsetVersion).CRT\\msvcp*.dll\" .\ $$escape_expand(\r\n)\
            if $(PlatformToolsetVersion) LSS 140 copy /y \"$(VcInstallDir)redist\\$(PlatformTarget)\\Microsoft.VC$(PlatformToolsetVersion).CRT\\msvcr$(PlatformToolsetVersion).dll\" .\ $$escape_expand(\r\n)\
            if $(PlatformToolsetVersion) EQU 140 copy /y \"$(VcInstallDir)redist\\$(PlatformTarget)\\Microsoft.VC$(PlatformToolsetVersion).CRT\\vcruntime$(PlatformToolsetVersion).dll\" .\ $$escape_expand(\r\n)\
            if $(PlatformToolsetVersion) GTR 140 copy /y \"$(VcInstallDir)Redist\\MSVC\\$(VCToolsRedistVersion)\\$(PlatformTarget)\\Microsoft.VC$(PlatformToolsetVersion).CRT\\vcruntime*.dll\" .\ $$escape_expand(\r\n)
        !contains(DEFINES, NO_OPENSSL) {
            # QT switched to OpenSSL 1.1 in 5.12.4 and to 3.x in 6, which has different binary names
            versionAtLeast(QT_VERSION, "6.0.0") {
                QMAKE_POST_LINK += copy /y \"$${OPENSSL_DIR}\\libcrypto-3-x64.dll\" .\ $$escape_expand(\r\n)\
                    copy /y \"$${OPENSSL_DIR}\\libssl-3-x64.dll\" .\ $$escape_expand(\r\n)\
                    IF EXIST  \"$${OPENSSL_DIR}\\msvcr*.dll\" copy /y \"$${OPENSSL_DIR}\\msvcr*.dll\" .\ $$escape_expand(\r\n)
            } else:versionAtLeast(QT_VERSION, "5.12.4") {
                QMAKE_POST_LINK += copy /y \"$${OPENSSL_DIR}\\libcrypto-1_1-x64.dll\" .\ $$escape_expand(\r\n)\
                    copy /y \"$${OPENSSL_DIR}\\libssl-1_1-x64.dll\" .\ $$escape_expand(\r\n)\
                    IF EXIST  \"$${OPENSSL_DIR}\\msvcr*.dll\" copy /y \"$${OPENSSL_DIR}\\msvcr*.dll\" .\ $$escape_expand(\r\n)
            } else {
                QMAKE_POST_LINK += copy /y \"$${OPENSSL_DIR}\\ssleay32.dll\" .\ $$escape_expand(\r\n)\
                    copy /y \"$${OPENSSL_DIR}\\libeay32.dll\" .\ $$escape_expand(\r\n)\
                    IF EXIST  \"$${OPENSSL_DIR}\\msvcr*.dll\" copy /y \"$${OPENSSL_DIR}\\msvcr*.dll\" .\ $$escape_expand(\r\n)
            }
        }

    } else {
        warning("unsupported setup - update src.pro to copy dependencies")
    }

    contains(DEFINES,BASS_SOUND_SUPPORT) {
        INCLUDEPATH += $${BASS_DIR}/c/ \
            $${BASSWASAPI_DIR}/c/

        contains(QMAKE_TARGET.arch, x86_64) {
            LIBS += -L$${BASS_DIR}/c/x64/ -L$${BASSWASAPI_DIR}/c/x64/
        } else {
            LIBS += -L$${BASS_DIR}/c/ -L$${BASSWASAPI_DIR}/c/
        }

        LIBS    += -lbass -lbasswasapi

        contains(QMAKE_TARGET.arch, x86_64) {
            QMAKE_POST_LINK += cd $(TargetDir) $$escape_expand(\r\n)\
                copy /y \"$${BASS_DIR}\\x64\\bass.dll\" .\ $$escape_expand(\r\n)\
                copy /y \"$${BASSWASAPI_DIR}\\x64\\basswasapi.dll\" .\
        } else {
            QMAKE_POST_LINK += cd $(TargetDir) $$escape_expand(\r\n)\
                copy /y \"$${BASS_DIR}\\bass.dll\" .\ $$escape_expand(\r\n)\
                copy /y \"$${BASSWASAPI_DIR}\\basswasapi.dll\" .\
        }

        DEFINES += SOUNDVIZ_SUPPORT
    }

    contains(DEFINES,NIGHTLIGHT_SUPPORT) {
        contains(QMAKE_TARGET.arch, x86_64) {
            Release:LIBS += -L$${NIGHTLIGHT_DIR}/Release/
            Debug:LIBS += -L$${NIGHTLIGHT_DIR}/Debug/
            LIBS += -lNightLightLibrary
        }
    }
}

unix:!macx{
    # Linux version using libusb and hidapi codes
    SOURCES += hidapi/linux/hid-libusb.c
    # For X11 grabber
    LIBS +=-lXext -lX11

    contains(DEFINES,PULSEAUDIO_SUPPORT) {
        INCLUDEPATH += $${PULSEAUDIO_INC_DIR} \
            $${FFTW3_INC_DIR}

        defined(PULSEAUDIO_LIB_DIR, var) {
            LIBS += -L$${PULSEAUDIO_LIB_DIR}
            QMAKE_POST_LINK += $(INSTALL_PROGRAM) "$${PULSEAUDIO_LIB_DIR}/libpulse.so.0" $(DESTDIR)
            QMAKE_POST_LINK += $(INSTALL_PROGRAM) "$${PULSEAUDIO_LIB_DIR}/libpulsecommon-13.0.so" $(DESTDIR)
        }

        defined(PULSEAUDIO_LIB_DIR, var) {
            LIBS += -L$${FFTW3_LIB_DIR}
            QMAKE_POST_LINK += $(INSTALL_PROGRAM) "$${FFTW3_LIB_DIR}/libfftw3f.so.3" $(DESTDIR)
        }

        LIBS += -lpulse -lfftw3f
        DEFINES += SOUNDVIZ_SUPPORT
    }
}

macx{
    XCODE_PATH = $$system(xcode-select -print-path)
    QMAKE_LFLAGS += -F/System/Library/Frameworks -F"$${XCODE_PATH}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/PrivateFrameworks"
    # MacOS version using libusb and hidapi codes
    SOURCES += hidapi/mac/hid.c \
    MacOSSession.mm

    HEADERS += \
    MacOSSession.h

    contains(DEFINES,SOUNDVIZ_SUPPORT) {
        SOURCES += MacOSSoundManager.mm
        HEADERS += MacOSSoundManager.h
    }

    LIBS += \
            -framework Cocoa \
            -framework Carbon \
            -framework CoreFoundation \
            #-framework CoreServices \
            -framework Foundation \
 #           -framework CoreGraphics \
            -framework ApplicationServices \
            -framework OpenGL \
            -framework IOKit \
            # private framework
            -weak_framework CoreBrightness \
            -framework AppKit \
            -framework Accelerate \
            -framework CoreMedia \
            -framework AVFoundation \
            -framework CoreVideo \

    ICON = ../res/icons/Prismatik.icns

    QMAKE_INFO_PLIST = ./Info.plist

    #see build-vars.prf
    #isEmpty( QMAKE_MAC_SDK_OVERRIDE ) {
    #    # Default value
    #    # For build universal binaries (native on Intel and PowerPC)
    #    QMAKE_MAC_SDK = macosx10.9
    #} else {
    #    message( "Overriding default QMAKE_MAC_SDK with value $${QMAKE_MAC_SDK_OVERRIDE}" )
    #    QMAKE_MAC_SDK = $${QMAKE_MAC_SDK_OVERRIDE}
    #}

    CONFIG(clang) {
        QMAKE_CXXFLAGS += -x objective-c++
    }
}

# Generate .qm language files
QMAKE_MAC_SDK = macosx
# Qt sets this to its own minimum
# QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.13
system($$[QT_INSTALL_BINS]/lrelease src.pro)

INCLUDEPATH += . \
               .. \
               ./hidapi \
               ../grab \
               ../alienfx \
               ../grab/include \
               ../math/include \
               ./stuff \

SOURCES += \
    LightpackApplication.cpp  main.cpp   SettingsWindow.cpp  Settings.cpp \
    GrabWidget.cpp  GrabConfigWidget.cpp \
    LogWriter.cpp \
    LedDeviceLightpack.cpp \
    LedDeviceAdalight.cpp \
    LedDeviceArdulight.cpp \
    LedDeviceVirtual.cpp \
    AbstractLedDeviceUdp.cpp \
    LedDeviceDrgb.cpp \
    LedDeviceDnrgb.cpp \
    LedDeviceWarls.cpp \
    ColorButton.cpp \
    ApiServer.cpp \
    ApiServerSetColorTask.cpp \
    MoodLampManager.cpp \
    MoodLamp.cpp \
    LiquidColorGenerator.cpp \
    LedDeviceManager.cpp \
    SelectWidget.cpp \
    GrabManager.cpp \
    AbstractLedDevice.cpp \
    PluginsManager.cpp \
    Plugin.cpp \
    LightpackPluginInterface.cpp \
    TimeEvaluations.cpp \
    SystemSession.cpp \
    wizard/ZonePlacementPage.cpp \
    wizard/Wizard.cpp \
    wizard/WizardPageUsingDevice.cpp \
    wizard/SelectProfilePage.cpp \
    wizard/MonitorIdForm.cpp \
    wizard/LightpackDiscoveryPage.cpp \
    wizard/ConfigureDevicePage.cpp \
    wizard/ConfigureUdpDevicePage.cpp \
    wizard/ConfigureDevicePowerPage.cpp \
    wizard/SelectDevicePage.cpp \
    wizard/GlobalColorCoefPage.cpp \
    wizard/CustomDistributor.cpp \
    systrayicon/SysTrayIcon.cpp \
    UpdatesProcessor.cpp \
    LightpackCommandLineParser.cpp

HEADERS += \
    LightpackApplication.hpp \
    SettingsWindow.hpp \
    Settings.hpp \
    SettingsDefaults.hpp \
    version.h \
    TimeEvaluations.hpp \
    GrabManager.hpp \
    GrabWidget.hpp \
    GrabConfigWidget.hpp \
    debug.h \
    LogWriter.hpp \
    alienfx/LFXDecl.h \
    alienfx/LFX2.h \
    LedDeviceLightpack.hpp \
    LedDeviceAdalight.hpp \
    LedDeviceArdulight.hpp \
    AbstractLedDeviceUdp.hpp \
    LedDeviceDrgb.hpp \
    LedDeviceDnrgb.hpp \
    LedDeviceWarls.hpp \
    LedDeviceVirtual.hpp \
    ColorButton.hpp \
    ../common/defs.h \
    enums.hpp         ApiServer.hpp     ApiServerSetColorTask.hpp \
    hidapi/hidapi.h \
    ../../CommonHeaders/COMMANDS.h \
    ../../CommonHeaders/USB_ID.h \
    MoodLampManager.hpp \
    MoodLamp.hpp \
    LiquidColorGenerator.hpp \
    LedDeviceManager.hpp \
    SelectWidget.hpp \
    ../common/D3D10GrabberDefs.hpp \
    AbstractLedDevice.hpp \
    PluginsManager.hpp \
    Plugin.hpp \
    LightpackPluginInterface.hpp \
    SystemSession.hpp \
    wizard/ZonePlacementPage.hpp \
    wizard/Wizard.hpp \
    wizard/WizardPageUsingDevice.hpp \
    wizard/SettingsAwareTrait.hpp \
    wizard/SelectProfilePage.hpp \
    wizard/MonitorIdForm.hpp \
    wizard/LightpackDiscoveryPage.hpp \
    wizard/ConfigureDevicePage.hpp \
    wizard/ConfigureUdpDevicePage.hpp \
    wizard/ConfigureDevicePowerPage.hpp \
    wizard/SelectDevicePage.hpp \
    wizard/GlobalColorCoefPage.hpp \
    types.h \
    wizard/AreaDistributor.hpp \
    wizard/CustomDistributor.hpp \
    systrayicon/SysTrayIcon.hpp \
    UpdatesProcessor.hpp \
    LightpackCommandLineParser.hpp

contains(DEFINES,SOUNDVIZ_SUPPORT) {
    SOURCES += SoundManagerBase.cpp SoundVisualizer.cpp
    HEADERS += SoundManagerBase.hpp SoundVisualizer.hpp
}

win32 {
    SOURCES += LedDeviceAlienFx.cpp \
    WindowsSession.cpp

    HEADERS += LedDeviceAlienFx.hpp \
    WindowsSession.hpp

    contains(DEFINES,SOUNDVIZ_SUPPORT) {
        SOURCES += WindowsSoundManager.cpp
        HEADERS += WindowsSoundManager.hpp
    }
}

unix:!macx {
    contains(DEFINES,SOUNDVIZ_SUPPORT) {
        SOURCES += PulseAudioSoundManager.cpp
        HEADERS += PulseAudioSoundManager.hpp
    }
}

FORMS += SettingsWindow.ui \
    GrabWidget.ui \
    GrabConfigWidget.ui \
    wizard/ZonePlacementPage.ui \
    wizard/Wizard.ui \
    wizard/SelectProfilePage.ui \
    wizard/MonitorIdForm.ui \
    wizard/LightpackDiscoveryPage.ui \
    wizard/ConfigureDevicePage.ui \
    wizard/ConfigureUdpDevicePage.ui \
    wizard/ConfigureDevicePowerPage.ui \
    wizard/SelectDevicePage.ui \
    wizard/GlobalColorCoefPage.ui

#
# QtSingleApplication
#
include(qtsingleapplication/src/qtsingleapplication.pri)

OTHER_FILES += \
    Info.plist
