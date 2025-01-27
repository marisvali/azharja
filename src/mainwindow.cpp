#include "mainwindow.h"
#include "BoyerMoore.h"

#include <QInputDialog>
#include <QDebug>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QLineEdit>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QXmlStreamReader>
#include <exception>

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      mUI(new Ui::MainWindow),
      mSettings(QDir(QCoreApplication::applicationDirPath()).filePath("Azharja.ini"),
                QSettings::IniFormat)
{
    mUI->setupUi(this);

    if (!mSettings.contains("DataPath"))
    {
        mSettings.setValue("DataPath", "Data");
    }

    // Warning!
    // "Although backslash is a special character in INI files, most Windows applications don't escape backslashes (\) in file paths. QSettings always treats backslash as a special character and provides no API for reading or writing such entries."
    // Translation: you must use / instead of \ in your paths, in the INI file, if you're reading it using QSettings.
    QString dataPath(mSettings.value("DataPath").toString());
    if (QDir(dataPath).isRelative())
        dataPath = QDir(QCoreApplication::applicationDirPath()).filePath(dataPath);

    mData.LoadFromDisk(dataPath);
    connect(&mData, SIGNAL(DoneWithLastSave()), this, SLOT(DoneWithLastSave()));

    mSplitterMain = new QSplitter();
    mSplitterMain->setOrientation(Qt::Orientation::Vertical);
    // Adds a new item in mSplitterMain. It's necessary to add instead of doing a replaceWidget because replaceWidget causes a bug if it's called right after addWidget, in the MainWindow constructor.
    ItemOpenNew(true);
    mSplitterMain->addWidget(new QWidget());
    QGridLayout* layout = new QGridLayout(this->centralWidget());
    layout->addWidget(mSplitterMain);

    // Initialize the widget with the list of parents.
    mItemParents = new ItemParentsWidget();
    mItemParents->mList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mItemParents->mList, &QListView::customContextMenuRequested,
            [this](QPoint) { ParentDelete(); });
    ItemParentsUpdate();

    // Add shortcuts.
    auto ctrlF = new QShortcut(QKeySequence("Ctrl+f"), this);
    connect(ctrlF, SIGNAL(activated()), this, SLOT(ItemFinder()));

    auto ctrlW = new QShortcut(QKeySequence("Ctrl+w"), this);
    connect(ctrlW, SIGNAL(activated()), this, SLOT(ItemCloseCurrent()));

    auto ctrlN = new QShortcut(QKeySequence("Ctrl+n"), this);
    connect(ctrlN, SIGNAL(activated()), this, SLOT(ItemOpenNew()));

    auto f3 = new QShortcut(QKeySequence(Qt::Key_F3), this);
    connect(f3, SIGNAL(activated()), this, SLOT(ItemExplorerShow()));

    auto f4 = new QShortcut(QKeySequence(Qt::Key_F4), this);
    connect(f4, SIGNAL(activated()), this, SLOT(ItemExplorerUnassignedShow()));

    auto altP = new QShortcut(QKeySequence("Alt+p"), this);
    connect(altP, SIGNAL(activated()), this, SLOT(ItemParentsShow()));

    auto ctrlP = new QShortcut(QKeySequence("Ctrl+p"), this);
    connect(ctrlP, SIGNAL(activated()), this, SLOT(AddParent()));

    auto esc = new QShortcut(QKeySequence("Esc"), this);
    connect(esc, SIGNAL(activated()), this, SLOT(CloseExtraWindows()));

    auto altDel = new QShortcut(QKeySequence("Alt+Del"), this);
    connect(altDel, SIGNAL(activated()), this, SLOT(ItemDeleteCurrent()));

    // Initialize the main item explorer dialog.
    mItemExplorer = new ItemExplorer("ExplorerMain", ItemExplorer::ExplorerType::Main, mData, this);
    mItemExplorer->setFont(this->font());
    mItemExplorer->setWindowTitle("Needs explorer");
    connect(mItemExplorer, SIGNAL(ItemOpen(int64_t, bool)), this, SLOT(ItemOpen(int64_t, bool)));
    connect(mItemExplorer, SIGNAL(AddParent()), this, SLOT(AddParent()));
    connect(mItemExplorer, SIGNAL(ItemCloseCurrent(bool)), this, SLOT(ItemCloseCurrent(bool)));
    connect(mItemExplorer, SIGNAL(ItemOpenNew(bool)), this, SLOT(ItemOpenNew(bool)));
    connect(mItemExplorer, SIGNAL(ItemDeleteCurrent(bool)), this, SLOT(ItemDeleteCurrent(bool)));
    connect(mItemExplorer, SIGNAL(ItemSwitchTabs()), this, SLOT(ItemSwitchTabs()));
    connect(mItemExplorer, SIGNAL(ShowMain()), this, SLOT(ItemExplorerShow()));
    connect(mItemExplorer, SIGNAL(ShowUnassigned()), this, SLOT(ItemExplorerUnassignedShow()));


    // Initialize the item explorer for unassigned items.
    mItemExplorerUnassigned =
        new ItemExplorer("ExplorerUnassigned", ItemExplorer::ExplorerType::Unassigned, mData, this);
    mItemExplorerUnassigned->setFont(this->font());
    mItemExplorerUnassigned->setWindowTitle("Unassigned needs");
    connect(mItemExplorerUnassigned, SIGNAL(ItemOpen(int64_t, bool)), this,
            SLOT(ItemOpen(int64_t, bool)));
    connect(mItemExplorerUnassigned, SIGNAL(AddParent()), this, SLOT(AddParent()));
    connect(mItemExplorerUnassigned, SIGNAL(ItemCloseCurrent(bool)), this,
            SLOT(ItemCloseCurrent(bool)));
    connect(mItemExplorerUnassigned, SIGNAL(ItemOpenNew(bool)), this, SLOT(ItemOpenNew(bool)));
    connect(mItemExplorerUnassigned, SIGNAL(ItemDeleteCurrent(bool)), this,
            SLOT(ItemDeleteCurrent(bool)));
    connect(mItemExplorerUnassigned, SIGNAL(ItemSwitchTabs()), this, SLOT(ItemSwitchTabs()));
    connect(mItemExplorerUnassigned, SIGNAL(ShowMain()), this, SLOT(ItemExplorerShow()));
    connect(mItemExplorerUnassigned, SIGNAL(ShowUnassigned()), this,
            SLOT(ItemExplorerUnassignedShow()));

    // Initialize the unassigned item explorer dialog.

    // Restore the last positions of our windows.
    QSettings settings("PlayfulPatterns", "Azharja");
    restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
    restoreState(settings.value("MainWindow/WindowState").toByteArray());
    mSplitterMain->restoreGeometry(
        settings.value("MainWindow/mSplitterMainGeometry").toByteArray());
    mSplitterMain->restoreState(settings.value("MainWindow/mSplitterMainState").toByteArray());

    // Save data from widget to Item periodically.
    mTimerSaveToMemory = new QTimer(this);
    connect(mTimerSaveToMemory, SIGNAL(timeout()), this, SLOT(SaveToMemoryTry()));
    mTimerSaveToMemory->start(300);

    // Create system tray icon.
    auto exitAction = new QAction(tr("&Exit"), this);
    connect(exitAction, &QAction::triggered,
            [this]()
            {
                mCloseFromSystemTray = true;
                close();
            });
    auto trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(exitAction);

    auto sysTrayIcon = new QSystemTrayIcon(this);
    sysTrayIcon->setContextMenu(trayIconMenu);
    sysTrayIcon->setIcon(QIcon(":/icon.ico"));
    sysTrayIcon->show();

    connect(sysTrayIcon, &QSystemTrayIcon::activated,
            [this](auto reason)
            {
                if (reason == QSystemTrayIcon::Trigger)
                {
                    if (isVisible())
                    {
                        hide();
                    }
                    else
                    {
                        show();
                        activateWindow();
                    }
                }
            });
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
    if (!mCloseFromSystemTray)
    {
        this->hide();
        event->ignore();
        return;
    }

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
    mWaitForSave->setText("Saving data, please wait..  ");
    mWaitForSave->setStandardButtons(QMessageBox::NoButton);
    mWaitForSave->setFont(this->font());
    auto pos = this->screen()->geometry().center();
    pos -= QPoint(mWaitForSave->sizeHint().width() / 2, 2 * mWaitForSave->sizeHint().height());
    mWaitForSave->move(pos);
    mWaitForSave->show();

    SaveWindowPositions();

    mCloseInitiated = true;
    event->ignore();
}

