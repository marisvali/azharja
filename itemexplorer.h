#ifndef ITEMEXPLORER_H
#define ITEMEXPLORER_H

#include <QDialog>
#include <QListWidget>
#include <data.h>

class ItemExplorer : public QDialog
{
    Q_OBJECT
    
public:
    explicit ItemExplorer(Data& data, QWidget *parent = nullptr);
    void RefreshAfterMaxOneItemDifference();
    
signals:
    void ItemOpen(int64_t itemID, bool grabFocus);
    void ItemCloseCurrent(bool grabFocus);
    void AddParent(int64_t itemID);
    void ItemOpenNew(bool grabFocus);
    void ItemDeleteCurrent(bool grabFocus);
    void ItemSwitchTabs();
    
protected:
    QListWidget* mItemList = nullptr;
    int64_t mItemCurrentID = -1;
    Data& mData;
    
    void closeEvent(QCloseEvent* event);
    int64_t ItemIDSelected();
    void ItemListUpdate(int64_t itemID);

protected slots:
    void ItemDoubleClicked(QListWidgetItem *item); 
    void ItemEnter();
    void AddParentSlot();
    void ItemPreview();
    void ItemPreviewClose();
    void ItemOpenNewSlot();
    void FocusMainWindow();
    void ItemDeleteCurrentSlot();
};

#endif // ITEMEXPLORER_H
