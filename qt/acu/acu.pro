#-------------------------------------------------
#
# Project created by QtCreator 2014-10-20T23:48:08
#
#-------------------------------------------------

QT       += core gui

TARGET = acu
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        syncdialog.cpp \
        updatedialog.cpp


HEADERS  += mainwindow.h \
    syncdialog.h \
    updatedialog.h

FORMS    += mainwindow.ui \
    syncdialog.ui \
    updatedialog.ui

#DEFINES = SIMULATOR
DEFINES += CAMERA
INCLUDEPATH += ../../
LIBS = -L../.. -L../../tools -L../../inih_r29 -lfid -ltool -linih -lpthread -lrt
#LIBS += -L../../camera -lcamera -lopenmaxil -lbcm_host -L.. -L/opt/vc/lib 
LIBS += -lsqlite3 
Release:DESTDIR = release
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:RCC_DIR = debug/.rcc
Debug:UI_DIR = debug/.ui

RESOURCES += \
    acu.qrc
