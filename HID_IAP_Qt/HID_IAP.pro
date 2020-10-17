#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T00:28:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HID_IAP
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    src/comthread.cpp \
    src/kit.cpp

HEADERS  += mainwindow.h \
    src/lusb0_usb.h \
    src/comthread.h \
    src/kit.h

FORMS    += mainwindow.ui

LIBS += "D:\\lgl\\iPrj\\USB\\HID_IAP\\HID_IAP\\HID_IAP_Qt\\libusb.a"

RC_FILE += myico.rc

OTHER_FILES += \
    myico.rc

RESOURCES += \
    images.qrc

