#ifndef DATA_H
#define DATA_H

#include <QString>
#include <QVector>
#include <QSet>
#include <QMap>
#include <QThread>
#include <atomic>
#include <QMutex>

class SaveDataThread; // Foward declaration.

class Data
{
public:
    static int64_t GetItemTop() { return 0; }
    
    Data() = default;
    Data(const Data&) = delete;
    Data& operator=(const Data&) = delete;
    ~Data();
    
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
        static Data::Item* LoadFromDisk(QString path, Data& parent);
        static void DeleteFromDisk(QString path, int64_t id);
        
        Item(Data& parent, int64_t id): mData(parent), mID(id) {}
        Item(const Item&) = delete;
        Item& operator=(const Item&) = delete;
        
        friend class Data;
        
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
        
        void SaveToDisk(QString path);
    };
    
    const QString BackupFolder = "Backup";

    void LoadFromDisk();
    void SaveToDisk();
    void LoadFromDiskOld();
    const QHash<int64_t, Item>& Items();
    Item& operator[](int64_t itemID) { return *mItems[itemID]; }
    Item& CreateNewItem();
    void DeleteItem(int64_t itemID);
    void SetDirty(int64_t itemID);

protected:
    const QString DataFolder = "c:/Azharja/Data";
    const QString ItemsFolder = DataFolder + "/Items";
    const QString ConfigFile = DataFolder + "/config.xml";
    
    int64_t mCurrentMaxID = -1;
    QMap<int64_t, Item*> mItems;
    QSet<int64_t> mItemsDirty;
    SaveDataThread* mSaveDataThread = nullptr;
    QMutex mMutexDirty;

    void AfterLoad();
};

#endif // DATA_H
