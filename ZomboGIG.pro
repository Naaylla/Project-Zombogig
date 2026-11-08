QT += widgets
CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = ZomboGIG

INCLUDEPATH += src ui

SOURCES += \
    src/main.cpp \
    src/entities/entity.cpp \
    src/entities/human.cpp \
    src/entities/zombie.cpp \
    ui/cell.cpp \
    ui/wall.cpp \
    ui/prop.cpp \
    ui/grid.cpp \
    ui/clockwidget.cpp \
    ui/soundmanager.cpp \
    ui/mapview.cpp \
    ui/mainwindow.cpp

HEADERS += \
    src/entities/entity.h \
    src/entities/human.h \
    src/entities/zombie.h \
    ui/cell.h \
    ui/wall.h \
    ui/prop.h \
    ui/grid.h \
    ui/clockwidget.h \
    ui/soundmanager.h \
    ui/mapview.h \
    ui/mainwindow.h
