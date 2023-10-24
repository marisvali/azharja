#include "data.h"

#include <QDir>
#include <QFile>
#include <QXmlStreamReader>
#include <thread>

#include "datasavethread.h"

Data::~Data()
{
    JustOneMoreSave();

    if (mSaveDataThread)
        mSaveDataThread->wait();

    for (auto &item : mItems)
        delete item;
}

void Data::JustOneMoreSave()
{
    if (mSaveDataThread)
        mSaveDataThread->Stop();
}

int64_t Data::GetValidItemID(int64_t defaultID)
{
    if (mItems.contains(defaultID))
        return defaultID;
    else if (mItems.count() > 0)
        return mItems.firstKey();
    else
        return GetItemTop();
}

QString Data::ItemsFolderPath() { return QDir(mDataFolder).filePath(ItemsFolder); }

void Data::LoadFromDisk(QString dataFolder)
{
    mDataFolder = dataFolder;
    QDir dir(ItemsFolderPath());
    if (!dir.exists())
        QDir().mkpath(dir.path());

    QStringList itemNames = dir.entryList(QStringList() << "*.xml", QDir::Files);
    for (auto &itemName : itemNames)
    {
        auto item = Item::LoadFromDisk(dir.filePath(itemName), *this);
        InsertItem(item);
    }

    AfterLoad();
}

void Data::SetDirty(int64_t itemID)
{
    QMutexLocker lock(&mMutexDirty);
    mItemsDirty.insert(itemID);
}

// This operation has to be thread-safe with regards to all other operations.
// Importantly, the item->SaveToDisk call must be thread-safe against the
// "delete item" call.
void Data::SaveToDisk()
{
    QSet<int64_t> itemsDirty;
    {
        QMutexLocker lock(&mMutexDirty);
        itemsDirty = mItemsDirty;
        mItemsDirty.clear();
    }

    for (auto itemID : qAsConst(itemsDirty))
    {
        // Lock the items so nothing gets deleted or inserted while we save.
        // The item can be modified while saving, this is thread-safe.
        QMutexLocker lock(&mMutexSave);

        if (mItems.contains(itemID))
            mItems[itemID]->SaveToDisk(ItemsFolderPath());
        else
            Item::DeleteFromDisk(ItemsFolderPath(), itemID);
    }
}

void Data::AfterLoad()
{
    auto itemTop = new Item(*this, GetItemTop());
    itemTop->mNeed = "Needs";
    InsertItem(itemTop);

    mCurrentMaxID = 100000000000ll;
    for (auto &item : mItems)
    {
        // Create parent -> child connections.
        for (auto parent : item->mParentsIDs)
            mItems[parent]->mChildrenIDs.push_back(item->ID());

        // Get the current maximum ID.
        if (item->ID() > mCurrentMaxID)
            mCurrentMaxID = item->ID();
    }

    mSaveDataThread = new DataSaveThread(*this);
    connect(mSaveDataThread, SIGNAL(finished()), this, SIGNAL(DoneWithLastSave()));
    mSaveDataThread->start();
}

Item &Data::CreateNewItem()
{
    if (mItems.contains(mCurrentMaxID) && mItems[mCurrentMaxID]->IsEmpty())
        return *mItems[mCurrentMaxID];

    ++mCurrentMaxID;
    auto newItem = new Item(*this, mCurrentMaxID);
    newItem->mDirtyMeta = true;
    newItem->mDirtyAnswer = true;
    newItem->mDirtyJournal = true;
    InsertItem(newItem);
    QMutexLocker lock(&mMutexDirty);
    mItemsDirty.insert(newItem->ID());
    return *newItem;
}

void Data::DeleteItem(int64_t itemID)
{
    // Lock so we don't delete an item while it is saved.
    QMutexLocker lock(&mMutexSave);

    auto item = mItems[itemID];
    if (item->NrChildren() > 0)
        return;

    // Remove all parents at once instead of having them be removed
    // automaticallyone by one by RemoveChild;
    auto parents = item->mParentsIDs;
    item->mParentsIDs.clear();

    // Remove the item as a child from its parents.
    for (auto parent : parents)
        mItems[parent]->RemoveChild(itemID);

    // Actual deletion.
    mItems.remove(itemID);
    delete item;

    // Update mCurrentMaxID as new item IDs may have become available (if the
    // last item was deleted, for example).
    while (mCurrentMaxID > 100000000000ll && !mItems.contains(mCurrentMaxID))
        --mCurrentMaxID;
    mItemsDirty.insert(itemID);
}

void Data::InsertItem(Item *item)
{
    // Lock so we don't modify mItems while it's being read by the save
    // function.
    QMutexLocker lock(&mMutexSave);
    mItems.insert(item->ID(), item);
}
