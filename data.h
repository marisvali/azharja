#ifndef DATA_H
#define DATA_H

#include <QString>
#include <QVector>
#include <QMap>

class Data
{
public:
    static int64_t GetItemTop() { return 0; }
    
    class Item
    {
    public:
        int64_t mID = -1;
        QString mNeed;
        QString mJournal;
        QString mAnswer;
        bool mSolved = false;

        void LoadFromDisk(QString path);
        const QVector<int64_t>& Children() { return mChildrenIDs; }
        const QVector<int64_t>& Parents() { return mParentsIDs; }
        bool NoParent();
        
    protected:
        friend class Data;
        
        QVector<int64_t> mChildrenIDs;
        QVector<int64_t> mParentsIDs;
    };
    
    const QString BackupFolder = "Backup";

    void LoadFromDisk();
    void LoadFromDiskOld();
    const QMap<int64_t, Item>& Items();
    void AddParent(int64_t itemID, int64_t parentID);
    void RemoveParent(int64_t itemID, int64_t parentID);
    Item& operator[](int64_t itemID) { return mItems[itemID]; }

protected:
    const QString DataFolder = "Data";
    const QString ItemsFolder = DataFolder + "/Items";
    const QString ConfigFile = DataFolder + "/config.xml";
    
    QMap<int64_t, Item> mItems;
    
    void AfterLoad();
};

#endif // DATA_H
