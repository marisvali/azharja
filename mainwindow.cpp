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
#include <QTimer>
#include <QThread>
#include <QScreen>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mUI(new Ui::MainWindow)
{
    mUI->setupUi(this);
    
    mData.LoadFromDisk();
    connect(&mData, SIGNAL(DoneWithLastSave()), this, SLOT(DoneWithLastSave()));
    
    mSplitterMain = new QSplitter();
    mSplitterMain->setOrientation(Qt::Orientation::Vertical);
    // Adds a new item in mSplitterMain. It's necessary to add instead of doing a replaceWidget because replaceWidget causes a bug if it's called right after addWidget, in the MainWindow constructor.
    ItemOpenNew(true);
    mSplitterMain->addWidget(new QWidget());
    QGridLayout* layout = new QGridLayout(mUI->MainWidget);
    layout->addWidget(mSplitterMain);
    
    // Initialize the widget with the list of parents.
    mItemParents = new ItemParentsWidget();
    mItemParents->mList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mItemParents->mList, &QListView::customContextMenuRequested, [this](QPoint){
        ParentDelete();
    });
    ItemParentsUpdate();
    
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
    
    // Initialize the item explorer dialog.
    mItemExplore = new DlgItemExplore(mData, this);
    mItemExplore->setFont(this->font());
    connect(mItemExplore, SIGNAL(ItemOpen(int64_t,bool)), this, SLOT(ItemOpen(int64_t,bool)));
    connect(mItemExplore, SIGNAL(AddParent(int64_t)), this, SLOT(AddParent(int64_t)));
    connect(mItemExplore, SIGNAL(ItemClose(int64_t,bool)), this, SLOT(ItemClose(int64_t,bool)));
    
    // Restore the last positions of our windows.
    QSettings settings("PlayfulPatterns", "Azharja");
    restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
    restoreState(settings.value("MainWindow/WindowState").toByteArray());
    mSplitterMain->restoreGeometry(settings.value("MainWindow/mSplitterMainGeometry").toByteArray());
    mSplitterMain->restoreState(settings.value("MainWindow/mSplitterMainState").toByteArray());
    
    // Save data from widget to Item periodically.
    mTimerSaveToMemory = new QTimer(this);
    connect(mTimerSaveToMemory, SIGNAL(timeout()), this, SLOT(SaveToMemoryTry()));
    mTimerSaveToMemory->start(300);
}

MainWindow::~MainWindow()
{
    mSplitterMain->replaceWidget(0, new QWidget());
    for (auto item : mItemsOpen)
        delete item;
    mItemsOpen.clear();
    delete mUI;
}

void MainWindow::DoneWithLastSave()
{
    mWaitForSave->hide();
    this->close();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (mCloseInitiated)
    {
        event->accept();
        return;
    }
    
    mTimerSaveToMemory->stop();
    for (auto item : mItemsOpen)
        item->SaveToMemoryGuaranteed();
    mData.JustOneMoreSave();
    
    mWaitForSave = new QMessageBox(this);
    mWaitForSave->setWindowTitle("Azharja");
    mWaitForSave->setText("  Saving data, please wait..  ");
    mWaitForSave->setStandardButtons(QMessageBox::NoButton);
    mWaitForSave->setFont(this->font());
    auto pos = this->screen()->geometry().center();
    pos -= QPoint(mWaitForSave->sizeHint().width() / 2, 2 * mWaitForSave->sizeHint().height());
    mWaitForSave->move(pos);
    mWaitForSave->show();
    
    QSettings settings("PlayfulPatterns", "Azharja");
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/WindowState", saveState());
    settings.setValue("MainWindow/mSplitterMainGeometry", mSplitterMain->saveGeometry());
    settings.setValue("MainWindow/mSplitterMainState", mSplitterMain->saveState());
    
    mCloseInitiated = true;
    event->ignore();
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
    connect(newItem, SIGNAL(ItemDeleted()), this, SLOT(ItemDeleted()));
    ItemOpen(newItem, grabFocus);
}

void MainWindow::ItemOpenNew(bool grabFocus)
{
    // Check if an empty item already exists.
    ItemWidget* found = nullptr;
    found = ItemFindEmpty();
    if (found)
        return ItemOpen(found, grabFocus);
    
    // Create new item.
    auto newItem = new ItemWidget(mData.CreateNewItem(), this->font());
    connect(newItem, SIGNAL(ItemDeleted()), this, SLOT(ItemDeleted()));
    ItemOpen(newItem, grabFocus);
}

void MainWindow::ItemOpen(ItemWidget* itemWidget, bool grabFocus)
{
    if (mSplitterMain->count() > 0)
        mSplitterMain->replaceWidget(0, itemWidget);
    else
        mSplitterMain->addWidget(itemWidget); // This is necessary for the first call to ItemOpen.
    
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
    if (!mItemParents)
        return;
    
    if (mItemsOpen.empty())
        return;
    
    // Update the list of parents.
    mItemParents->mList->clear();
    auto itemID = mItemsOpen.last()->ItemID();
    if (itemID >= 0)
    {
        auto& item = mData[itemID];
        for (auto parentID: item.Parents())
            if (parentID != Data::GetItemTop())
                mItemParents->mList->addItem(mData[parentID].Need());
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
    if (mItemsOpen.empty())
        return;
    
    if (HasOnlyEmptyItem())
        return;
    
    mItemsOpen.last()->SaveToMemoryGuaranteed();
    mItemsOpen.last()->deleteLater();
    mItemsOpen.pop_back();
    
    if (mItemsOpen.size() == 0)
        ItemOpenNew(grabFocus);
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
    if (mItemsOpen.empty())
        return;
    
    if (mItemsOpen.last()->ItemID() < 0)
        return;
    
    int idx = mItemParents->mList->currentRow();
    if (idx < 0)
        return;
    
    auto& item = mData[mItemsOpen.last()->ItemID()];
    item.RemoveParent(item.Parents()[idx]);
    ItemParentsUpdate();
    
    // Update the items in explorer.
    mItemExplore->RefreshAfterMaxOneItemDifference();
}

void MainWindow::AddParent(int64_t itemParentID)
{
    if (mItemsOpen.empty())
        return;
    
    if (mItemsOpen.last()->ItemID() < 0)
        return;
    
    auto& item = mData[mItemsOpen.last()->ItemID()];
    item.AddParent(itemParentID);
    ItemParentsUpdate();
    
    // Update the items in explorer.
    mItemExplore->RefreshAfterMaxOneItemDifference();
}

void MainWindow::ItemClose(int64_t itemID, bool grabFocus)
{
    if (mItemsOpen.empty())
        return;
    
    if (mItemsOpen.last()->ItemID() == itemID)
        ItemCloseCurrent(grabFocus);
}

void MainWindow::ItemCurrentFocus()
{
    if (mItemsOpen.empty())
        return;
    
    mItemsOpen.last()->activateWindow();
    mItemsOpen.last()->setFocus();
}

void MainWindow::CloseExtraWindows()
{
    mItemExplore->hide();
    mSplitterMain->replaceWidget(1, new QWidget()); // Hide the list of parents.
}

void MainWindow::ItemOpenNew()
{
    ItemOpenNew(true);
}

void MainWindow::SaveToMemoryTry(QPrivateSignal)
{
    if (!mItemsOpen.empty())
        mItemsOpen.last()->SaveToMemoryTry();
}

void MainWindow::ItemDeleted()
{
    // Update the items in explorer.
    mItemExplore->RefreshAfterMaxOneItemDifference();
}
