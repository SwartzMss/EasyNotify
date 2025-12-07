QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets sql

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/src

SOURCES += \
    src/app/main.cpp \
    src/core/config/configmanager.cpp \
    src/core/logging/logger.cpp \
    src/core/providers/priorityiconprovider.cpp \
    src/core/reminders/reminder.cpp \
    src/core/reminders/remindermanager.cpp \
    src/core/system/singleinstance.cpp \
    src/core/calendar/workdaycalendar.cpp \
    src/models/active_remindertablemodel.cpp \
    src/models/completed_remindertablemodel.cpp \
    src/ui/windows/mainwindow.cpp \
    src/ui/windows/activereminderwindow.cpp \
    src/ui/windows/completedreminderwindow.cpp \
    src/ui/widgets/active_reminderlist.cpp \
    src/ui/widgets/completed_reminderlist.cpp \
    src/ui/widgets/reminderliststyler.cpp \
    src/ui/widgets/active_reminderedit.cpp \
    src/ui/notifications/notificationPopup.cpp

HEADERS += \
    src/core/config/configmanager.h \
    src/core/logging/logger.h \
    src/core/providers/priorityiconprovider.h \
    src/core/reminders/reminder.h \
    src/core/reminders/remindermanager.h \
    src/core/system/singleinstance.h \
    src/core/calendar/workdaycalendar.h \
    src/models/active_remindertablemodel.h \
    src/models/completed_remindertablemodel.h \
    src/ui/windows/mainwindow.h \
    src/ui/windows/activereminderwindow.h \
    src/ui/windows/completedreminderwindow.h \
    src/ui/widgets/active_reminderlist.h \
    src/ui/widgets/completed_reminderlist.h \
    src/ui/widgets/reminderliststyler.h \
    src/ui/widgets/active_reminderedit.h \
    src/ui/notifications/notificationPopup.h

FORMS += \
    src/ui/windows/mainwindow.ui \
    src/ui/widgets/active_reminderlist.ui \
    src/ui/widgets/completed_reminderlist.ui \
    src/ui/widgets/reminderedit.ui \
    src/ui/notifications/notificationPopup.ui \
    src/ui/windows/activereminderwindow.ui \
    src/ui/windows/completedreminderwindow.ui

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
