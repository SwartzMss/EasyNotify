#include "mainwindow.h"
#include "logger.h"
#include "singleinstance.h"
#include <QApplication>
#include <QDir>
#include <QDateTime>
#include <windows.h>
#include <dbghelp.h>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QMessageBox>

// 设置异常处理函数
LONG WINAPI TopLevelExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo)
{
    // 获取程序所在目录
    QString appDir = QCoreApplication::applicationDirPath();
    QString dumpDir = appDir + "/dump";
    QDir().mkpath(dumpDir);

    // 生成 dump 文件名
    QString dumpFile = dumpDir + "/crash_" + 
                      QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + 
                      ".dmp";

    // 创建 dump 文件
    HANDLE hFile = CreateFileW(dumpFile.toStdWString().c_str(),
                             GENERIC_WRITE,
                             0,
                             NULL,
                             CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        MINIDUMP_EXCEPTION_INFORMATION exInfo;
        exInfo.ExceptionPointers = pExceptionInfo;
        exInfo.ThreadId = GetCurrentThreadId();
        exInfo.ClientPointers = TRUE;

        // 写入 dump 文件
        MiniDumpWriteDump(GetCurrentProcess(),
                         GetCurrentProcessId(),
                         hFile,
                         MiniDumpNormal,
                         &exInfo,
                         NULL,
                         NULL);

        CloseHandle(hFile);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

int main(int argc, char *argv[])
{
    // 设置异常处理
    SetUnhandledExceptionFilter(TopLevelExceptionHandler);

    QApplication a(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("EasyNotify");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("SwartzMss");
    QApplication::setOrganizationDomain("github.com/SwartzMss");
    
    // 初始化日志系统
    Logger::instance();
    LOG_INFO("应用程序启动");

    // 检查是否已经有实例在运行
    if (SingleInstance::instance().isRunning()) {
        LOG_INFO("程序已经在运行，发送激活消息");
        SingleInstance::instance().sendMessage("ACTIVATE");
        return 0;
    }
    
    MainWindow w;
    w.show();
    
    // 连接消息接收信号
    QObject::connect(&SingleInstance::instance(), &SingleInstance::messageReceived,
                    &w, [&w](const QString &message) {
                        if (message == "ACTIVATE") {
                            w.show();
                            w.activateWindow();
                            w.raise();
                        }
                    });
    
    int result = a.exec();
    
    LOG_INFO("应用程序退出");
    return result;
} 