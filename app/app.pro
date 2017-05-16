TARGET = phone
QT = quickcontrols2

SOURCES = main.cpp

SUBDIRS = telephony-binding

RESOURCES += \
    phone.qrc \
    images/images.qrc \
    audio.qrc

include(app.pri)
