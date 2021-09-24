#include "dlgitemexplore.h"
#include <QSettings>
#include <QGridLayout>
#include <QShortcut>

DlgItemExplore::DlgItemExplore(Data& data, QWidget *parent) : QDialog(parent), mData(data)
{
    QGridLayout* layout = new QGridLayout(this);
    mItemList = new QListWidget();
    layout->addWidget(mItemList);
    connect(mItemList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(ItemDoubleClicked(QListWidgetItem*)));
    
    // Add new tab when right clicking the item list.
    // I use the custom context menu because the context menu is what is triggered on right click.
    mItemList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mItemList, &QListView::customContextMenuRequested, [this](QPoint){
        emit ItemOpen(ItemIDSelected(), true);
    });
    
    ItemListUpdate(mData.GetItemTop());
    
    // Add shortcut.
    auto altE = new QShortcut(QKeySequence(Qt::Key_Return), this);
    connect(altE, SIGNAL(activated()), this, SLOT(ItemEnter()));
    
    auto ctrlP = new QShortcut(QKeySequence("Ctrl+p"), this);
    connect(ctrlP, SIGNAL(activated()), this, SLOT(AddParentSlot()));
    
    auto right = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(right, SIGNAL(activated()), this, SLOT(ItemPreview()));
    
    auto left = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(left, SIGNAL(activated()), this, SLOT(ItemPreviewClose()));
    
    QSettings settings("PlayfulPatterns", "Azharja");
    restoreGeometry(settings.value("DlgItemExplore/Geometry").toByteArray());
}

void DlgItemExplore::closeEvent(QCloseEvent* event)
{
    QSettings settings("PlayfulPatterns", "Azharja");
    settings.setValue("DlgItemExplore/Geometry", saveGeometry());
    QDialog::closeEvent(event);
}

int64_t DlgItemExplore::ItemIDSelected()
{
    int row = mItemList->currentRow();
    auto& item = mData.Items[mItemCurrentID];
    
    if (row < item.mParentsIDs.size())
        return item.mParentsIDs[row];
    
    if (row == item.mParentsIDs.size())
        return item.mID;
    
    if (row > item.mParentsIDs.size())
        return item.mChildrenIDs[row - item.mParentsIDs.size() - 1];
    
    return -1;
}

QListWidgetItem* ItemToWidget(Data::Item item)
{
    auto itemWidget = new QListWidgetItem(item.mNeed);
    if (item.mSolved)
        itemWidget->setBackground(QBrush(QColor("#AEBD93"))); //green
    else
        itemWidget->setBackground(QBrush("#FFA9A3")); //red
    return itemWidget;
}

void DlgItemExplore::ItemListUpdate(int64_t itemID)
{
    auto& item = mData.Items[itemID];
    
    if (item.mChildrenIDs.size() == 0)
        return;
    
    mItemList->clear();
    
    for (auto parentID: item.mParentsIDs)
        mItemList->addItem(ItemToWidget(mData.Items[parentID]));
    
    auto widget = ItemToWidget(item);
    auto font = widget->font();
    font.setBold(true);
    widget->setFont(font);
    mItemList->addItem(widget);
    
    for (auto childID: item.mChildrenIDs)
        mItemList->addItem(ItemToWidget(mData.Items[childID]));
    
    int idxItemCurrent = item.mParentsIDs.indexOf(mItemCurrentID);
    if (idxItemCurrent < 0)
        idxItemCurrent = item.mParentsIDs.size() + 1 + item.mChildrenIDs.indexOf(mItemCurrentID);
    mItemList->scrollToItem(mItemList->item(idxItemCurrent), QAbstractItemView::PositionAtCenter);
    mItemList->setCurrentRow(idxItemCurrent);
    
    mItemCurrentID = itemID;
}

void DlgItemExplore::ItemDoubleClicked(QListWidgetItem*)
{
    ItemEnter();
}

void DlgItemExplore::ItemEnter()
{
    auto itemID = ItemIDSelected();
    if (mData.Items[itemID].mChildrenIDs.size() == 0)
        emit ItemOpen(itemID, true);
    else
        ItemListUpdate(itemID);
}

void DlgItemExplore::ItemPreview()
{
    emit ItemOpen(ItemIDSelected(), false);
}

void DlgItemExplore::ItemPreviewClose()
{
    emit ItemClose(ItemIDSelected(), false);
}

void DlgItemExplore::AddParentSlot()
{
    auto itemID = ItemIDSelected();
    if (itemID >= 0)
        emit AddParent(itemID);
}
