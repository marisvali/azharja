#include "data.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDir>
#include <savedatathread.h>

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
    const QString DataFolder = "c:/Azharja-1.0/Data";
    const QString ItemsFolder = DataFolder + "/Items";
    const QString ConfigFile = DataFolder + "/config.xml";
};

int64_t Data::Item::ID()
{
    return mID;
}

const QString Data::Item::Need()
{ 
    QMutexLocker lock(&mMutexMeta);
    return mNeed;
}

const QString Data::Item::Journal()
{
    QMutexLocker lock(&mMutexJournal);
    return mJournal;
}

const QString Data::Item::Answer()
{
    QMutexLocker lock(&mMutexAnswer);
    return mAnswer;
}

bool Data::Item::Solved()
{ 
    QMutexLocker lock(&mMutexMeta);
    return mSolved;
}

bool Data::Item::SetNeed(const QString& str)
{
    QMutexLocker lock(&mMutexMeta);
    mNeed = str;
    mData.mItemsDirty.insert(mID);
    return true;
}

bool Data::Item::SetJournal(const QString& str)
{
    QMutexLocker lock(&mMutexJournal);
    mJournal = str;
    mData.mItemsDirty.insert(mID);
    return true;
}

bool Data::Item::SetAnswer(const QString& str)
{
    QMutexLocker lock(&mMutexAnswer);
    mAnswer = str;
    mData.mItemsDirty.insert(mID);
    return true;
}

Data::Item* Data::Item::LoadFromDisk(QString pathXml, Data& parent)
{
    auto newItem = new Item(parent, -1);
    
    // Meta info.
    QFile fileMeta(pathXml);
    if (!fileMeta.open(QIODevice::ReadOnly | QIODevice::Text))
        throw new std::exception(("Invalid path: " + pathXml).toUtf8().constData());
    
    QXmlStreamReader xml(&fileMeta);
    while (!xml.atEnd())
    {
        xml.readNext();
        if (!xml.isStartElement())
            continue;
        
        QString node = xml.name().toString();
        
        if (node == "ID")
            newItem->mID = xml.readElementText().toLongLong();
        
        else if (node == "need")
            newItem->mNeed = xml.readElementText();
        
        else if (node == "solved")
            newItem->mSolved = xml.readElementText().toInt() != 0;
        
        else if (node == "parent")
            newItem->mParentsIDs.push_back(xml.readElementText().toLongLong());
    }
    
    QString path = QFileInfo(pathXml).absolutePath();
    
    // Journal.
    QString pathJournal = path + "/" + QString::number(newItem->mID) + "-J.txt";
    QFile fileJournal(pathJournal);
    if (!fileJournal.open(QIODevice::ReadOnly | QIODevice::Text))
        throw new std::exception(("Invalid path: " + pathJournal).toUtf8().constData());
    
    QTextStream streamJournal(&fileJournal);
    newItem->mJournal = streamJournal.readAll();
    
    // Answer.
    QString pathAnswer = path + "/" + QString::number(newItem->mID) + "-A.txt";
    QFile fileAnswer(pathAnswer);
    if (!fileAnswer.open(QIODevice::ReadOnly | QIODevice::Text))
        throw new std::exception(("Invalid path: " + pathAnswer).toUtf8().constData());
    
    QTextStream streamAnswer(&fileAnswer);
    newItem->mAnswer = streamAnswer.readAll();
    
    return newItem;
}