void MainWindow::SaveWindowPositions()
{
    QSettings settings("PlayfulPatterns", "Azharja");
    settings.setValue("MainWindow/Geometry", saveGeometry());
    settings.setValue("MainWindow/WindowState", saveState());
    settings.setValue("MainWindow/mSplitterMainGeometry", mSplitterMain->saveGeometry());
    settings.setValue("MainWindow/mSplitterMainState", mSplitterMain->saveState());

    if (mItemExplorer)
        mItemExplorer->SaveSettings();
    if (mItemExplorerUnassigned)
        mItemExplorerUnassigned->SaveSettings();
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
    connect(newItem, SIGNAL(NeedChanged()), this, SLOT(NeedChanged()));
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
    connect(newItem, SIGNAL(NeedChanged()), this, SLOT(NeedChanged()));
    ItemOpen(newItem, grabFocus);
    if (mItemExplorer)
        mItemExplorer->RefreshAfterMaxOneItemDifference();
    if (mItemExplorerUnassigned)
        mItemExplorerUnassigned->RefreshAfterMaxOneItemDifference();
}

void MainWindow::ItemOpen(ItemWidget* itemWidget, bool grabFocus)
{
    if (mSplitterMain->count() > 0)
        mSplitterMain->replaceWidget(0, itemWidget);
    else
        mSplitterMain->addWidget(itemWidget);  // This is necessary for the first call to ItemOpen.

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
        auto parents = mData[itemID].Parents();
        for (auto parentID : parents)
            mItemParents->mList->addItem(mData[parentID].Need());
    }
}

