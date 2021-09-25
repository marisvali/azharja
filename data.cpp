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

void Data::Item::LoadFromDisk(QString path)
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
        
        else if (node == "ID")
            mID = xml.readElementText().toLongLong();
        
        else if (node == "solved")
            mSolved = xml.readElementText().toInt() != 0;
    }
}

bool Data::Item::NoParent()
{
    return  mParentsIDs.size() == 0 || 
            (mParentsIDs.size() == 1 && 
            mParentsIDs[0] == Data::GetItemTop());
}

void Data::LoadFromDisk()
{
    QDir dir(ItemsFolder);
    QStringList itemNames = dir.entryList(QStringList() << "*.xml", QDir::Files);
    for (auto& itemName: itemNames)
    {
        Item item;
        item.LoadFromDisk(ItemsFolder + "/" + itemName);
        mItems.insert(item.mID, item);
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
                Data::Item item;
                item.mID = ++maxID;
                item.mNeed = tag;
                mItems.insert(item.mID, item);
                itemsTags.insert(tag, item.mID);
            }
        
    for (auto& itemOld: dataOld.Items)
    {
        Data::Item item;
        item.mID = itemOld.mID;
        item.mNeed = itemOld.mNeed;
        item.mJournal = itemOld.mJournal;
        item.mAnswer = itemOld.mAnswer;
        mItems.insert(item.mID, item);
        
        for (auto& tagParent: itemOld.mTags)
            AddParent(item.mID, itemsTags[tagParent]);
    }
    
    AfterLoad();
}

void Data::AfterLoad()
{
    Item itemTop;
    itemTop.mID = GetItemTop();
    itemTop.mNeed = "Needs";
    mItems.insert(itemTop.mID, itemTop);
    
    for (auto& item: mItems)
        if (item.NoParent() && item.mID != GetItemTop())
            AddParent(item.mID, itemTop.mID);
}

void Data::AddParent(int64_t itemID, int64_t parentID)
{
    if (mItems[itemID].NoParent())
        mItems[itemID].mParentsIDs.clear();
    
    if (!mItems[itemID].mParentsIDs.contains(parentID))
        mItems[itemID].mParentsIDs.push_back(parentID);
    
    if (!mItems[parentID].mChildrenIDs.contains(itemID))
        mItems[parentID].mChildrenIDs.push_back(itemID);
}

void Data::RemoveParent(int64_t itemID, int64_t parentID)
{
    auto& vec = mItems[itemID].mParentsIDs;
    vec.removeOne(parentID);
    if (vec.size() == 0)
        vec.push_back(GetItemTop());
    
    mItems[parentID].mChildrenIDs.removeOne(itemID);
}
