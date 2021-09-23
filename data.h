#ifndef DATA_H
#define DATA_H

#include <QString>
#include <QVector>
#include <QMap>

class Data
{
public:
    class Item
    {
    public:
        int64_t mID = -1;
        QVector<int64_t> mChildrenIDs;
        QVector<int64_t> mParentsIDs;
        QString mNeed;
        QString mJournal;
        QString mAnswer;
        bool mSolved = false;

        void LoadFromDisk(QString path);
    };

    const QString BackupFolder = "Backup";

    void LoadFromDisk();
    
    void LoadFromDiskOld();
    
    int64_t GetItemTop() { return 0; }

    QMap<int64_t, Item> Items;

protected:
    const QString DataFolder = "Data";
    const QString ItemsFolder = DataFolder + "/Items";
    const QString ConfigFile = DataFolder + "/config.xml";
    
    void AfterLoad();
};

#endif // DATA_H
