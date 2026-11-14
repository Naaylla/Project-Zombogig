QT += widgets
CONFIG += c++20
CONFIG -= app_bundle

TEMPLATE = app
TARGET = ZomboGIG

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/logic/types.h \
    src/logic/wall.cpp \
    src/logic/plant.cpp \
    src/logic/cell.cpp \
    src/logic/grid.cpp \
    src/logic/game.cpp \
    src/entities/entity.cpp \
    src/entities/human.cpp \
    src/entities/farmer.cpp \
    src/entities/doctor.cpp \
    src/entities/military.cpp \
    src/entities/builder.cpp \
    src/entities/zombie.cpp \
    src/entities/tank.cpp \
    src/entities/runner.cpp \
    src/entities/exploder.cpp \
    src/ui/ui.cpp

HEADERS += \
    src/logic/types.h \
    src/logic/wall.h \
    src/logic/plant.h \
    src/logic/cell.h \
    src/logic/grid.h \
    src/logic/game.h \
    src/entities/entity.h \
    src/entities/human.h \
    src/entities/farmer.h \
    src/entities/doctor.h \
    src/entities/military.h \
    src/entities/builder.h \
    src/entities/zombie.h \
    src/entities/tank.h \
    src/entities/runner.h \
    src/entities/exploder.h \
    src/ui/ui.h
