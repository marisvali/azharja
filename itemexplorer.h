#ifndef ITEMEXPLORER_H
#define ITEMEXPLORER_H

#include <QDialog>
#include <QListWidget>
#include <data.h>

class ItemExplorer : public QDialog
{
    Q_OBJECT
    
public:
    enum ExplorerType
    {
        Main,
        Unassigned,
        Search
    };
    
    explicit ItemExplorer(QString name, ExplorerType type, Data& data, QWidget *parent = nullptr);
    void RefreshAfterMaxOneItemDifference();
    int64_t GetSelectedID() { return mSelectedID; }
    
signals:
    void ItemOpen(int64_t itemID, bool grabFocus);
    void ItemCloseCurrent(bool grabFocus);
    void AddParent();
    void ItemOpenNew(bool grabFocus);
    void ItemDeleteCurrent(bool grabFocus);
    void ItemSwitchTabs();
    void ShowMain();
    void ShowUnassigned();
    
protected:
    QListWidget* mItemList = nullptr;
    int64_t mItemIDCurrent = -1;
    QVector<int64_t> mItemIDs;
    Data& mData;
    ExplorerType mType;
    QString mName;
    int64_t mSelectedID = -1;
    
    void closeEvent(QCloseEvent* event) override;
    void accept() override;
    void reject() override;
    int64_t ItemIDSelected();
    void ItemListUpdate(int64_t itemID);
    void UpdateCurrentIDs();
    void SaveSettings();

protected slots:
    void ItemDoubleClicked(QListWidgetItem *item); 
    void ItemEnter();
    void ItemPreview();
    void ItemPreviewClose();
    void ItemOpenNewSlot();
    void ItemDeleteCurrentSlot();
};

#endif // ITEMEXPLORER_H
