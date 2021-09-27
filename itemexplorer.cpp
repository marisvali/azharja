#include "itemexplorer.h"
#include <QSettings>
#include <QGridLayout>
#include <QShortcut>
#include <QMainWindow>

ItemExplorer::ItemExplorer(QString name, ExplorerType type, Data& data, QWidget *parent): 
    QDialog(parent), mData(data), mType(type), mName(name)
{
    QSettings settings("PlayfulPatterns", "Azharja");
    
    setWindowFlag(Qt::WindowMinimizeButtonHint, true);
    setWindowFlag(Qt::WindowMaximizeButtonHint, true);

    QGridLayout* layout = new QGridLayout(this);
    mItemList = new QListWidget();
    mItemList->setWordWrap(true);
    layout->addWidget(mItemList);
    connect(mItemList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(ItemDoubleClicked(QListWidgetItem*)));
    
    // Open the item when right clicking the item list.
    // I use the custom context menu because the context menu is what is triggered on right click.
    mItemList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mItemList, &QListView::customContextMenuRequested, [this](QPoint){
        emit ItemOpen(ItemIDSelected(), true);
    });

    if (mType == ExplorerType::Main || mType == ExplorerType::Search)
    {
        if (settings.contains(mName + "/StartItem"))
            mItemIDCurrent = settings.value(mName + "/StartItem").toLongLong();
        else
            mItemIDCurrent = mData.GetItemTop();
    }
    
    ItemListUpdate(mItemIDCurrent);
    
    // Add shortcuts.
    if (mType == ExplorerType::Main || mType == ExplorerType::Unassigned)
    {
        auto returnKey = new QShortcut(QKeySequence(Qt::Key_Return), this);
        connect(returnKey, SIGNAL(activated()), this, SLOT(ItemEnter()));
        
        auto right = new QShortcut(QKeySequence(Qt::Key_Right), this);
        connect(right, SIGNAL(activated()), this, SLOT(ItemPreview()));
        
        auto left = new QShortcut(QKeySequence(Qt::Key_Left), this);
        connect(left, SIGNAL(activated()), this, SLOT(ItemPreviewClose()));
        
        auto f3 = new QShortcut(QKeySequence(Qt::Key_F3), this);
        connect(f3, SIGNAL(activated()), this, SIGNAL(ShowMain()));
        
        auto f4 = new QShortcut(QKeySequence(Qt::Key_F4), this);
        connect(f4, SIGNAL(activated()), this, SIGNAL(ShowUnassigned()));
        
        auto ctrlTab = new QShortcut(QKeySequence("Ctrl+Tab"), this);
        connect(ctrlTab, SIGNAL(activated()), this, SIGNAL(ItemSwitchTabs()));
        
        // Shortcuts copied from the main dialog.
        auto ctrlW = new QShortcut(QKeySequence("Ctrl+w"), this);
        connect(ctrlW, SIGNAL(activated()), this, SLOT(ItemPreviewClose()));
        
        auto ctrlN = new QShortcut(QKeySequence("Ctrl+n"), this);
        connect(ctrlN, SIGNAL(activated()), this, SLOT(ItemOpenNewSlot()));
        
        auto altP = new QShortcut(QKeySequence("Alt+p"), this);
        connect(altP, SIGNAL(activated()), parent, SLOT(ItemParentsShow()));
        
        auto ctrlP = new QShortcut(QKeySequence("Ctrl+p"), this);
        connect(ctrlP, SIGNAL(activated()), this, SIGNAL(AddParent()));
        
        auto esc = new QShortcut(QKeySequence("Esc"), this);
        connect(esc, SIGNAL(activated()), parent, SLOT(CloseExtraWindows()));
        
        auto shiftDel = new QShortcut(QKeySequence("Shift+Del"), this);
        connect(shiftDel, SIGNAL(activated()), this, SLOT(ItemDeleteCurrentSlot()));
    }
    else if (mType == ExplorerType::Search)
    {
        auto returnKey = new QShortcut(QKeySequence(Qt::Key_Return), this);
        connect(returnKey, SIGNAL(activated()), this, SLOT(ItemEnter()));
        
        auto right = new QShortcut(QKeySequence(Qt::Key_Right), this);
        connect(right, SIGNAL(activated()), this, SLOT(ItemPreview()));
    }
    
    // Restore window position.
    restoreGeometry(settings.value(mName + "/Geometry").toByteArray());
}

void ItemExplorer::closeEvent(QCloseEvent* event)
{
    QDialog::closeEvent(event);
}

void ItemExplorer::accept()
{
    SaveSettings();
    QDialog::accept();
}
void ItemExplorer::reject()
{
    SaveSettings();
    QDialog::reject();
}

void ItemExplorer::SaveSettings()
{
    QSettings settings("PlayfulPatterns", "Azharja");
    settings.setValue(mName + "/Geometry", saveGeometry());
    settings.setValue(mName + "/StartItem", mItemIDCurrent);    
}

int64_t ItemExplorer::ItemIDSelected()
{
    if (mItemList->currentRow() >= 0)
        return mItemIDs[mItemList->currentRow()];
    else
        return -1;
}

QString ItemWidgetName(Item& item)
{
    if (item.NrChildren() > 0)
        return "[" + item.Need() + "]";
    else
    {
        auto need = item.Need();
        if (need == "")
            return "Untitled";
        else
            return need;
    }
}

