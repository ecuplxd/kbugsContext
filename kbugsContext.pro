#-------------------------------------------------
#
# Project created by QtCreator 2019-07-09T17:25:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kbugsContext
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
                kubeconfig.cpp \
                loadconfigfromserverdialog.cpp \
                main.cpp \
                kbugscontext.cpp \
                ctxindicator.cpp

HEADERS += \
                kubeconfig.h \
                kbugscontext.h \
                loadconfigfromserverdialog.h \
                ctxindicator.h

FORMS += \
                kbugscontext.ui \
                loadconfigfromserverdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

unix:!macx: LIBS += -L$$PWD/lib/ -lyaml-cpp

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include

unix:!macx: PRE_TARGETDEPS += $$PWD/lib/libyaml-cpp.a

RESOURCES += \
        res.qrc

#include(./qtsingleapplication/src/qtsingleapplication.pri)


unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += gtk+-3.0
unix: PKGCONFIG += appindicator3-0.1
