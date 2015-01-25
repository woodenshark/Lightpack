#-------------------------------------------------
#
# Project created by QtCreator 2012-06-05T13:03:08
#
#-------------------------------------------------

QT       -= core gui

DESTDIR  = ../lib
TARGET   = prismatik-unhook
TEMPLATE = lib

include(../build-config.prf)

LIBS += -lshlwapi -ladvapi32 -luser32

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
} else {
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_LFLAGS += -static
}
CONFIG(msvc) {
	QMAKE_POST_LINK = cd $(TargetDir) && \
			cp -f prismatik-unhook.dll ../src/bin/
} else {
	QMAKE_POST_LINK = cd $(DESTDIR) && \
			cp -f prismatik-unhook.dll ../src/bin/
}

SOURCES += \
    main.cpp 

HEADERS +=
