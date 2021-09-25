#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H

#include <QWidget>
#include <scintillaeditcustom.h>
#include <QLineEdit>
#include <data.h>
#include <QTabWidget>

class ItemWidget: public QWidget
{
    Q_OBJECT
    
public:
    ItemWidget(const Data::Item& item, QFont font);
    
    QLineEdit* mNeed = nullptr;
    ScintillaEditCustom* mJournal = nullptr;
    ScintillaEditCustom* mAnswer = nullptr;
    
    int64_t ItemID() { return mItemID; }
    bool IsEmpty();

protected:
    int64_t mItemID = -1;
    
    QTabWidget* mJournalAnswer;
    
private slots:
    void TabChanged(int index);
    void UpdateTabTitleText();
};

#endif // ITEMWIDGET_H
