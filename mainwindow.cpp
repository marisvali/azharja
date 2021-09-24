#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QXmlStreamReader>
#include <QFile>
#include <exception>
#include <QLineEdit>
#include <QShortcut>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mUI(new Ui::MainWindow)
{
    mUI->setupUi(this);
    
    mSplitterMain = new QSplitter();
    mItemCurrentWidget = new ItemWidget(Data::Item(), this->font());
    mItemsOpen.push_back(mItemCurrentWidget);
    mSplitterMain->addWidget(mItemCurrentWidget);
    mSplitterMain->addWidget(new QWidget());
    mSplitterMain->setOrientation(Qt::Orientation::Vertical);
    mItemParents = new ItemParentsWidget();
    
    QGridLayout* layout = new QGridLayout(mUI->MainWidget);
    layout->addWidget(mSplitterMain);
    
    // Add shortcuts.
    auto ctrlW = new QShortcut(QKeySequence("Ctrl+w"), this);
    connect(ctrlW, SIGNAL(activated()), this, SLOT(ItemCloseCurrent()));
    
    auto ctrlN = new QShortcut(QKeySequence("Ctrl+n"), this);
    connect(ctrlN, SIGNAL(activated()), this, SLOT(ItemNew()));
    
    auto altE = new QShortcut(QKeySequence("Alt+e"), this);
    connect(altE, SIGNAL(activated()), this, SLOT(ItemExploreShow()));
    
    auto altP = new QShortcut(QKeySequence("Alt+p"), this);
    connect(altP, SIGNAL(activated()), this, SLOT(ItemParentsShow()));
    
    mItemParents->mList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mItemParents->mList, &QListView::customContextMenuRequested, [this](QPoint){
        ParentDelete();
    });
    
    mData.LoadFromDiskOld();
    
    mItemExplore = new DlgItemExplore(mData, this);
    mItemExplore->setFont(this->font());
    connect(mItemExplore, SIGNAL(ItemOpen(int64_t,bool)), this, SLOT(ItemOpen(int64_t,bool)));
    connect(mItemExplore, SIGNAL(AddParent(int64_t)), this, SLOT(AddParent(int64_t)));
    connect(mItemExplore, SIGNAL(ItemClose(int64_t,bool)), this, SLOT(ItemClose(int64_t,bool)));
    
    QSettings settings("PlayfulPatterns", "Azharja");
    restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
    restoreState(settings.value("MainWindow/WindowState").toByteArray());
    mSplitterMain->restoreGeometry(settings.value("MainWindow/mSplitterMainGeometry").toByteArray());
    mSplitterMain->restoreState(settings.value("MainWindow/mSplitterMainState").toByteArray());
    
    ItemOpen(-1);
}

MainWindow::~MainWindow()
{
    delete mUI;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("PlayfulPatterns", "Azharja");
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/WindowState", saveState());
    settings.setValue("MainWindow/mSplitterMainGeometry", mSplitterMain->saveGeometry());
    settings.setValue("MainWindow/mSplitterMainState", mSplitterMain->saveState());
    QMainWindow::closeEvent(event);
}

void MainWindow::ItemOpen(int64_t itemID, bool grabFocus)
{
    mItemCurrentWidget = ItemOpenGetter(itemID);
    mSplitterMain->replaceWidget(0, mItemCurrentWidget);
    
    // Update mItemsOpen.
    if (HasOnlyEmptyItem() && mItemCurrentWidget != mItemsOpen[0])
    {
        delete mItemsOpen[0];
        mItemsOpen.clear();
    }
    if (!mItemsOpen.contains(mItemCurrentWidget))
        mItemsOpen.push_back(mItemCurrentWidget);
    
    if (grabFocus)
        ItemCurrentFocus();
    
    ItemParentsUpdate();
}

void MainWindow::ItemParentsUpdate()
{
    // Update the list of parents.
    mItemParents->mList->clear();
    if (mItemCurrentWidget->ItemID() >= 0)
    {
        Data::Item& item = mData.Items[mItemCurrentWidget->ItemID()];
        for (auto parentID: item.mParentsIDs)
            mItemParents->mList->addItem(mData.Items[parentID].mNeed);
    }
}

