#include "mainwindow.h"
#include "logger.h"

#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // 设置应用程序信息
    QApplication::setApplicationName("EasyNotify");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("SwartzMss");
    QApplication::setOrganizationDomain("github.com/SwartzMss");
    
    // 加载翻译文件（如果需要）
    QTranslator translator;
    if (translator.load(":/translations/easynotify_zh_CN")) {
        a.installTranslator(&translator);
    }
    
    // 初始化日志系统
    Logger::instance();
    LOG_INFO("应用程序启动");
    
    MainWindow w;
    w.show();
    
    int result = a.exec();
    
    LOG_INFO("应用程序退出");
    return result;
} 