TARGET = phone
QT = quickcontrols2 websockets

SOURCES = main.cpp

SUBDIRS = telephony-binding

CONFIG += link_pkgconfig
PKGCONFIG += qtappfw

RESOURCES += \
    phone.qrc \
    images/images.qrc \
    audio.qrc

include(app.pri)
