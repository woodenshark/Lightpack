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

include(../build-config.prf)

DEFINES += LIBRARYINJECTOR_LIBRARY
CONFIG(msvc) {
    # This will suppress many MSVC warnings about 'unsecure' CRT functions.
    DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    # Parallel build
    QMAKE_CXXFLAGS += /MP
    # Add export definition for COM methods
    QMAKE_LFLAGS += /DEF:"LibraryInjector.def"
    # Copy output to ../bin
    QMAKE_POST_LINK = cd $(TargetDir) && \
        copy /y libraryinjector.dll ..\\bin\\
} else {
    QMAKE_LFLAGS +=-Wl,--kill-at
    # Copy output to ../bin
    QMAKE_POST_LINK = cd $$DESTDIR && \
        cp -f libraryinjector.dll ../bin/
}

SOURCES += \
    LibraryInjector.c \
    dllmain.c

HEADERS += \
    ILibraryInjector.h \
    LibraryInjector.h
