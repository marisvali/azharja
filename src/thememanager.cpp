#include "thememanager.h"
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include "ScintillaEdit.h"

void ThemeManager::applyGlobalPalette(Theme theme) {
    QPalette palette;

    if (theme == Theme::Dark) {
        palette.setColor(QPalette::Window, QColor(53, 53, 53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(42, 42, 42));
        palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, Qt::white);
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53, 53, 53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::BrightText, Qt::red);
        palette.setColor(QPalette::Highlight, QColor(70, 110, 200));
        palette.setColor(QPalette::HighlightedText, Qt::white);
    } else {
        // Light theme.
        palette = QApplication::style()->standardPalette();
    }

    qApp->setPalette(palette);
}

void ThemeManager::applyEditorTheme(ScintillaEdit* editor, Theme theme) {
    if (!editor) return;

    QColor fore = (theme == Theme::Dark) ? Qt::white : Qt::black;
    QColor back = (theme == Theme::Dark) ? QColor(30, 30, 30) : Qt::white;

    // Selection colors.
    QColor selBack = (theme == Theme::Dark) ? QColor(192, 192, 192) : QColor(255, 140, 0);
    QColor selFore = (theme == Theme::Dark) ? Qt::black : Qt::white;

    QColor caretLine = (theme == Theme::Dark) ? QColor(40, 40, 40) : QColor(255, 240, 200);

    // Base style.
    editor->send(SCI_STYLESETFORE, STYLE_DEFAULT, fore.rgb());
    editor->send(SCI_STYLESETBACK, STYLE_DEFAULT, back.rgb());
    editor->send(SCI_STYLECLEARALL);

    // Caret & selection.
    editor->send(SCI_SETCARETFORE, fore.rgb());
    editor->send(SCI_SETSELFORE, 1, selFore.rgb());
    editor->send(SCI_SETSELBACK, 1, selBack.rgb());
    editor->send(SCI_SETSELALPHA, SC_ALPHA_NOALPHA);

    // Force mouse selection background.
    editor->setElementColour(SC_ELEMENT_SELECTION_BACK, selBack.rgba());
    editor->setElementColour(SC_ELEMENT_SELECTION_TEXT, selFore.rgba());

    // Caret line background
    editor->setElementColour(SC_ELEMENT_CARET_LINE_BACK, caretLine.rgba());
    editor->setCaretLineVisibleAlways(false);
    
    // Explicitly set the paper (document background) color for Windows.
    editor->send(SCI_SETVIEWWS, SCWS_INVISIBLE);
    editor->send(SCI_SETVIEWEOL, false);
    
    // Ensure the margins also use the correct background color.
    for (int margin = 0; margin < editor->margins(); margin++) {
        editor->send(SCI_SETMARGINBACKN, margin, back.rgb());
    }
    
    // Set whitespace colors to match theme.
    QColor wsColor = (theme == Theme::Dark) ? QColor(70, 70, 70) : QColor(230, 230, 230);
    editor->send(SCI_SETWHITESPACEFORE, true, wsColor.rgb());
    editor->send(SCI_SETWHITESPACEBACK, false, back.rgb());
}

