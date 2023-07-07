#-------------------------------------------------
#
# Project created by QtCreator 2023-02-22T19:11:19
#
#-------------------------------------------------

QT       += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = rk3399_qt5_windows
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_NO_PRINTER
# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    ftpServer/dataconnection.cpp \
    ftpServer/ftpcommand.cpp \
    ftpServer/ftpcontrolconnection.cpp \
    ftpServer/ftplistcommand.cpp \
    ftpServer/ftpretrcommand.cpp \
    ftpServer/ftpserver.cpp \
    ftpServer/ftpstorcommand.cpp \
    debuglogdialog/debuglogdialog.cpp \
        main.cpp \
        widget.cpp \
    mytcpsocketclient.cpp \
    mysocket_handle_message.cpp

HEADERS += \
    ftpServer/dataconnection.h \
    ftpServer/ftpcommand.h \
    ftpServer/ftpcontrolconnection.h \
    ftpServer/ftplistcommand.h \
    ftpServer/ftpretrcommand.h \
    ftpServer/ftpserver.h \
    ftpServer/ftpstorcommand.h \
    debuglogdialog/debuglogdialog.h \
        widget.h \
    mytcpsocketclient.h


FORMS += \
        widget.ui \
    debuglogdialog/debuglogdialog.ui \
    mytcpsocketclient.ui



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#include(./QFtpServerLib/)
#include(ftpserver_src/src.pri)
#win32:CONFIG(release, debug|release): LIBS += -L ./QFtpServerLib/release/ -lQFtpServerLib
#else:win32:CONFIG(debug, debug|release): LIBS += -L./QFtpServerLib/debug/ -lQFtpServerLib
#else:unix: LIBS += -L./QFtpServerLib/ -lQFtpServerLib

#INCLUDEPATH += ./QFtpServerLib
#DEPENDPATH += ./QFtpServerLib