bool MainWindow::HasOnlyEmptyItem() { return mItemsOpen.size() == 1 && mItemsOpen[0]->IsEmpty(); }

void MainWindow::ItemCloseCurrent() { ItemCloseCurrent(true); }

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
                              [itemID](ItemWidget* x) { return x->ItemID() == itemID; });
    return found == mItemsOpen.end() ? nullptr : *found;
}

ItemWidget* MainWindow::ItemFindEmpty()
{
    auto found = std::find_if(mItemsOpen.begin(), mItemsOpen.end(),
                              [](ItemWidget* x) { return x->IsEmpty(); });
    return found == mItemsOpen.end() ? nullptr : *found;
}

void MainWindow::ItemExplorerShow()
{
    if (mItemExplorer->isVisible())
    {
        if (mItemExplorer->isActiveWindow())
            this->activateWindow();
        else
            mItemExplorer->activateWindow();
    }
    else
    {
        mItemExplorer->show();
        mItemExplorer->activateWindow();
        mItemExplorerUnassigned->hide();
    }
}

void MainWindow::ItemExplorerUnassignedShow()
{
    if (mItemExplorerUnassigned->isVisible())
    {
        if (mItemExplorerUnassigned->isActiveWindow())
            this->activateWindow();
        else
            mItemExplorerUnassigned->activateWindow();
    }
    else
    {
        mItemExplorerUnassigned->show();
        mItemExplorerUnassigned->activateWindow();
        mItemExplorer->hide();
    }
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
    auto parents = item.Parents();
    item.RemoveParent(parents[idx]);
    ItemParentsUpdate();

    // Update the items in explorer.
    mItemExplorer->RefreshAfterMaxOneItemDifference();
    mItemExplorerUnassigned->RefreshAfterMaxOneItemDifference();
}

