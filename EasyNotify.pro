QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/reminderlist.cpp \
    src/reminderedit.cpp \
    src/remindermanager.cpp \
    src/logger.cpp

HEADERS += \
    src/mainwindow.h \
    src/reminderlist.h \
    src/reminderedit.h \
    src/remindermanager.h \
    src/logger.h

FORMS += \
    src/mainwindow.ui \
    src/reminderlist.ui \
    src/reminderedit.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 资源文件
RESOURCES += \
    resources.qrc

# 指定 Qt 安装路径
QTDIR = C:/Qt/6.8.2/msvc2022_64

# 指定编译器
QMAKE_CXXFLAGS += /std:c++17 