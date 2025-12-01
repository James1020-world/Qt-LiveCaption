QT += core gui widgets

CONFIG += c++17

TARGET = LiveCaption
TEMPLATE = app

win32 {
    LIBS += -luser32 -lole32 -loleaut32 -luuid
    QMAKE_CXXFLAGS += /std:c++17
}

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

DEFINES += UNICODE _UNICODE