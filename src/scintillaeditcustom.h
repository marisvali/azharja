#ifndef SCINTILLAEDITCUSTOM_H
#define SCINTILLAEDITCUSTOM_H

// This #define took a lot of hours to uncover.
// Without it, MSVC 2019 complains that there are unresolved external symbols
// when linking to the debug version of the static lib. It has something to do
// with __declspec(dllexport) and __declspec(dllimport). I don't know enough
// about this aspect of Microsoft C++ coding to understand why adding this
// define solves the problem. I suspect it has something to do with the
// Scintilla component expecting to be used as a DLL and I'm using it as a
// static library.
#define MAKING_LIBRARY
#include <ScintillaEdit.h>

class ScintillaEditCustom : public ScintillaEdit
{
    Q_OBJECT
public:
    explicit ScintillaEditCustom(QFont font, QWidget *parent = nullptr);

signals:

private slots:
    void CharAdded(int ch);
    void AddDate();
};

#endif  // SCINTILLAEDITCUSTOM_H
