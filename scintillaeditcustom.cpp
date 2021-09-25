#include "scintillaeditcustom.h"

ScintillaEditCustom::ScintillaEditCustom(QFont font, QWidget *parent) : ScintillaEdit(parent)
{
    styleSetFont(STYLE_DEFAULT, font.family().toUtf8().data());
    styleSetSize(STYLE_DEFAULT, font.pointSize());
    setWrapMode(1);
    for (int idx = 0; idx < margins(); ++idx)
        setMarginWidthN(idx, 0);
    setTabWidth(4);
    setUseTabs(false);
    setTabIndents(true);
    setBackSpaceUnIndents(true);
    setIndent(4);
    setWrapIndentMode(SC_WRAPINDENT_SAME);
    setModEventMask(SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT);
    setCommandEvents(true);
    connect(this, SIGNAL(charAdded(int)), this, SLOT(CharAdded(int)));
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
