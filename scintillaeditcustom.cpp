#include "scintillaeditcustom.h"

ScintillaEditCustom::ScintillaEditCustom(QFont font, QWidget *parent) : ScintillaEdit(parent)
{
    // Set font.
    styleSetFont(STYLE_DEFAULT, font.family().toUtf8().data());
    styleSetSize(STYLE_DEFAULT, font.pointSize());
    
    // Wrap text.
    setWrapMode(1);
    
    // Don't display line numbers on the left.
    for (int idx = 0; idx < margins(); ++idx)
        setMarginWidthN(idx, 0);
    
    // Make tabs and indentation work like in Notepad++.
    setTabWidth(4);
    setUseTabs(false);
    setTabIndents(true);
    setBackSpaceUnIndents(true);
    setIndent(4);
    setWrapIndentMode(SC_WRAPINDENT_SAME);
    
    // Enable the emission of the notifyChange signal which I need.
    setModEventMask(SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT);
    setCommandEvents(true);
    
    // When selecting a word with control, go to the end of the current word, not the beginning of the next one.
    assignCmdKey(SCK_RIGHT + ((SCMOD_CTRL | SCMOD_SHIFT) << 16), SCI_WORDRIGHTENDEXTEND);
    
    // Maintain the indent of the previous line.
    connect(this, SIGNAL(charAdded(int)), this, SLOT(CharAdded(int)));
    
    // Make the current line visible.
    setElementColour(SC_ELEMENT_CARET_LINE_BACK, 217 | 235 << 8 | 249 << 16 | 255 << 24);
    setCaretLineVisibleAlways(true);
}

void ScintillaEditCustom::CharAdded(int ch)
{
    // Maintain the indent of the previous line.
    if (ch == '\n')
    {
        auto currentLine = lineFromPosition(currentPos());
        if (currentLine > 0)
        {
            auto indentation = lineIndentation(currentLine - 1);
            setLineIndentation(currentLine, indentation);
            auto newPos = currentPos() + indentation;
            setSel(newPos, newPos);
        }
    }
}
