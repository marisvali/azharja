#include "data.h"
#include "datasavethread.h"
#include <QDir>
#include <QFile>
#include <QXmlStreamReader>
#include <thread>

class DataOld {
public:
  class Item {
  public:
    int64_t mID = -1;
    QString mNeed;
    QString mJournal;
    QString mAnswer;
    QVector<QString> mTags;

    void LoadFromDisk(QString path) {
      QFile file(path);
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw new std::runtime_error(
            ("Invalid path: " + path).toUtf8().constData());

      QXmlStreamReader xml(&file);
      while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement())
          continue;

        QString node = xml.name().toString();

        if (node == "need")
          mNeed = xml.readElementText();

        else if (node == "journal")
          mJournal = xml.readElementText();

        else if (node == "answer")
          mAnswer = xml.readElementText();

        else if (node == "ID" && mID < 0)
          mID = xml.readElementText().toLongLong();

        else if (node == "tags") {
          while (!xml.atEnd()) {
            xml.readNext();
            QString node = xml.name().toString();

            if (xml.isEndElement() && node == "tags")
              break;

            if (!xml.isStartElement())
              continue;

            if (node != "tag")
              continue;

            mTags.push_back(xml.attributes().value("name").toString());
          }
        }
      }
    }
  };

  const QString BackupFolder = "Backup";

  void LoadFromDisk() {
    QDir dir(ItemsFolder);
    QStringList itemNames =
        dir.entryList(QStringList() << "*.xml", QDir::Files);
    for (auto &itemName : itemNames) {
      Item item;
      item.LoadFromDisk(ItemsFolder + "/" + itemName);
      Items.push_back(item);
    }
  }

  std::vector<Item> Items;

private:
  const QString DataFolder = "c:/Azharja-1.0/Data";
  const QString ItemsFolder = DataFolder + "/Items";
  const QString ConfigFile = DataFolder + "/config.xml";
};

Data::~Data() {
  JustOneMoreSave();

  if (mSaveDataThread)
    mSaveDataThread->wait();

  for (auto &item : mItems)
    delete item;
}

void Data::JustOneMoreSave() {
  if (mSaveDataThread)
    mSaveDataThread->Stop();
}

QString Data::ItemsFolderPath() {
  return QDir(mDataFolder).filePath(ItemsFolder);
}

void Data::LoadFromDisk(QString dataFolder) {
  mDataFolder = dataFolder;
  QDir dir(ItemsFolderPath());

  QStringList itemNames = dir.entryList(QStringList() << "*.xml", QDir::Files);
  for (auto &itemName : itemNames) {
    auto item = Item::LoadFromDisk(dir.filePath(itemName), *this);
    InsertItem(item);
  }

  AfterLoad();
}

void Data::SetDirty(int64_t itemID) {
  QMutexLocker lock(&mMutexDirty);
  mItemsDirty.insert(itemID);
}

// This operation has to be thread-safe with regards to all other operations.
// Importantly, the item->SaveToDisk call must be thread-safe against the
// "delete item" call.
void Data::SaveToDisk() {
  QSet<int64_t> itemsDirty;
  {
    QMutexLocker lock(&mMutexDirty);
    itemsDirty = mItemsDirty;
    mItemsDirty.clear();
  }

  for (auto itemID : qAsConst(itemsDirty)) {
    // Lock the items so nothing gets deleted or inserted while we save.
    // The item can be modified while saving, this is thread-safe.
    QMutexLocker lock(&mMutexSave);

    if (mItems.contains(itemID))
      mItems[itemID]->SaveToDisk(ItemsFolderPath());
    else
      Item::DeleteFromDisk(ItemsFolderPath(), itemID);
  }
}

void Data::LoadFromDiskOld() {
  DataOld dataOld;
  dataOld.LoadFromDisk();

  int64_t maxID = 0;
  for (auto &itemOld : dataOld.Items)
    if (itemOld.mID > maxID)
      maxID = itemOld.mID;

  QMap<QString, int64_t> itemsTags;
  for (auto &item : dataOld.Items)
    for (auto &tag : item.mTags)
      if (!itemsTags.contains(tag)) {
        auto item = new Item(*this, ++maxID);
        item->mDirtyMeta = item->mDirtyJournal = item->mDirtyAnswer = true;
        item->mNeed = tag;
        InsertItem(item);
        mItemsDirty.insert(item->ID());
        itemsTags.insert(tag, item->mID);
        item->mParentsIDs.push_back(Data::GetItemTop());
      }

  for (auto &itemOld : dataOld.Items) {
    auto item = new Item(*this, itemOld.mID);
    item->mNeed = itemOld.mNeed;
    item->mJournal = itemOld.mJournal;
    item->mAnswer = itemOld.mAnswer;
    item->mDirtyMeta = item->mDirtyJournal = item->mDirtyAnswer = true;
    InsertItem(item);
    mItemsDirty.insert(item->ID());

    for (auto &tagParent : itemOld.mTags)
      item->mParentsIDs.push_back(itemsTags[tagParent]);
  }

  AfterLoad();
}

void Data::AfterLoad() {
  auto itemTop = new Item(*this, GetItemTop());
  itemTop->mNeed = "Needs";
  InsertItem(itemTop);

  mCurrentMaxID = 100000000000ll;
  for (auto &item : mItems) {
    // Create parent -> child connections.
    for (auto parent : item->mParentsIDs)
      mItems[parent]->mChildrenIDs.push_back(item->ID());

    // Get the current maximum ID.
    if (item->ID() > mCurrentMaxID)
      mCurrentMaxID = item->ID();
  }

  mSaveDataThread = new DataSaveThread(*this);
  connect(mSaveDataThread, SIGNAL(finished()), this,
          SIGNAL(DoneWithLastSave()));
  mSaveDataThread->start();
}

Item &Data::CreateNewItem() {
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

void Data::DeleteItem(int64_t itemID) {
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

  // Update mCurrentMaxID as new item IDs may have become available (if the last
  // item was deleted, for example).
  while (mCurrentMaxID > 100000000000ll && !mItems.contains(mCurrentMaxID))
    --mCurrentMaxID;
  mItemsDirty.insert(itemID);
}

void Data::InsertItem(Item *item) {
  // Lock so we don't modify mItems while it's being read by the save function.
  QMutexLocker lock(&mMutexSave);
  mItems.insert(item->ID(), item);
}