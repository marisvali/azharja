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
    ItemWidget(Data::Item& item, QFont font);
    
    QLineEdit* mNeed = nullptr;
    ScintillaEditCustom* mJournal = nullptr;
    ScintillaEditCustom* mAnswer = nullptr;
    
    int64_t ItemID() { return mItem.ID(); }
    bool IsEmpty();

protected:
    QTabWidget* mJournalAnswer;
    Data::Item& mItem;
    bool mDirtyJournal = false;
    bool mDirtyAnswer = false;
    bool mDirtyNeed = false;
    
private slots:
    void TabChanged(int index);
    void UpdateJournal();
    void UpdateAnswer();
    void UpdateNeed();
};

#endif // ITEMWIDGET_H
