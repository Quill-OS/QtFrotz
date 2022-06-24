#-------------------------------------------------
#
# Project created by QtCreator 2013-12-09T07:39:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtFrotz
TEMPLATE = app


SOURCES += \
    qttext.cpp \
    qtscreen.cpp \
    qtpic.cpp \
    qtinput.cpp \
    qtinit.cpp \
    qtfile.cpp \
    qtaudio.cpp \
    frotz/variable.c \
    frotz/text.c \
    frotz/table.c \
    frotz/stream.c \
    frotz/sound.c \
    frotz/screen.c \
    frotz/redirect.c \
    frotz/random.c \
    frotz/quetzal.c \
    frotz/process.c \
    frotz/object.c \
    frotz/math.c \
    frotz/frotz_main.c \
    frotz/input.c \
    frotz/hotkey.c \
    frotz/files.c \
    frotz/fastmem.c \
    frotz/err.c \
    frotz/buffer.c \
    main.cpp \
    qtfrotzview.cpp \
    qtfrotzwindow.cpp

HEADERS  += \
    common/setup.h \
    common/frotz.h \
    k_frotz.h \
    qtfrotzview.h \
    qtfrotzwindow.h

FORMS    += \
    qtfrotzwindow.ui

include(QScreenKeyboard/QScreenKeyboard.pri)
include(KoFileDialog/KoFileDialog.pri)
include(KoSettingsDialog/KoSettingsDialog.pri)

