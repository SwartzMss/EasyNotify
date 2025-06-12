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

// 设置崩溃转储文件的保存路径
QString getDumpFilePath() {
	QString dumpDir = QCoreApplication::applicationDirPath() + "/dumps";
	QDir().mkpath(dumpDir);
	return dumpDir + "/crash_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".dmp";
}

// 崩溃处理函数
LONG WINAPI TopLevelExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo) {
	static bool isHandling = false;
	if (isHandling) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	isHandling = true;

	try {
		QString dumpPath = getDumpFilePath();
		HANDLE hFile = CreateFileW(
			dumpPath.toStdWString().c_str(),
			GENERIC_WRITE,
			0,
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (hFile != INVALID_HANDLE_VALUE) {
			MINIDUMP_EXCEPTION_INFORMATION exInfo;
			exInfo.ExceptionPointers = pExceptionInfo;
			exInfo.ThreadId = GetCurrentThreadId();
			exInfo.ClientPointers = TRUE;

			// 创建完整的内存转储
			MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				hFile,
				static_cast<MINIDUMP_TYPE>(MiniDumpNormal | MiniDumpWithFullMemory | MiniDumpWithHandleData),
				&exInfo,
				NULL,
				NULL
			);

			CloseHandle(hFile);

			// 记录崩溃信息到日志
			LOG_ERROR(QString("程序崩溃，转储文件已保存到: %1").arg(dumpPath));
			LOG_ERROR(QString("异常代码: 0x%1").arg(pExceptionInfo->ExceptionRecord->ExceptionCode, 8, 16, QChar('0')));
			LOG_ERROR(QString("异常地址: 0x%1").arg((quintptr)pExceptionInfo->ExceptionRecord->ExceptionAddress, 8, 16, QChar('0')));
		}
	}
	catch (...) {
		// 如果转储过程中发生异常，至少记录一下
        LOG_INFO("程序崩溃，但无法创建转储文件");
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