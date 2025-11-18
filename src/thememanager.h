#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QPalette>
#include <QString>
#include <QVector>

class ThemeManager
{
public:
    enum class Theme {
        Light,
        Dark,
        Aurora
    };

    static ThemeManager& instance();

    void applyTheme(Theme theme);
    Theme currentTheme() const;

    QVector<Theme> availableThemes() const;
    QString key(Theme theme) const;
    QString displayName(Theme theme) const;
    Theme themeFromKey(const QString &key) const;

private:
    ThemeManager();
    QPalette paletteFor(Theme theme) const;
    QString styleSheetFor(Theme theme) const;

    struct ThemeMeta {
        Theme theme;
        const char *key;
        const char *name;
    };

    QVector<ThemeMeta> metadata;
    Theme current;
};

#endif // THEMEMANAGER_H
