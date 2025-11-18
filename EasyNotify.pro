QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/active_reminderlist.cpp \
    src/completed_reminderlist.cpp \
    src/active_reminderedit.cpp \
    src/remindermanager.cpp \
    src/logger.cpp \
    src/notificationPopup.cpp \
    src/configmanager.cpp \
    src/singleinstance.cpp \
    src/reminder.cpp   \
    src/active_remindertablemodel.cpp \
    src/completed_remindertablemodel.cpp \
    src/activereminderwindow.cpp \
    src/completedreminderwindow.cpp \
    src/remoteclient.cpp \
    src/workdaycalendar.cpp

HEADERS += \
    src/mainwindow.h \
    src/active_reminderlist.h \
    src/completed_reminderlist.h \
    src/active_reminderedit.h \
    src/remindermanager.h \
    src/logger.h \
    src/notificationPopup.h \
    src/configmanager.h \
    src/singleinstance.h \
    src/reminder.h \
    src/active_remindertablemodel.h \
    src/completed_remindertablemodel.h \
    src/activereminderwindow.h \
    src/completedreminderwindow.h \
    src/remoteclient.h \
    src/workdaycalendar.h

FORMS += \
    src/mainwindow.ui \
    src/active_reminderlist.ui \
    src/completed_reminderlist.ui \
    src/reminderedit.ui \
    src/notificationPopup.ui \
    src/activereminderwindow.ui \
    src/completedreminderwindow.ui

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

# 添加 dbghelp 库
LIBS += -ldbghelp

# 设置应用程序信息
QMAKE_TARGET_COMPANY = "Your Company"
QMAKE_TARGET_PRODUCT = "EasyNotify"
QMAKE_TARGET_DESCRIPTION = "A simple reminder application"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2024"

# 生成 PDB 文件
QMAKE_CXXFLAGS_DEBUG += /Zi
QMAKE_CXXFLAGS_RELEASE += /Zi
QMAKE_LFLAGS_DEBUG += /DEBUG
QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF /OPT:ICF 
