#include "data.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDir>

class DataOld
{
public:
    class Item
    {
    public:
        int64_t mID = -1;
        QString mNeed;
        QString mJournal;
        QString mAnswer;
        QVector<QString> mTags;

        void LoadFromDisk(QString path)
        {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                throw new std::exception(("Invalid path: " + path).toUtf8().constData());

            QXmlStreamReader xml(&file);
            while (!xml.atEnd())
            {
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
                
                else if (node == "tags")
                {
                    while (!xml.atEnd())
                    {
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

    void LoadFromDisk()
    {
        QDir dir(ItemsFolder);
        QStringList itemNames = dir.entryList(QStringList() << "*.xml", QDir::Files);
        for (auto& itemName: itemNames)
        {
            Item item;
            item.LoadFromDisk(ItemsFolder + "/" + itemName);
            Items.push_back(item);
        }
    }

    std::vector<Item> Items;

private:
    const QString DataFolder = "Data";
    const QString ItemsFolder = DataFolder + "/Items";
    const QString ConfigFile = DataFolder + "/config.xml";
};

void Data::Item::SetNeed(const QString& str)
{
    mNeed = str;
    mData.mItemsDirty.insert(mID);
}

void Data::Item::SetJournal(const QString& str)
{
    mJournal = str;
    mData.mItemsDirty.insert(mID);
}

void Data::Item::SetAnswer(const QString& str)
{
    mAnswer = str;
    mData.mItemsDirty.insert(mID);
}

Data::Item* Data::Item::LoadFromDisk(QString path, Data& parent)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw new std::exception(("Invalid path: " + path).toUtf8().constData());
    
    auto newItem = new Item(parent, -1);
    
    QXmlStreamReader xml(&file);
    while (!xml.atEnd())
    {
        xml.readNext();
        if (!xml.isStartElement())
            continue;

        QString node = xml.name().toString();
        
        if (node == "need")
            newItem->mNeed = xml.readElementText();
        
        else if (node == "journal")
            newItem->mJournal = xml.readElementText();
        
        else if (node == "answer")
            newItem->mAnswer = xml.readElementText();
        
        else if (node == "ID")
            newItem->mID = xml.readElementText().toLongLong();
        
        else if (node == "solved")
            newItem->mSolved = xml.readElementText().toInt() != 0;
        
        else if (node == "parent")
            newItem->mParentsIDs.push_back(xml.readElementText().toLongLong());
    }
    
    return newItem;
}

bool Data::Item::NoParent()
{
    return  mParentsIDs.size() == 0 || 
            (mParentsIDs.size() == 1 && 
            mParentsIDs[0] == Data::GetItemTop());
}

bool Data::Item::IsEmpty()
{
    return mNeed == "" && mJournal == "" && mAnswer == "";
}


void Data::Item::AddParent(int64_t parentID)
{
    if (NoParent())
        mParentsIDs.clear();
    
    if (!mParentsIDs.contains(parentID))
        mParentsIDs.push_back(parentID);
    
    if (!mData[parentID].mChildrenIDs.contains(mID))
        mData[parentID].mChildrenIDs.push_back(mID);
}

void Data::Item::RemoveParent(int64_t parentID)
{
    mParentsIDs.removeOne(parentID);
    if (mParentsIDs.size() == 0)
        mParentsIDs.push_back(GetItemTop());
    
    mData[parentID].mChildrenIDs.removeOne(mID);
}

Data::~Data()
{
    for (auto item: mItems)
        delete item;
}

void Data::LoadFromDisk()
{
    QDir dir(ItemsFolder);
    QStringList itemNames = dir.entryList(QStringList() << "*.xml", QDir::Files);
    for (auto& itemName: itemNames)
    {
        auto item = Item::LoadFromDisk(ItemsFolder + "/" + itemName, *this);
        mItems.insert(item->mID, item);
    }
    
    AfterLoad();
}
    
void Data::LoadFromDiskOld()
{
    DataOld dataOld;
    dataOld.LoadFromDisk();
    
    int64_t maxID = 0;
    for (auto& itemOld: dataOld.Items)
        if (itemOld.mID > maxID)
            maxID = itemOld.mID;
    
    QMap<QString, int64_t> itemsTags;
    for (auto& item: dataOld.Items)
        for (auto& tag: item.mTags)
            if (!itemsTags.contains(tag))
            {
                auto item = new Data::Item(*this, ++maxID);
                item->mNeed = tag;
                mItems.insert(item->mID, item);
                itemsTags.insert(tag, item->mID);
            }
        
    for (auto& itemOld: dataOld.Items)
    {
        auto item = new Data::Item(*this, itemOld.mID);
        item->mNeed = itemOld.mNeed;
        item->mJournal = itemOld.mJournal;
        item->mAnswer = itemOld.mAnswer;
        mItems.insert(item->mID, item);
        
        for (auto& tagParent: itemOld.mTags)
            mItems[itemOld.mID]->AddParent(itemsTags[tagParent]);
    }
    
    AfterLoad();
}

void Data::AfterLoad()
{
    auto itemTop = new Item(*this, GetItemTop());
    itemTop->mNeed = "Needs";
    mItems.insert(itemTop->mID, itemTop);
    
    for (auto& item: mItems)
    {
        if (item->NoParent() && item->mID != GetItemTop())
            item->AddParent(itemTop->mID);
        if (item->ID() > mCurrentMaxID)
            mCurrentMaxID = item->ID();
    }
}

Data::Item& Data::CreateNewItem()
{
    if (mItems[mCurrentMaxID]->IsEmpty())
        return *mItems[mCurrentMaxID];
    
    ++mCurrentMaxID;
    mItems.insert(mCurrentMaxID, new Item(*this, mCurrentMaxID));
    mItemsDirty.insert(mCurrentMaxID);
    return *mItems[mCurrentMaxID];
}

void Data::DeleteItem(int64_t itemID)
{
    delete mItems[itemID];
    mItems.remove(itemID);
    if (itemID == mCurrentMaxID)
        --mCurrentMaxID;
    mItemsDirty.insert(itemID);
}
