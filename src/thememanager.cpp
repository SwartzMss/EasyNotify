#include "thememanager.h"

#include <QApplication>
#include <QColor>
#include <QStyle>
#include <QObject>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager()
    : current(Theme::Light)
{
    metadata = {
        { Theme::Light,  "light",  QT_TR_NOOP("浅色") },
        { Theme::Dark,   "dark",   QT_TR_NOOP("深色") },
        { Theme::Aurora, "aurora", QT_TR_NOOP("极光") }
    };
}

void ThemeManager::applyTheme(Theme theme)
{
    current = theme;
    qApp->setPalette(paletteFor(theme));
    qApp->setStyleSheet(styleSheetFor(theme));
}

ThemeManager::Theme ThemeManager::currentTheme() const
{
    return current;
}

QVector<ThemeManager::Theme> ThemeManager::availableThemes() const
{
    QVector<Theme> themes;
    themes.reserve(metadata.size());
    for (const ThemeMeta &meta : metadata) {
        themes.append(meta.theme);
    }
    return themes;
}

QString ThemeManager::key(Theme theme) const
{
    for (const ThemeMeta &meta : metadata) {
        if (meta.theme == theme) {
            return QString::fromLatin1(meta.key);
        }
    }
    return QStringLiteral("light");
}

QString ThemeManager::displayName(Theme theme) const
{
    for (const ThemeMeta &meta : metadata) {
        if (meta.theme == theme) {
            return QObject::tr(meta.name);
        }
    }
    return QObject::tr("浅色");
}

ThemeManager::Theme ThemeManager::themeFromKey(const QString &key) const
{
    for (const ThemeMeta &meta : metadata) {
        if (key.compare(QLatin1String(meta.key), Qt::CaseInsensitive) == 0) {
            return meta.theme;
        }
    }
    return Theme::Light;
}

QPalette ThemeManager::paletteFor(Theme theme) const
{
    QPalette palette;
    switch (theme) {
    case Theme::Light: {
        palette = qApp->style()->standardPalette();
        palette.setColor(QPalette::Window, QColor(247, 249, 252));
        palette.setColor(QPalette::Base, QColor(255, 255, 255));
        palette.setColor(QPalette::AlternateBase, QColor(241, 245, 250));
        palette.setColor(QPalette::Button, QColor(240, 244, 249));
        palette.setColor(QPalette::Highlight, QColor(67, 130, 197));
        palette.setColor(QPalette::ButtonText, QColor(32, 38, 41));
        palette.setColor(QPalette::Text, QColor(33, 37, 41));
        palette.setColor(QPalette::WindowText, QColor(33, 37, 41));
        palette.setColor(QPalette::Link, QColor(67, 130, 197));
        palette.setColor(QPalette::BrightText, QColor(255, 255, 255));
        break;
    }
    case Theme::Dark: {
        palette.setColor(QPalette::Window, QColor(30, 32, 36));
        palette.setColor(QPalette::Base, QColor(24, 26, 30));
        palette.setColor(QPalette::AlternateBase, QColor(36, 38, 43));
        palette.setColor(QPalette::Button, QColor(45, 48, 54));
        palette.setColor(QPalette::ButtonText, QColor(230, 230, 230));
        palette.setColor(QPalette::Text, QColor(225, 225, 225));
        palette.setColor(QPalette::WindowText, QColor(225, 225, 225));
        palette.setColor(QPalette::Highlight, QColor(82, 139, 255));
        palette.setColor(QPalette::HighlightedText, QColor(15, 16, 19));
        palette.setColor(QPalette::Link, QColor(90, 170, 255));
        palette.setColor(QPalette::BrightText, QColor(255, 255, 255));
        break;
    }
    case Theme::Aurora: {
        palette.setColor(QPalette::Window, QColor(14, 23, 32));
        palette.setColor(QPalette::Base, QColor(18, 28, 39));
        palette.setColor(QPalette::AlternateBase, QColor(22, 36, 50));
        palette.setColor(QPalette::Button, QColor(24, 64, 85));
        palette.setColor(QPalette::ButtonText, QColor(220, 235, 241));
        palette.setColor(QPalette::Text, QColor(227, 241, 245));
        palette.setColor(QPalette::WindowText, QColor(227, 241, 245));
        palette.setColor(QPalette::Highlight, QColor(0, 173, 181));
        palette.setColor(QPalette::HighlightedText, QColor(12, 18, 25));
        palette.setColor(QPalette::Link, QColor(57, 255, 201));
        palette.setColor(QPalette::BrightText, QColor(255, 255, 255));
        break;
    }
    }
    return palette;
}

