QT += core gui network serialport charts multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

INCLUDEPATH += /path/to/python/include
# LIBS += -L/path/to/python/libs -lpython3.13.5  # XX是Python版本号，如3.8

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chartwindow.cpp \
    connectway.cpp \
    main.cpp \
    bcai.cpp \
    newcamera.cpp \
    prodata.cpp

HEADERS += \
    bcai.h \
    chartwindow.h \
    connectway.h \
    newcamera.h \
    prodata.h

FORMS += \
    bcai.ui \
    connectway.ui \
    newcamera.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icon.qrc \
    icon.qrc

DISTFILES += \
    plot.py
