#-------------------------------------------------
#
# Project created by QtCreator 2012-06-05T13:03:08
#
#-------------------------------------------------

QT -= core gui

DESTDIR  = ../bin
TARGET   = libraryinjector
TEMPLATE = lib
LIBS += -luuid -lole32 -ladvapi32 -luser32

include(../build-config.prf)

DEFINES += LIBRARYINJECTOR_LIBRARY
CONFIG(msvc) {
    # This will suppress many MSVC warnings about 'unsecure' CRT functions.
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    # Parallel build
    QMAKE_CXXFLAGS += /MP
    # Add export definition for COM methods and place *.lib and *.exp files in ../lib
    QMAKE_LFLAGS += /DEF:"LibraryInjector.def" /IMPLIB:..\\lib\\$(TargetName).lib
} else {
    QMAKE_LFLAGS +=-Wl,--kill-at
}

SOURCES += \
    LibraryInjector.c \
    dllmain.c

HEADERS += \
    ILibraryInjector.h \
    LibraryInjector.h