QString ThemeManager::styleSheetFor(Theme theme) const
{
    switch (theme) {
    case Theme::Light:
        return QStringLiteral(R"(
            QWidget#centralWidget {
                background-color: #f7f9fc;
            }
            QTabWidget::pane {
                border: 1px solid #d4deef;
                border-radius: 8px;
                background: #ffffff;
            }
            QTabBar::tab {
                background: #e5ecf6;
                border: 1px solid #d4deef;
                padding: 6px 18px;
                min-width: 120px;
                border-top-left-radius: 8px;
                border-top-right-radius: 8px;
            }
            QTabBar::tab:selected {
                background: #ffffff;
                color: #1a2233;
            }
            QPushButton {
                background-color: #ffffff;
                border: 1px solid #cdd6e6;
                border-radius: 6px;
                padding: 6px 16px;
            }
            QPushButton:hover {
                background-color: #eaf0fb;
            }
            QComboBox {
                border: 1px solid #cdd6e6;
                border-radius: 6px;
                padding: 4px 10px;
                background: #ffffff;
            }
            QComboBox QAbstractItemView {
                background: #ffffff;
                selection-background-color: #d3e2ff;
            }
        )");
    case Theme::Dark:
        return QStringLiteral(R"(
            QWidget#centralWidget {
                background-color: #1e2024;
            }
            QTabWidget::pane {
                border: 1px solid #3a3d43;
                border-radius: 8px;
                background: #25282d;
            }
            QTabBar::tab {
                background: #1f2124;
                color: #dcdfe5;
                border: 1px solid #3a3d43;
                padding: 6px 18px;
                min-width: 120px;
                border-top-left-radius: 8px;
                border-top-right-radius: 8px;
            }
            QTabBar::tab:selected {
                background: #2a2d33;
                color: #ffffff;
            }
            QPushButton {
                background-color: #30343b;
                border: 1px solid #4a4f57;
                border-radius: 6px;
                padding: 6px 16px;
                color: #f0f0f0;
            }
            QPushButton:hover {
                background-color: #3b3f47;
            }
            QComboBox {
                border: 1px solid #4a4f57;
                border-radius: 6px;
                padding: 4px 10px;
                background: #1f2124;
                color: #f0f0f0;
            }
            QComboBox QAbstractItemView {
                background: #2a2d33;
                selection-background-color: #3d5a9b;
            }
        )");
    case Theme::Aurora:
        return QStringLiteral(R"(
            QWidget#centralWidget {
                background-color: #0e1720;
            }
            QTabWidget::pane {
                border: 1px solid #215360;
                border-radius: 8px;
                background: #12202c;
            }
            QTabBar::tab {
                background: #142738;
                color: #d0f6ff;
                border: 1px solid #215360;
                padding: 6px 18px;
                min-width: 120px;
                border-top-left-radius: 8px;
                border-top-right-radius: 8px;
            }
            QTabBar::tab:selected {
                background: #143142;
                color: #8ef2ff;
            }
            QPushButton {
                background-color: #1a3f4f;
                border: 1px solid #1fb0c2;
                border-radius: 6px;
                padding: 6px 16px;
                color: #d8faff;
            }
            QPushButton:hover {
                background-color: #1f5267;
            }
            QComboBox {
                border: 1px solid #1fb0c2;
                border-radius: 6px;
                padding: 4px 10px;
                background: #12202c;
                color: #d8faff;
            }
            QComboBox QAbstractItemView {
                background: #12202c;
                selection-background-color: #1f5267;
            }
        )");
    }
    return QString();
}
