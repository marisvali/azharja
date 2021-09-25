#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H

#include <QWidget>
#include <ScintillaEditCustom.h>
#include <QLineEdit>
#include <data.h>

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
    
private slots:
    void TabChanged(int index);
};

#endif // ITEMWIDGET_H
