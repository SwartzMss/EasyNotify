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
#include <psapi.h>
#include <QFileInfo>

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

			// 初始化符号处理器
			HANDLE process = GetCurrentProcess();
			SymInitialize(process, NULL, TRUE);
			SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

			// 获取堆栈信息
			const int MAX_STACK_FRAMES = 64;
			void* stack[MAX_STACK_FRAMES];
			WORD frames = CaptureStackBackTrace(0, MAX_STACK_FRAMES, stack, NULL);
			
			LOG_ERROR("堆栈跟踪信息:");
			for (WORD i = 0; i < frames; i++) {
				DWORD64 address = (DWORD64)stack[i];
				HMODULE module;
				if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
									GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
									(LPCTSTR)address, &module)) {
					char moduleName[MAX_PATH];
					if (GetModuleFileNameA(module, moduleName, MAX_PATH)) {
						DWORD64 moduleBase = (DWORD64)module;
						DWORD64 offset = address - moduleBase;

						// 获取符号信息
						DWORD64 displacement = 0;
						char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
						PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
						symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
						symbol->MaxNameLen = MAX_SYM_NAME;

						// 获取行号信息
						DWORD lineDisplacement = 0;
						IMAGEHLP_LINE64 line;
						line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

						QString symbolInfo;
						if (SymFromAddr(process, address, &displacement, symbol)) {
							symbolInfo = QString(" - %1").arg(symbol->Name);
						}

						QString lineInfo;
						if (SymGetLineFromAddr64(process, address, &lineDisplacement, &line)) {
							lineInfo = QString(" (%1:%2)").arg(line.FileName).arg(line.LineNumber);
						}

						LOG_ERROR(QString("  [%1] 0x%2 - %3+0x%4%5%6")
							.arg(i)
							.arg(address, 16, 16, QChar('0'))
							.arg(QFileInfo(moduleName).fileName())
							.arg(offset, 16, 16, QChar('0'))
							.arg(symbolInfo)
							.arg(lineInfo));
					}
				}
			}

			// 清理符号处理器
			SymCleanup(process);
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