void MainWindow::AddParent()
{
    if (mItemsOpen.isEmpty())
        return;

    bool activeMain = this->isActiveWindow();
    bool activeExplorer = mItemExplorer->isActiveWindow();
    bool activeUnassigned = mItemExplorerUnassigned->isActiveWindow();

    // Show dialog which allows the selection of a parent.
    auto search =
        new ItemExplorer("ExplorerSearch", ItemExplorer::ExplorerType::Search, mData, this);
    search->setFont(this->font());
    search->setWindowTitle("Search need");
    if (search->exec() == QDialog::Accepted)
    {
        auto& item = mData[mItemsOpen.last()->ItemID()];
        item.AddParent(search->GetSelectedID());
        ItemParentsUpdate();

        delete search;

        // Update the items in explorers.
        mItemExplorer->RefreshAfterMaxOneItemDifference();
        mItemExplorerUnassigned->RefreshAfterMaxOneItemDifference();
    }

    if (activeMain)
        mSplitterMain->setFocus();
    if (activeExplorer)
        mItemExplorer->activateWindow();
    if (activeUnassigned)
        mItemExplorerUnassigned->activateWindow();
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
    if (mItemExplorer->isVisible() || mItemExplorerUnassigned->isVisible())
    {
        mItemExplorer->hide();
        mItemExplorerUnassigned->hide();
        mSplitterMain->replaceWidget(1, new QWidget());  // Hide the list of parents.
    }
    else
    {
        close();
    }
}

void MainWindow::SaveToMemoryTry(QPrivateSignal)
{
    SaveWindowPositions();

    if (!mItemsOpen.empty())
        mItemsOpen.last()->SaveToMemoryTry();
}

void MainWindow::ItemDeleted()
{
    // Update the items in explorers.
    mItemExplorer->RefreshAfterMaxOneItemDifference();
    mItemExplorerUnassigned->RefreshAfterMaxOneItemDifference();
}

void MainWindow::ItemOpenNew() { ItemOpenNew(true); }

void MainWindow::ItemDeleteCurrent() { ItemDeleteCurrent(true); }

void MainWindow::ItemDeleteCurrent(bool grabFocus)
{
    if (mItemsOpen.empty())
        return;

    if (HasOnlyEmptyItem())
        return;

    auto confirmDelete = new QMessageBox();
    confirmDelete->setWindowTitle("Azharja");
    confirmDelete->setText("Are you sure you want to delete this item?<br><br><b>" +
                           mData[mItemsOpen.last()->ItemID()].Need());
    confirmDelete->setTextFormat(Qt::TextFormat::RichText);
    confirmDelete->setFont(this->font());
    confirmDelete->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    auto pos = this->screen()->geometry().center();
    pos -= QPoint(confirmDelete->sizeHint().width() / 2, confirmDelete->sizeHint().height());
    confirmDelete->move(pos);
    if (confirmDelete->exec() != QMessageBox::Yes)
        return;

    mItemsOpen.last()->MarkItemForDeletion();
    mItemsOpen.last()->deleteLater();
    mItemsOpen.removeLast();

    if (mItemsOpen.size() == 0)
        ItemOpenNew(grabFocus);
    else
        ItemOpen(mItemsOpen.last(), grabFocus);
}

void MainWindow::ItemSwitchTabs()
{
    if (mItemsOpen.empty())
        return;

    mItemsOpen.last()->SwitchTabs();
}

void MainWindow::NeedChanged()
{
    mItemExplorer->RefreshAfterMaxOneItemDifference();
    mItemExplorerUnassigned->RefreshAfterMaxOneItemDifference();
}

void MainWindow::ItemFinder() {
    bool okPressed = false;
    QString message = "Enter the word you want to search for:";
    QString searchWord = QInputDialog::getText(this, "Azharja", message, QLineEdit::Normal, "", &okPressed);

    if (!okPressed || searchWord.isEmpty()) {
        return;
    }

    int totalCount = 0;
    const auto& items = mData.Items();
    for (auto it = items.begin(); it != items.end(); ++it) {
        Item* item = it.value();
        if (item) {
            QString journal = item->Journal();
            totalCount += journal.count(searchWord, Qt::CaseInsensitive);

            QString answer = item->Answer();
            totalCount += answer.count(searchWord, Qt::CaseInsensitive);
        }
    }

    QMessageBox::information(this, "Azharja",
                             QString("The word '%1' was found %2 times in the data.")
                                 .arg(searchWord)
                                 .arg(totalCount));
}

