#include "item.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDir>
#include <data.h>

class MutexTryLocker {
  QRecursiveMutex& mMutex;
  bool mLocked;
public:
  MutexTryLocker(QRecursiveMutex& mutex) : mMutex(mutex), mLocked(mutex.tryLock()) {}
  ~MutexTryLocker() { if (mLocked) mMutex.unlock(); }
  bool isLocked() const { return mLocked; }
};

int64_t Item::ID()
{
    QMutexLocker lock(&mMutexMeta);
    return mID;
}

const QString Item::Need()
{ 
    QMutexLocker lock(&mMutexMeta);
    return mNeed;
}

const QString Item::Journal()
{
    QMutexLocker lock(&mMutexJournal);
    return mJournal;
}

const QString Item::Answer()
{
    QMutexLocker lock(&mMutexAnswer);
    return mAnswer;
}

bool Item::Solved()
{ 
    QMutexLocker lock(&mMutexMeta);
    return mSolved;
}

bool Item::SetNeed(const QString& str)
{
    MutexTryLocker lock(mMutexMeta);
    if (!lock.isLocked())
        return false;
    
    mNeed = str;
    mDirtyMeta = true;
    mData.SetDirty(mID);
    return true;
}

bool Item::SetJournal(const QString& str)
{
    MutexTryLocker lock(mMutexJournal);
    if (!lock.isLocked())
        return false;
    
    mJournal = str;
    mDirtyJournal = true;
    mData.SetDirty(mID);
    return true;
}

bool Item::SetAnswer(const QString& str)
{
    MutexTryLocker lock(mMutexAnswer);
    if (!lock.isLocked())
        return false;
    
    mAnswer = str;
    mDirtyAnswer = true;
    mData.SetDirty(mID);
    return true;
}

Item* Item::LoadFromDisk(QString pathXml, Data& parent)
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
    
    newItem->mJournal = fileJournal.readAll();
    
    // Answer.
    QString pathAnswer = path + "/" + QString::number(newItem->mID) + "-A.txt";
    QFile fileAnswer(pathAnswer);
    if (!fileAnswer.open(QIODevice::ReadOnly | QIODevice::Text))
        throw new std::exception(("Invalid path: " + pathAnswer).toUtf8().constData());
    
    newItem->mAnswer = fileAnswer.readAll();
    
    return newItem;
}

// Thread safe operation.
void Item::SaveToDisk(QString path)
{
    // Meta info.
    {
        QMutexLocker lock(&mMutexMeta);
        if (mID == Data::GetItemTop())
            return;
        
        if (mDirtyMeta)
        {
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
        mDirtyMeta = false;
    }
    
    // Journal.
    {
        QMutexLocker lock(&mMutexJournal);
        if (mDirtyJournal)
        {
            QString pathJournal = QDir(path).filePath(QString::number(mID) + "-J.txt");
            QFile fileJournal(pathJournal);
            if (!fileJournal.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
                throw new std::exception(("Invalid path: " + pathJournal).toUtf8().constData());
            
            fileJournal.write(mJournal.toUtf8());    
        }
        mDirtyJournal = false;
    }
    
    // Answer.
    {
        QMutexLocker lock(&mMutexAnswer);
        if (mDirtyAnswer)
        {
            QString pathAnswer = QDir(path).filePath(QString::number(mID) + "-A.txt");
            QFile fileAnswer(pathAnswer);
            if (!fileAnswer.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
                throw new std::exception(("Invalid path: " + pathAnswer).toUtf8().constData());
            
            fileAnswer.write(mAnswer.toUtf8());
        }
        mDirtyAnswer = false;
    }
}

void Item::DeleteFromDisk(QString path, int64_t id)
{
    QString pathXml = QDir(path).filePath(QString::number(id) + ".xml");
    QFile::remove(pathXml);
    QString pathJournal = QDir(path).filePath(QString::number(id) + "-J.txt");
    QFile::remove(pathJournal);
    QString pathAnswer = QDir(path).filePath(QString::number(id) + "-A.txt");
    QFile::remove(pathAnswer);
}

bool Item::IsEmpty()
{
    QMutexLocker lock1(&mMutexMeta);
    QMutexLocker lock2(&mMutexJournal);
    QMutexLocker lock3(&mMutexAnswer);
    return mNeed == "" && mJournal == "" && mAnswer == "";
}

void Item::AddParent(int64_t parentID)
{
    QMutexLocker lock(&mMutexMeta);
    
    if (mParentsIDs.contains(parentID))
        return;
    
    // Add the parent to this item.
    mParentsIDs.push_back(parentID);
    mDirtyMeta = true;
    mData.SetDirty(mID);
    
    // Remove the top item as a parent.
    RemoveParent(Data::GetItemTop());
    
    // Add this item as a child to new parent.
    mData[parentID].AddChild(mID);
}

void Item::RemoveParent(int64_t parentID)
{
    QMutexLocker lock(&mMutexMeta);
    
    if (!mParentsIDs.removeOne(parentID))
        return;
    
    mDirtyMeta = true;
    mData.SetDirty(mID);
    
    // Add the top item as a parent if no other parents are left.
    if (mParentsIDs.size() == 0)
        this->AddParent(Data::GetItemTop());
    
    mData[parentID].RemoveChild(mID);
}

void Item::AddChild(int64_t childID)
{
    QMutexLocker lock(&mMutexMeta);
    if (mChildrenIDs.contains(childID))
        return;
    
    mChildrenIDs.push_back(childID);
    mDirtyMeta = true;
    mData.SetDirty(mID);
    
    mData[childID].AddParent(mID);
}

void Item::RemoveChild(int64_t childID)
{
    QMutexLocker lock(&mMutexMeta);
    if (!mChildrenIDs.removeOne(childID))
        return;
    
    mDirtyMeta = true;
    mData.SetDirty(mID);
    
    mData[childID].RemoveParent(mID);
}
