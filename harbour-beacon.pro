# NOTICE:
#
# Application name defined in TARGET has a corresponding QML filename.
# If name defined in TARGET is changed, the following needs to be done
# to match new name:
#   - corresponding QML filename must be changed
#   - desktop icon filename must be changed
#   - desktop filename must be changed
#   - icon definition filename in desktop file must be changed
#   - translation filenames have to be changed

# The name of your application
TARGET = harbour-beacon

CONFIG += sailfishapp

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QT += core

SOURCES += src/harbour-beacon.cpp \
    src/huebridge.cpp \
    src/huediscovery.cpp \
    src/mdns/mdns.c \
    src/resourcemodel.cpp \
#    src/test/testbridge.cpp

HEADERS += \
    src/huebridge.h \
    src/huediscovery.h \
    src/mdns/mdns.h \
    src/resourcemodel.h \
 \#    src/test/testbridge.h
    src/resourceobjects/lightobject.h \
    src/resourceobjects/resourceobject.h

DISTFILES += qml/harbour-beacon.qml \
    LICENSE \
    README.md \
    qml/components/ColorPickerItem.qml \
    qml/components/GroupListItem.qml \
    qml/components/LightListItem.qml \
    qml/cover/CoverPage.qml \
    qml/js/hue-colors.js \
    qml/pages/AboutPage.qml \
    qml/pages/AddBridgeDialog.qml \
    qml/pages/BridgesPage.qml \
    qml/pages/ConnectBridgePage.qml \
    qml/hueiconpack/HueIconPack2019/*.svg \
    qml/hueiconpack/ApiV2Archetype/*.svg \
    qml/pages/DevicePage.qml \
    qml/pages/DevicesPage.qml \
    qml/pages/GroupPage.qml \
    qml/pages/HomePage.qml \
    qml/pages/LightPage.qml \
    qml/pages/SettingsPage.qml \
    rpm/harbour-beacon.changes.in \
    rpm/harbour-beacon.changes.run.in \
    rpm/harbour-beacon.spec \
    rpm/harbour-beacon.yaml \
    translations/*.ts \
    harbour-beacon.desktop

SAILFISHAPP_ICONS = 86x86 108x108 128x128 172x172

# to disable building translations every time, comment out the
# following CONFIG line
CONFIG += sailfishapp_i18n

# German translation is enabled as an example. If you aren't
# planning to localize your app, remember to comment out the
# following TRANSLATIONS line. And also do not forget to
# modify the localized app name in the the .desktop file.
TRANSLATIONS += translations/harbour-beacon-de.ts
