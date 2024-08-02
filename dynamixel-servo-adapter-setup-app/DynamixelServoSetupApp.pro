QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

win32|linux-g++|linux-g++-64 {
DEFINES += BUILD_DESKTOP
message(Desktop build)
} else {
DEFINES += BUILD_DEVICE
message(Device build)
}

SOURCES += \
    about.cpp \
    dxl1x.cpp \
    feature_changeid.cpp \
    feature_setposition.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    about.h \
    dxl1x.h \
    feature_changeid.h \
    feature_setposition.h \
    mainwindow.h

FORMS += \
    about.ui \
    feature_changeid.ui \
    feature_setposition.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc


RC_ICONS = resources/icon.ico
ICON = icon.icns