ItemWidget* MainWindow::ItemOpenGetter(int64_t itemID)
{
    // Check if the item is already open.
    ItemWidget* found = nullptr;
    if (itemID >= 0)
        found = ItemFind(itemID);
    else
        found = ItemFindEmpty();
    if (found)
        return found;
    
    // Create new item.
    ItemWidget* newItem = nullptr;
    if (itemID >= 0)
        newItem = new ItemWidget(mData.Items[itemID], this->font());
    else
        newItem = new ItemWidget(mData.Items[itemID], this->font());
    return newItem;
}

bool MainWindow::HasOnlyEmptyItem()
{
    return mItemsOpen.size() == 1 && mItemsOpen[0]->IsEmpty();
}

void MainWindow::ItemCloseCurrent()
{
    ItemCloseCurrent(true);
}

void MainWindow::ItemCloseCurrent(bool grabFocus)
{
    if (HasOnlyEmptyItem())
        return;
    
    if (mItemsOpen.size() == 1)
    {
        ItemOpen(-1, grabFocus);
        delete mItemsOpen[0];
        mItemsOpen.removeAt(0);
    }
    else
    {
        int idxToClose = ItemFind(mItemCurrentWidget);
        int idxToOpen = idxToClose + 1;
        if (idxToOpen >= mItemsOpen.size())
            idxToOpen = idxToClose - 1;
        ItemOpen(mItemsOpen[idxToOpen]->ItemID(), grabFocus);
        delete mItemsOpen[idxToClose];
        mItemsOpen.removeAt(idxToClose);
    }
}

void MainWindow::ItemNew()
{
    ItemOpen(-1);
}

ItemWidget* MainWindow::ItemFind(int64_t itemID)
{
    auto found = std::find_if(mItemsOpen.begin(), mItemsOpen.end(),
                             [itemID](ItemWidget* x){ return x->ItemID() == itemID; });
    return found == mItemsOpen.end() ? nullptr : *found;
}

ItemWidget* MainWindow::ItemFindEmpty()
{
    auto found = std::find_if(mItemsOpen.begin(), mItemsOpen.end(),
                         [](ItemWidget* x){ return x->IsEmpty(); });
    return found == mItemsOpen.end() ? nullptr : *found;
}

int MainWindow::ItemFind(ItemWidget* itemWidget)
{
    for (int idx = 0; idx < mItemsOpen.size(); ++idx)
        if (itemWidget == mItemsOpen[idx])
            return idx;
    
    return -1;
}

void MainWindow::ItemExploreShow()
{
    mItemExplore->show();
    mItemExplore->activateWindow();
}

void MainWindow::ItemParentsShow()
{
    if (dynamic_cast<ItemParentsWidget*>(mSplitterMain->widget(1)))
        mSplitterMain->replaceWidget(1, new QWidget());
    else
        mSplitterMain->replaceWidget(1, mItemParents);
}

void MainWindow::ParentDelete()
{
    if (!mItemCurrentWidget || mItemCurrentWidget->ItemID() < 0)
        return;
    
    int idx = mItemParents->mList->currentRow();
    if (idx < 0)
        return;
    
    auto& item = mData.Items[mItemCurrentWidget->ItemID()];
    auto parentID = item.mParentsIDs[idx];
    item.mParentsIDs.removeAt(idx);
    mData.Items[parentID].mChildrenIDs.removeIf([parentID](int64_t id) { return id == parentID; });
    ItemParentsUpdate();
}

void MainWindow::AddParent(int64_t itemParentID)
{
    if (mItemCurrentWidget->ItemID() < 0)
        return;
    
    auto& item = mData.Items[mItemCurrentWidget->ItemID()];
    if (!item.mParentsIDs.contains(itemParentID))
    {
        item.mParentsIDs.push_back(itemParentID);
        mData.Items[itemParentID].mChildrenIDs.push_back(item.mID);
    }
    ItemParentsUpdate();
}

void MainWindow::ItemClose(int64_t itemID, bool grabFocus)
{
    if (mItemCurrentWidget && mItemCurrentWidget->ItemID() == itemID)
        ItemCloseCurrent(grabFocus);
}

void MainWindow::ItemCurrentFocus()
{
    if (mItemCurrentWidget)
    {
        mItemCurrentWidget->activateWindow();
        mItemCurrentWidget->setFocus();
    }
}
