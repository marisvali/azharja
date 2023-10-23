#ifndef ITEM_H
#define ITEM_H

#include <QMutex>
#include <QString>
#include <QVector>

class Data;  // Forward declaration.

// Thread-safe class.
class Item
{
public:
    int64_t ID() const;
    const QString Need() const;
    const QString Journal() const;
    const QString Answer() const;
    bool Solved() const;
    bool IsEmpty() const;
    QVector<int64_t> Children() const;
    int NrChildren() const;
    QVector<int64_t> Parents() const;
    int NrParents() const;

    bool SetNeed(const QString& str);
    bool SetJournal(const QString& str);
    bool SetAnswer(const QString& str);
    void AddParent(int64_t parentID);
    void RemoveParent(int64_t parentID);
    Data& GetData() { return mData; };

protected:
    friend class Data;

    static Item* LoadFromDisk(QString path, Data& parent);
    static void DeleteFromDisk(QString path, int64_t id);

    Item(Data& parent, int64_t id) : mData(parent), mID(id) {}
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
    mutable QRecursiveMutex mMutexMeta;
    mutable QRecursiveMutex mMutexJournal;
    mutable QRecursiveMutex mMutexAnswer;
    bool mDirtyMeta = false;
    bool mDirtyJournal = false;
    bool mDirtyAnswer = false;

    void SaveToDisk(QString path);
    void AddChild(int64_t childID);
    void RemoveChild(int64_t childID);
};

#endif  // ITEM_H
