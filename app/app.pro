TARGET = phone
QT = quickcontrols2 websockets multimedia

SOURCES = main.cpp phone.cpp
HEADERS = phone.h

SUBDIRS = telephony-binding

CONFIG += link_pkgconfig
PKGCONFIG += qtappfw

RESOURCES += \
    phone.qrc \
    images/images.qrc \
    audio.qrc

include(app.pri)
