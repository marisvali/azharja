#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QXmlStreamReader>
#include <QFile>
#include <exception>
#include <QLineEdit>
#include <QShortcut>
#include <QMessageBox>

MainWindow::ItemEditTab::ItemEditTab(const Data::Item& item, QFont font)
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

bool MainWindow::ItemEditTab::IsEmpty()
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
    
    mUI->ItemEdit->removeTab(0);
    
    //Add an empty tab that's not part of mData.
    OpenItem(-1);
    
    //Add new tab when right clicking the item list.
    //I use the custom context menu because the context menu is what is triggered on right click.
    mUI->ItemList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mUI->ItemList, &QListView::customContextMenuRequested, [this](QPoint){
        OpenItem(ItemIDSelected());
    });
    
    //Add shortcuts.
    auto ctrlW = new QShortcut(QKeySequence("Ctrl+w"), this);
    connect(ctrlW, SIGNAL(activated()), this, SLOT(CloseCurrentTab()));
    
    auto ctrlN = new QShortcut(QKeySequence("Ctrl+n"), this);
    connect(ctrlN, SIGNAL(activated()), this, SLOT(NewTab()));
    
    mData.LoadFromDiskOld();
    
    SetItemList(mData.GetItemTop());
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

void MainWindow::SetItemList(int64_t itemID)
{
    if (mData.Items[itemID].mChildrenIDs.size() == 0)
        return;
    
    mItemIDCurrent = itemID;
    mUI->ItemList->clear();
    
    for (auto parentID: mData.Items[itemID].mParentsIDs)
        mUI->ItemList->addItem(ItemToWidget(mData.Items[parentID]));
    
    auto widget = ItemToWidget(mData.Items[itemID]);
    auto font = widget->font();
    font.setBold(true);
    widget->setFont(font);
    mUI->ItemList->addItem(widget);
    
    for (auto childID: mData.Items[itemID].mChildrenIDs)
        mUI->ItemList->addItem(ItemToWidget(mData.Items[childID]));
}

void MainWindow::on_ItemList_itemDoubleClicked(QListWidgetItem*)
{
    auto itemID = ItemIDSelected();
    if (mData.Items[itemID].mChildrenIDs.size() == 0)
        OpenItem(itemID);
    else
        SetItemList(itemID);
}

int64_t MainWindow::ItemIDSelected()
{
    int row = mUI->ItemList->currentRow();
    auto& item = mData.Items[mItemIDCurrent];
    
    if (row < item.mParentsIDs.size())
        return item.mParentsIDs[row];
    
    if (row == item.mParentsIDs.size())
        return item.mID;
    
    if (row > item.mParentsIDs.size())
        return item.mChildrenIDs[row - item.mParentsIDs.size() - 1];
    
    return -1;
}

void MainWindow::OpenItem(int64_t itemID)
{
    if (itemID >= 0)
    {
        for (int idx = 0; idx < mUI->ItemEdit->count(); ++idx)
            if (dynamic_cast<ItemEditTab*>(mUI->ItemEdit->widget(idx))->ItemID() == itemID)
            {
                mUI->ItemEdit->setCurrentIndex(idx);
                OpenItemSetFocus();
                return;
            }
        
        if (HasOnlyEmptyTab())
            mUI->ItemEdit->removeTab(0);
        
        auto tab = new ItemEditTab(mData.Items[itemID], this->font());
        mUI->ItemEdit->setCurrentIndex(mUI->ItemEdit->addTab(tab, tab->mNeed->text()));
    }
    else
    {
        for (int idx = 0; idx < mUI->ItemEdit->count(); ++idx)
            if (dynamic_cast<ItemEditTab*>(mUI->ItemEdit->widget(idx))->IsEmpty())
            {
                mUI->ItemEdit->setCurrentIndex(idx);
                OpenItemSetFocus();
                return;
            }
        
        auto tab = new ItemEditTab(Data::Item(), this->font());
        mUI->ItemEdit->setCurrentIndex(mUI->ItemEdit->addTab(tab, "Untitled"));
    }
    
    OpenItemSetFocus();
}

void MainWindow::OpenItemSetFocus()
{
    int idx = mUI->ItemEdit->currentIndex();
    if (idx < 0)
        return;
    
    auto tab = dynamic_cast<ItemEditTab*>(mUI->ItemEdit->widget(idx));
    tab->mAnswer->activateWindow();
    tab->mAnswer->grabFocus();
}

bool MainWindow::HasOnlyEmptyTab()
{
    return mUI->ItemEdit->count() == 1 && 
           dynamic_cast<ItemEditTab*>(mUI->ItemEdit->widget(0))->IsEmpty();
}

void MainWindow::on_ItemEdit_tabCloseRequested(int index)
{
    mUI->ItemEdit->removeTab(index);
    if (mUI->ItemEdit->count() == 0)
        OpenItem(-1);
}

void MainWindow::CloseCurrentTab()
{
    if (!HasOnlyEmptyTab())
    {
        mUI->ItemEdit->removeTab(mUI->ItemEdit->currentIndex());
        if (mUI->ItemEdit->count() == 0)
            OpenItem(-1);
    }
}

void MainWindow::NewTab()
{
    OpenItem(-1);
}