void Data::Item::SaveToDisk(QString path)
{
    // Meta info.
    {
        QMutexLocker lock(&mMutexMeta);
        QString pathXml = QDir(path).filePath(QString::number(mID) + ".xml");
        QFile fileMeta(pathXml);
        if (!fileMeta.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            throw new std::exception(("Invalid path: " + pathXml).toUtf8().constData());
        
        QXmlStreamWriter xml(&fileMeta);
        xml.setAutoFormatting(true);
        xml.writeStartDocument();
        xml.writeStartElement("item");
        xml.writeTextElement("ID", QString::number(mID));
        xml.writeTextElement("need", mNeed);
        xml.writeTextElement("solved", mSolved ? "1" : "0");
        xml.writeStartElement("parents");
        for (auto parent: mParentsIDs)
            xml.writeTextElement("parent", QString::number(parent));
        xml.writeEndElement();
        xml.writeEndElement();
        xml.writeEndDocument();
    }
    
    // Journal.
    {
        QMutexLocker lock(&mMutexJournal);
        QString pathJournal = QDir(path).filePath(QString::number(mID) + "-J.txt");
        QFile fileJournal(pathJournal);
        if (!fileJournal.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            throw new std::exception(("Invalid path: " + pathJournal).toUtf8().constData());
        
        fileJournal.write(mJournal.toUtf8());
    }
    
    // Answer.
    {
        QMutexLocker lock(&mMutexAnswer);
        QString pathAnswer = QDir(path).filePath(QString::number(mID) + "-A.txt");
        QFile fileAnswer(pathAnswer);
        if (!fileAnswer.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            throw new std::exception(("Invalid path: " + pathAnswer).toUtf8().constData());
        
        fileAnswer.write(mAnswer.toUtf8());
    }
}

void Data::Item::DeleteFromDisk(QString path, int64_t id)
{
    QString pathXml = QDir(path).filePath(QString::number(id) + ".xml");
    QFile::remove(pathXml);
    QString pathJournal = QDir(path).filePath(QString::number(id) + "-J.txt");
    QFile::remove(pathJournal);
    QString pathAnswer = QDir(path).filePath(QString::number(id) + "-A.txt");
    QFile::remove(pathAnswer);
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
    if (mSaveDataThread)
    {
        mSaveDataThread->Stop();
        mSaveDataThread->wait();
    }
            
    for (auto& item: mItems)
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

void Data::SetDirty(int64_t itemID)
{
    QMutexLocker lock(&mMutexDirty);
    mItemsDirty.insert(itemID);
}

void Data::SaveToDisk()
{
    QSet<int64_t> itemsDirty;
    {
        QMutexLocker lock(&mMutexDirty);
        itemsDirty = mItemsDirty;
        mItemsDirty.clear();
    }
    
    for (auto itemID: qAsConst(itemsDirty))
        if (mItems.contains(itemID))
            mItems[itemID]->SaveToDisk(ItemsFolder);
        else
            Item::DeleteFromDisk(ItemsFolder, itemID);
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
            mItems[itemOld.mID]->mParentsIDs.push_back(itemsTags[tagParent]);
    }
    
    AfterLoad();
}

void Data::AfterLoad()
{
    auto itemTop = new Item(*this, GetItemTop());
    itemTop->mNeed = "Needs";
    mItems.insert(itemTop->mID, itemTop);
    
    mCurrentMaxID = 100000000000ll;
    for (auto& item: mItems)
    {
        // Create parent -> child connections.
        for (auto parent: item->Parents())
            mItems[parent]->mChildrenIDs.push_back(item->ID());
        
        // Handle top items.
        if (item->NoParent() && item->mID != GetItemTop())
            item->AddParent(itemTop->mID);
        
        // Get the current maximum ID.
        if (item->ID() > mCurrentMaxID)
            mCurrentMaxID = item->ID();
    }
    
    mSaveDataThread = new SaveDataThread(*this);
    mSaveDataThread->start();
}

Data::Item& Data::CreateNewItem()
{
    if (mItems.contains(mCurrentMaxID) && 
        mItems[mCurrentMaxID]->IsEmpty())
        return *mItems[mCurrentMaxID];
    
    ++mCurrentMaxID;
    auto newItem = new Item(*this, mCurrentMaxID);
    mItems.insert(newItem->ID(), newItem);
    mItemsDirty.insert(mCurrentMaxID);
    return *mItems[mCurrentMaxID];
}

void Data::DeleteItem(int64_t itemID)
{
    auto item = mItems[itemID];
    if (item->Children().size() > 0)
        return;
        
    //remove the item as a child from its parents
    for (auto parent: item->Parents())
        mItems[parent]->mChildrenIDs.removeOne(itemID);
    
    delete item;
    mItems.remove(itemID);
    while (mCurrentMaxID > 100000000000ll && 
           !mItems.contains(mCurrentMaxID))
        --mCurrentMaxID;
    mItemsDirty.insert(itemID);
}
