#include "scintillaeditcustom.h"
#include <QShortcut>
#include <QDateTime>

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
    
    // Insert date.
    auto altD = new QShortcut(QKeySequence("Alt+d"), this);
    connect(altD, SIGNAL(activated()), this, SLOT(AddDate()));
    
    // Make the current line visible.
    setElementColour(SC_ELEMENT_CARET_LINE_BACK, 217 | 235 << 8 | 249 << 16 | 255 << 24);
    setCaretLineVisibleAlways(false);
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

void ScintillaEditCustom::AddDate()
{
    auto date = QDateTime::currentDateTime();
    
    QString day;
    switch (date.date().dayOfWeek())
    {
    case 1:
        day = "Monday";
        break;
    case 2:
        day = "Tuesday";
        break;
    case 3:
        day = "Wednesday";
        break;
    case 4:
        day = "Thursday";
        break;
    case 5:
        day = "Friday";
        break;
    case 6:
        day = "Saturday";
        break;
    case 7:
        day = "Sunday";
        break;
    }
    
    QString dateStr = date.toString("yyyy-MM-dd") + ", " + day;
    QString separator(dateStr.length(), '-');
    dateStr = separator + "\n" + dateStr + "\n" + separator + "\n";
        
    insertText(currentPos(), dateStr.toStdString().c_str());
    auto newPos = currentPos() + dateStr.length();
    setSel(newPos, newPos);
}
