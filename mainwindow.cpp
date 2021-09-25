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
    mItemsOpen.push_back(new ItemWidget(Data::Item(), this->font()));
    mSplitterMain->addWidget(mItemsOpen[0]);
    mSplitterMain->addWidget(new QWidget());
    mSplitterMain->setOrientation(Qt::Orientation::Vertical);
    mItemParents = new ItemParentsWidget();
    
    QGridLayout* layout = new QGridLayout(mUI->MainWidget);
    layout->addWidget(mSplitterMain);
    
    // Add shortcuts.
    auto ctrlW = new QShortcut(QKeySequence("Ctrl+w"), this);
    connect(ctrlW, SIGNAL(activated()), this, SLOT(ItemCloseCurrent()));
    
    auto ctrlN = new QShortcut(QKeySequence("Ctrl+n"), this);
    connect(ctrlN, SIGNAL(activated()), this, SLOT(ItemOpenNew()));
    
    auto altE = new QShortcut(QKeySequence("Alt+e"), this);
    connect(altE, SIGNAL(activated()), this, SLOT(ItemExploreShow()));
    
    auto altP = new QShortcut(QKeySequence("Alt+p"), this);
    connect(altP, SIGNAL(activated()), this, SLOT(ItemParentsShow()));
    
    auto esc = new QShortcut(QKeySequence("Esc"), this);
    connect(esc, SIGNAL(activated()), this, SLOT(CloseExtraWindows()));
    
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
    
    ItemOpenNew();
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
    // Check if the item is already open.
    ItemWidget* found = nullptr;
    found = ItemFind(itemID);
    if (found)
        return ItemOpen(found, grabFocus);
    
    // Create new item widget.
    auto newItem = new ItemWidget(mData[itemID], this->font());
    ItemOpen(newItem, grabFocus);
}

void MainWindow::ItemOpenNew()
{
    // Check if an empty item already exists.
    ItemWidget* found = nullptr;
    found = ItemFindEmpty();
    if (found)
        return ItemOpen(found, true);
    
    // Create new item.
    auto newItem = new ItemWidget(Data::Item(), this->font());
    ItemOpen(newItem, true);
}

void MainWindow::ItemOpen(ItemWidget* itemWidget, bool grabFocus)
{
    mSplitterMain->replaceWidget(0, itemWidget);
    
    // Delete the empty item if necessary.
    if (HasOnlyEmptyItem() && itemWidget != mItemsOpen[0])
    {
        mItemsOpen[0]->deleteLater();
        mItemsOpen.clear();
    }
    
    // Put the newly opened item at the end of the list of open items.
    if (mItemsOpen.contains(itemWidget))
    {
        mItemsOpen.removeAt(mItemsOpen.indexOf(itemWidget));
        mItemsOpen.push_back(itemWidget);
    }
    else
    {
        mItemsOpen.push_back(itemWidget);
    }
    
    // Clear memory beyond 20 open items.
    if (mItemsOpen.size() > 20)
    {
        mItemsOpen[0]->deleteLater();
        mItemsOpen.removeAt(0);
    }
    
    if (grabFocus)
        ItemCurrentFocus();
    
    ItemParentsUpdate();
}

void MainWindow::ItemParentsUpdate()
{
    // Update the list of parents.
    mItemParents->mList->clear();
    auto itemID = mItemsOpen.last()->ItemID();
    if (itemID >= 0)
    {
        Data::Item& item = mData[itemID];
        for (auto parentID: item.Parents())
            if (parentID != Data::GetItemTop())
                mItemParents->mList->addItem(mData[parentID].mNeed);
    }
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
    
    mItemsOpen.last()->deleteLater();
    mItemsOpen.pop_back();
    
    if (mItemsOpen.size() == 0)
        ItemOpenNew();
    else
        ItemOpen(mItemsOpen.last(), grabFocus);
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
    if (mItemsOpen.last()->ItemID() < 0)
        return;
    
    int idx = mItemParents->mList->currentRow();
    if (idx < 0)
        return;
    
    auto& item = mData[mItemsOpen.last()->ItemID()];
    mData.RemoveParent(item.mID, item.Parents()[idx]);
    ItemParentsUpdate();
    
    // Update the items in explorer.
    mItemExplore->RefreshAfterMaxOneItemDifference();
}

void MainWindow::AddParent(int64_t itemParentID)
{
    if (mItemsOpen.last()->ItemID() < 0)
        return;
    
    auto& item = mData[mItemsOpen.last()->ItemID()];
    mData.AddParent(item.mID, itemParentID);
    ItemParentsUpdate();
    
    // Update the items in explorer.
    mItemExplore->RefreshAfterMaxOneItemDifference();
}

void MainWindow::ItemClose(int64_t itemID, bool grabFocus)
{
    if (mItemsOpen.last()->ItemID() == itemID)
        ItemCloseCurrent(grabFocus);
}

void MainWindow::ItemCurrentFocus()
{
    mItemsOpen.last()->activateWindow();
    mItemsOpen.last()->setFocus();
}

void MainWindow::CloseExtraWindows()
{
    mItemExplore->hide();
    mSplitterMain->replaceWidget(1, new QWidget()); // Hide the list of parents.
}
