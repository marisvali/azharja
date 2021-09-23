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

void Data::LoadFromDisk()
{
    QDir dir(ItemsFolder);
    QStringList itemNames = dir.entryList(QStringList() << "*.xml", QDir::Files);
    for (auto& itemName: itemNames)
    {
        Item item;
        item.LoadFromDisk(ItemsFolder + "/" + itemName);
        Items.insert(item.mID, item);
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
    
    QMap<QString, Data::Item> itemsTags;
    for (auto& item: dataOld.Items)
        for (auto& tag: item.mTags)
            if (!itemsTags.contains(tag))
            {
                Data::Item item;
                item.mID = ++maxID;
                item.mNeed = tag;
                itemsTags.insert(tag, item);
            }
        
    for (auto& itemOld: dataOld.Items)
    {
        Data::Item item;
        item.mID = itemOld.mID;
        item.mNeed = itemOld.mNeed;
        item.mJournal = itemOld.mJournal;
        item.mAnswer = itemOld.mAnswer;
        for (auto& tagParent: itemOld.mTags)
        {
            itemsTags[tagParent].mChildrenIDs.push_back(item.mID);
            item.mParentsIDs.push_back(itemsTags[tagParent].mID);
        }
        
        Items.insert(item.mID, item);
    }
    
    for (auto& item: itemsTags)
        Items.insert(item.mID, item);
    
    AfterLoad();
}

void Data::AfterLoad()
{
    Item itemTop;
    itemTop.mID = GetItemTop();
    itemTop.mNeed = "Needs";
    for (auto& item: Items)
        if (item.mParentsIDs.size() == 0)
        {
            itemTop.mChildrenIDs.push_back(item.mID);
            item.mParentsIDs.push_back(itemTop.mID);
        }
    Items.insert(itemTop.mID, itemTop);
}
