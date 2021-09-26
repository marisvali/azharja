#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QVector>
#include <QMutex>

class Data; // Forward declaration.

// Thread-safe class.
class Item
{
public:
    int64_t ID();
    const QString Need();
    const QString Journal();
    const QString Answer();
    bool Solved();
    bool SetNeed(const QString& str);
    bool SetJournal(const QString& str);
    bool SetAnswer(const QString& str);

    const QVector<int64_t>& Children() const { return mChildrenIDs; }
    const QVector<int64_t>& Parents() const { return mParentsIDs; }
    bool IsEmpty();
    void AddParent(int64_t parentID);
    void RemoveParent(int64_t parentID);
    Data& GetData() { return mData; };
    
protected:
    friend class Data;
    
    static Item* LoadFromDisk(QString path, Data& parent);
    static void DeleteFromDisk(QString path, int64_t id);
    
    Item(Data& parent, int64_t id): mData(parent), mID(id) {}
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;
    
    Data& mData;
    int64_t mID = -1;
    QString mNeed;
    QString mJournal;
    QString mAnswer;
    QVector<int64_t> mChildrenIDs;
    QVector<int64_t> mParentsIDs;
    bool mSolved = false;
    QMutex mMutexMeta;
    QMutex mMutexJournal;
    QMutex mMutexAnswer;
    bool mDirtyMeta = false;
    bool mDirtyJournal = false;
    bool mDirtyAnswer = false;
    
    void SaveToDisk(QString path);
    void AddChild(int64_t childID);
    void RemoveChild(int64_t childID);
};

#endif // ITEM_H
