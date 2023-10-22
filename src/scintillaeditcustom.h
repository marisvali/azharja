#ifndef SCINTILLAEDITCUSTOM_H
#define SCINTILLAEDITCUSTOM_H

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

#endif // SCINTILLAEDITCUSTOM_H
