#-------------------------------------------------
#
# Project created by QtCreator 2012-01-20T22:09:20
#
#-------------------------------------------------

QT       += core gui

TARGET = lighttasks
TEMPLATE = app


SOURCES += main.cpp \
    MainWindow.cpp \
    Task.cpp \
    TaskButton.cpp \
    TimeEditWidget.cpp \
    qticonloader.cc

HEADERS  += \
    MainWindow.h \
    Task.h \
    TaskButton.h \
    TimeEditWidget.h \
    qticonloader.h

FORMS    += \
    MainWindow.ui \
    TimeEditWidget.ui

RESOURCES += icons/icons.qrc

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }

    BINDIR = $$PREFIX/bin
    DATADIR = $$PREFIX/share

    INSTALLS += target desktop iconsvg icon16 icon32 icon128

    target.path = $$BINDIR

    desktop.path = $$DATADIR/applications
    desktop.files += lighttasks.desktop

    iconsvg.path = $$DATADIR/icons/hicolor/scalable/apps
    iconsvg.files += icons/lighttasks.svg

    icon16.path = $$DATADIR/icons/hicolor/16x16/apps
    icon16.files += icons/16x16/lighttasks.png

    icon32.path = $$DATADIR/icons/hicolor/32x32/apps
    icon32.files += icons/32x32/lighttasks.png

    icon128.path = $$DATADIR/icons/hicolor/128x128/apps
    icon128.files += icons/128x128/lighttasks.png


}
