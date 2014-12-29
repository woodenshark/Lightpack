#-------------------------------------------------
#
# Project created by QtCreator 2012-06-05T13:03:08
#
#-------------------------------------------------

QT -= core gui

DESTDIR  = ../lib
TARGET   = libraryinjector
TEMPLATE = lib
LIBS += -luuid -lole32 -ladvapi32 -luser32

DEFINES += LIBRARYINJECTOR_LIBRARY
CONFIG(msvc) {
    # This will suppress many MSVC warnings about 'unsecure' CRT functions.
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    # Parallel build
    QMAKE_CXXFLAGS += /MP
    # Add export definition for COM methods
    QMAKE_LFLAGS += /DEF:"LibraryInjector.def"
} else {
    QMAKE_LFLAGS +=-Wl,--kill-at
}

SOURCES += \
    LibraryInjector.c \
    dllmain.c

HEADERS += \
    ILibraryInjector.h \
    LibraryInjector.h
