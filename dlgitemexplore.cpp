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
    auto& item = mData[mItemCurrentID];
    
    if (row < item.Parents().size())
        return item.Parents()[row];
    
    if (row == item.Parents().size())
        return item.mID;
    
    if (row > item.Parents().size())
        return item.Children()[row - item.Parents().size() - 1];
    
    return -1;
}

QString ItemWidgetName(Data::Item item)
{
    if (item.Children().size() > 0)
        return "[" + item.mNeed + "]";
    else
        return item.mNeed;
}

QListWidgetItem* ItemToWidget(Data::Item item)
{
    auto itemWidget = new QListWidgetItem(ItemWidgetName(item));
    if (item.mSolved)
        itemWidget->setBackground(QBrush(QColor("#AEBD93"))); //green
    else
        itemWidget->setBackground(QBrush("#FFA9A3")); //red
    return itemWidget;
}

void DlgItemExplore::ItemListUpdate(int64_t itemID)
{
    auto& item = mData[itemID];
    
    mItemList->clear();
    
    for (auto parentID: item.Parents())
        mItemList->addItem(ItemToWidget(mData[parentID]));
    
    auto widget = ItemToWidget(item);
    auto font = widget->font();
    font.setBold(true);
    widget->setFont(font);
    mItemList->addItem(widget);
    
    for (auto childID: item.Children())
        mItemList->addItem(ItemToWidget(mData[childID]));
    
    int idxItemCurrent = item.Parents().indexOf(mItemCurrentID);
    if (idxItemCurrent < 0)
        idxItemCurrent = item.Parents().size() + 1 + item.Children().indexOf(mItemCurrentID);
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
    if (mData[itemID].Children().size() == 0)
    {
        emit ItemOpen(itemID, true);
        hide();
    }
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

void DlgItemExplore::RefreshAfterMaxOneItemDifference()
{
    auto& item = mData[mItemCurrentID];
    
    QVector<QString> listPrev;
    for (int idx = 0; idx < mItemList->count(); ++idx)
        listPrev.push_back(mItemList->item(idx)->text());
    
    QVector<int64_t> listNowIDs;
    for (auto parentID: item.Parents())
        listNowIDs.push_back(parentID);
    listNowIDs.push_back(mItemCurrentID);
    for (auto childID: item.Children())
        listNowIDs.push_back(childID);
    
    QVector<QString> listNow;
    for (auto id: listNowIDs)
        listNow.push_back(ItemWidgetName(mData[id]));
    
    // Check if we could be in the case that the item just became childless.
    // In this case there are 2 changes happening to the list:
    // - the name of the current item loses the brackets []
    // - the last child disappeared
    // In this case just update the whole item and lose the current row selection and the scroll position. I assume this won't be a common or big inconvenience.
    if (item.Children().size() == 0 &&
        listPrev.size() > listNow.size())
    {
        ItemListUpdate(mItemCurrentID);
        return;
    }
    
    // Synchronize mItemList so that it contains listNow.
    // Find the idx where there is a difference between listPrev and listNow
    int idx = 0;
    while (idx < listPrev.size() &&
           idx < listNow.size() &&
           listPrev[idx] == listNow[idx])
        ++idx;
    
    if (listNow.size() == listPrev.size() && idx == listNow.size())
        return; // No change.
    
    if (listPrev.size() > listNow.size())
    {
        // An item was removed from listPrev, namely listPrev[idx].
        delete mItemList->takeItem(idx); // We are responsible for destroying the item we take.
    }
    else if (listNow.size() == listPrev.size())
    {
        // An item was changed from listPrev to listNow, namely the idx item.
        delete mItemList->takeItem(idx);
        mItemList->insertItem(idx, ItemToWidget(mData[listNowIDs[idx]]));
        if (mItemList->currentRow() > 0)
            mItemList->setCurrentRow(mItemList->currentRow() - 1);
    }
    else
    {
        // An item was added to listNow, namely listNow[idx].
        mItemList->insertItem(idx, ItemToWidget(mData[listNowIDs[idx]]));
    }
}
