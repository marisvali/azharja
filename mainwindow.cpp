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

MainWindow::ItemWidget::ItemWidget(const Data::Item& item, QFont font)
{
    mItemID = item.mID;
    
    mNeed = new QLineEdit();
    mNeed->setObjectName("Need");
    
    auto journalAnswer = new QTabWidget();
    
    auto tabJournal = new QWidget();
    journalAnswer->addTab(tabJournal, "Journal");
    
    auto tabAnswer = new QWidget();
    journalAnswer->addTab(tabAnswer, "Answer");
    
    QVBoxLayout* tabJournalLayout = new QVBoxLayout(tabJournal);
    mJournal = new ScintillaEdit();
    mJournal->styleSetFont(STYLE_DEFAULT, font.family().toUtf8().data());
    mJournal->styleSetSize(STYLE_DEFAULT, font.pointSize());
    mJournal->setWrapMode(1);
    for (int idx = 0; idx < mJournal->margins(); ++idx)
        mJournal->setMarginWidthN(idx, 0);
    mJournal->setUseTabs(false);
    mJournal->setTabIndents(true);
    mJournal->setIndent(4);
    
    tabJournalLayout->addWidget(mJournal);
    mJournal->setObjectName("Journal");
    
    QVBoxLayout* tabAnswerLayout = new QVBoxLayout(tabAnswer);
    mAnswer = new ScintillaEdit();
    mAnswer->styleSetFont(STYLE_DEFAULT, font.family().toUtf8().data());
    mAnswer->styleSetSize(STYLE_DEFAULT, font.pointSize());
    mAnswer->setWrapMode(1);
    for (int idx = 0; idx < mAnswer->margins(); ++idx)
        mAnswer->setMarginWidthN(idx, 0);
    mAnswer->setUseTabs(false);
    mAnswer->setTabIndents(true);
    mAnswer->setIndent(4);
    
    tabAnswerLayout->addWidget(mAnswer);
    mAnswer->setObjectName("Answer");
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(mNeed);
    layout->addWidget(journalAnswer);
    
    if (mItemID >= 0)
    {
        mNeed->setText(item.mNeed);
        mJournal->setText(item.mJournal.toUtf8().data());
        mAnswer->setText(item.mAnswer.toUtf8().data());
    }
    
    journalAnswer->setCurrentIndex(1);
}

bool MainWindow::ItemWidget::IsEmpty()
{
    return ItemID() == -1 &&
           mNeed->text() == "" &&
           mJournal->length() == 0 &&
           mAnswer->length() == 0;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mUI(new Ui::MainWindow)
{
    mUI->setupUi(this);
    
    mSplitterMain = new QSplitter();
    mItemCurrentWidget = new ItemWidget(Data::Item(), this->font());
    mItemsOpen.push_back(mItemCurrentWidget);
    mSplitterMain->addWidget(mItemCurrentWidget);
    mItemList = new QListWidget();
    mSplitterMain->addWidget(mItemList);
    mSplitterMain->setOrientation(Qt::Orientation::Vertical);
    
    QGridLayout* layout = new QGridLayout(mUI->MainWidget);
    layout->addWidget(mSplitterMain);
    
    connect(mItemList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(on_ItemList_itemDoubleClicked(QListWidgetItem *)));
    
    // Add new tab when right clicking the item list.
    // I use the custom context menu because the context menu is what is triggered on right click.
    mItemList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mItemList, &QListView::customContextMenuRequested, [this](QPoint){
        ItemOpen(ItemIDSelected());
    });
    
    // Add shortcuts.
    auto ctrlW = new QShortcut(QKeySequence("Ctrl+w"), this);
    connect(ctrlW, SIGNAL(activated()), this, SLOT(ItemCloseCurrent()));
    
    auto ctrlN = new QShortcut(QKeySequence("Ctrl+n"), this);
    connect(ctrlN, SIGNAL(activated()), this, SLOT(ItemNew()));
    
    mData.LoadFromDiskOld();
    
    ItemListUpdate(mData.GetItemTop());
}

MainWindow::~MainWindow()
{
    delete mUI;
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

void MainWindow::ItemListUpdate(int64_t itemID)
{
    if (mData.Items[itemID].mChildrenIDs.size() == 0)
        return;
    
    mItemCurrentID = itemID;
    mItemList->clear();
    
    for (auto parentID: mData.Items[itemID].mParentsIDs)
        mItemList->addItem(ItemToWidget(mData.Items[parentID]));
    
    auto widget = ItemToWidget(mData.Items[itemID]);
    auto font = widget->font();
    font.setBold(true);
    widget->setFont(font);
    mItemList->addItem(widget);
    
    for (auto childID: mData.Items[itemID].mChildrenIDs)
        mItemList->addItem(ItemToWidget(mData.Items[childID]));
}

void MainWindow::on_ItemList_itemDoubleClicked(QListWidgetItem*)
{
    auto itemID = ItemIDSelected();
    if (mData.Items[itemID].mChildrenIDs.size() == 0)
        ItemOpen(itemID);
    else
        ItemListUpdate(itemID);
}

int64_t MainWindow::ItemIDSelected()
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

void MainWindow::ItemOpen(int64_t itemID)
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
    
    // Set focus on the answer edit box.
    mItemCurrentWidget->mAnswer->activateWindow();
    mItemCurrentWidget->mAnswer->grabFocus();
}

MainWindow::ItemWidget* MainWindow::ItemOpenGetter(int64_t itemID)
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
    if (!HasOnlyEmptyItem())
    {
        if (mItemsOpen.size() == 1)
        {
            ItemOpen(-1);
            delete mItemsOpen[0];
            mItemsOpen.removeAt(0);
        }
        else
        {
            int idxToClose = ItemFind(mItemCurrentWidget);
            int idxToOpen = idxToClose + 1;
            if (idxToOpen >= mItemsOpen.size())
                idxToOpen = idxToClose - 1;
            ItemOpen(mItemsOpen[idxToOpen]->ItemID());
            delete mItemsOpen[idxToClose];
            mItemsOpen.removeAt(idxToClose);
        }
    }
}

void MainWindow::ItemNew()
{
    ItemOpen(-1);
}

MainWindow::ItemWidget* MainWindow::ItemFind(int64_t itemID)
{
    auto found = std::find_if(mItemsOpen.begin(), mItemsOpen.end(),
                             [itemID](ItemWidget* x){ return x->ItemID() == itemID; });
    return found == mItemsOpen.end() ? nullptr : *found;
}

MainWindow::ItemWidget* MainWindow::ItemFindEmpty()
{
    auto found = std::find_if(mItemsOpen.begin(), mItemsOpen.end(),
                         [](ItemWidget* x){ return x->IsEmpty(); });
    return found == mItemsOpen.end() ? nullptr : *found;
}

int MainWindow::ItemFind(MainWindow::ItemWidget* itemWidget)
{
    for (int idx = 0; idx < mItemsOpen.size(); ++idx)
        if (itemWidget == mItemsOpen[idx])
            return idx;
    
    return -1;
}
