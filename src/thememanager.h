#ifndef AZHARJA_THEMEMANAGER_H
#define AZHARJA_THEMEMANAGER_H
#include "ScintillaEdit.h"

#include <QPalette>
#include <QApplication>

enum class Theme { Light, Dark };

class ThemeManager {
public:
    static void applyGlobalPalette(Theme theme);
    static void applyEditorTheme(ScintillaEdit* editor, Theme theme);
};
#endif  //AZHARJA_THEMEMANAGER_H