QListWidgetItem* ItemToWidget(Item& item)
{
    auto itemWidget = new QListWidgetItem(ItemWidgetName(item));
    if (item.Solved())
        itemWidget->setBackground(QBrush(QColor("#AEBD93"))); //green
    else
        itemWidget->setBackground(QBrush("#FFA9A3")); //red
    return itemWidget;
}

void ItemExplorer::ItemListUpdate(int64_t itemIDToExplore)
{
    if (mType == ExplorerType::Main || mType == ExplorerType::Search)
    {
        auto itemIDOld = mItemIDCurrent;
        mItemIDCurrent = itemIDToExplore;
        auto& item = mData[mItemIDCurrent];
        
        UpdateCurrentIDs();
        
        mItemList->clear();
        
        for (auto itemID: mItemIDs)
            if (itemID == mItemIDCurrent)
            {
                auto widget = ItemToWidget(item);
                auto font = widget->font();
                font.setBold(true);
                widget->setFont(font);
                mItemList->addItem(widget);
            }
            else
                mItemList->addItem(ItemToWidget(mData[itemID]));
        
        int idxItemOld = mItemIDs.indexOf(itemIDOld);
        mItemList->scrollToItem(mItemList->item(idxItemOld), QAbstractItemView::PositionAtCenter);
        mItemList->setCurrentRow(idxItemOld);    
    }
    else
    {
        UpdateCurrentIDs();
        
        for (auto itemID: mItemIDs)
            mItemList->addItem(ItemToWidget(mData[itemID]));
        mItemList->setCurrentRow(0);
    }
}

void ItemExplorer::ItemDoubleClicked(QListWidgetItem*)
{
    ItemEnter();
}

void ItemExplorer::ItemEnter()
{
    if (mType == ExplorerType::Main)
    {
        auto itemID = ItemIDSelected();
        if (mData[itemID].NrChildren() == 0)
        {
            emit ItemOpen(itemID, true);
            hide();
        }
        else
            ItemListUpdate(itemID);
    }
    else if (mType == ExplorerType::Unassigned)
    {
        if (mItemList->currentRow() >= 0)
        {
            emit ItemOpen(mItemIDs[mItemList->currentRow()], true);
            hide();
        }
    }
    else if (mType == ExplorerType::Search)
    {
        auto itemID = ItemIDSelected();
        if (mData[itemID].NrChildren() == 0)
        {
            mSelectedID = itemID;
            accept();
        }
        else
            ItemListUpdate(itemID);
    }
}

void ItemExplorer::ItemPreview()
{
    if (mType == ExplorerType::Main || mType == ExplorerType::Unassigned)
        emit ItemOpen(ItemIDSelected(), false);
    else if (mType == ExplorerType::Search)
    {
        mSelectedID = ItemIDSelected();
        accept();
    }
}

void ItemExplorer::ItemPreviewClose()
{
    emit ItemCloseCurrent(false);
}

void ItemExplorer::UpdateCurrentIDs()
{
    mItemIDs.clear();
    if (mType == ExplorerType::Main || mType == ExplorerType::Search)
    {
        auto& item = mData[mItemIDCurrent];
        auto parents = item.Parents();
        for (auto parentID: parents)
            mItemIDs.push_back(parentID);
        mItemIDs.push_back(mItemIDCurrent);
        auto children = item.Children();
        for (auto childID: children)
            mItemIDs.push_back(childID);
    }
    else if (mType == ExplorerType::Unassigned)
    {
        for (auto item: mData.Items())
            if (item->NrParents() == 0 && item->ID() != Data::GetItemTop())
                mItemIDs.push_back(item->ID());
    }
}

void ItemExplorer::RefreshAfterMaxOneItemDifference()
{
    QVector<QString> listPrev;
    for (int idx = 0; idx < mItemList->count(); ++idx)
        listPrev.push_back(mItemList->item(idx)->text());
 
    UpdateCurrentIDs();
    
    QVector<QString> listNow;
    for (auto id: mItemIDs)
        listNow.push_back(ItemWidgetName(mData[id]));
    
    if (mType == ExplorerType::Main || mType == ExplorerType::Search)
    {
        // Check if we could be in the case that the item just became childless.
        // In this case there are 2 changes happening to the list:
        // - the name of the current item loses the brackets []
        // - the last child disappeared
        // In this case just update the whole item and lose the current row selection and the scroll position. I assume this won't be a common or big inconvenience.
        
        if (mData[mItemIDCurrent].NrChildren() == 0 &&
                listPrev.size() > listNow.size())
        {
            ItemListUpdate(mItemIDCurrent);
            return;
        }
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
        mItemList->insertItem(idx, ItemToWidget(mData[mItemIDs[idx]]));
        if (mItemList->currentRow() > 0)
            mItemList->setCurrentRow(mItemList->currentRow() - 1);
    }
    else
    {
        // An item was added to listNow, namely listNow[idx].
        mItemList->insertItem(idx, ItemToWidget(mData[mItemIDs[idx]]));
    }
}

void ItemExplorer::ItemOpenNewSlot()
{
    emit ItemOpenNew(false);
}

void ItemExplorer::ItemDeleteCurrentSlot()
{
    emit ItemDeleteCurrent(false);
}
