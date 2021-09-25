#ifndef DLGITEMEXPLORE_H
#define DLGITEMEXPLORE_H

#include <QDialog>
#include <QListWidget>
#include <data.h>

class DlgItemExplore : public QDialog
{
    Q_OBJECT
    
public:
    explicit DlgItemExplore(Data& data, QWidget *parent = nullptr);
    void RefreshAfterMaxOneItemDifference();
    
signals:
    void ItemOpen(int64_t itemID, bool grabFocus);
    void ItemClose(int64_t itemID, bool grabFocus);
    void AddParent(int64_t itemID);
    
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
};

#endif // DLGITEMEXPLORE_H
