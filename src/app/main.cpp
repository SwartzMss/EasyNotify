#include "ui/windows/mainwindow.h"
#include "core/logging/logger.h"
#include "core/system/singleinstance.h"
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
#include <QFile>

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

        // 记录捕获到未处理异常
        LOG_ERROR("捕获到未处理异常");

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
                        BOOL dumpResult = MiniDumpWriteDump(
                                GetCurrentProcess(),
                                GetCurrentProcessId(),
                                hFile,
                                static_cast<MINIDUMP_TYPE>(MiniDumpNormal | MiniDumpWithFullMemory | MiniDumpWithHandleData),
                                &exInfo,
                                NULL,
                                NULL
                        );

                        if (!dumpResult) {
                                DWORD err = GetLastError();
                                LOG_ERROR(QString("写入转储文件失败(%1): %2").arg(err).arg(dumpPath));
                        }

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
                } else {
                        DWORD err = GetLastError();
                        LOG_ERROR(QString("无法创建转储文件(%1): %2").arg(err).arg(dumpPath));
                }
        }
        catch (...) {
                // 如果转储过程中发生异常，至少记录一下
                LOG_INFO("程序崩溃，但无法创建转储文件");
        }

	return EXCEPTION_CONTINUE_SEARCH;
}

bool checkWavFormat(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR(QString("无法打开文件: %1").arg(filePath));
        return false;
    }
    QByteArray header = file.read(44);
    if (header.size() < 44) {
        LOG_ERROR("文件太小，不是有效的wav文件");
        return false;
    }
    if (header.mid(0, 4) != "RIFF" || header.mid(8, 4) != "WAVE") {
        LOG_ERROR("不是有效的WAV文件");
        return false;
    }
    quint16 audioFormat = *reinterpret_cast<const quint16*>(header.mid(20, 2).constData());
    if (audioFormat != 1) {
        LOG_ERROR("不是PCM格式");
        return false;
    }
    quint16 numChannels = *reinterpret_cast<const quint16*>(header.mid(22, 2).constData());
    if (numChannels != 1 && numChannels != 2) {
        LOG_ERROR("声道数不是1或2");
        return false;
    }
    quint32 sampleRate = *reinterpret_cast<const quint32*>(header.mid(24, 4).constData());
    if (sampleRate != 44100 && sampleRate != 22050) {
        LOG_ERROR("采样率不是44100或22050");
        return false;
    }
    quint16 bitsPerSample = *reinterpret_cast<const quint16*>(header.mid(34, 2).constData());
    if (bitsPerSample != 16) {
        LOG_ERROR("不是16bit采样");
        return false;
    }
    LOG_INFO(QString("WAV格式正确: PCM %1bit %2Hz %3声道").arg(bitsPerSample).arg(sampleRate).arg(numChannels));
    return true;
}

int main(int argc, char *argv[])
{
	// 设置异常处理
	SetUnhandledExceptionFilter(TopLevelExceptionHandler);

	QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(false);
	
	// 设置应用程序信息
	QApplication::setApplicationName("EasyNotify");
	QApplication::setApplicationVersion("1.0.0");
	QApplication::setOrganizationName("SwartzMss");
	QApplication::setOrganizationDomain("github.com/SwartzMss");
	
	// 初始化日志系统
	Logger::instance();
	LOG_INFO("应用程序启动");

	// 检查WAV文件格式
	checkWavFormat(":/sound/Ding.wav");

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
