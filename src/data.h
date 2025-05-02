#ifndef DATA_H
#define DATA_H

#include <QMap>
#include <QMutex>
#include <QSet>
#include <QString>
#include <QThread>
#include <QVector>
#include <atomic>
#include <map>

#include "item.h"

class DataSaveThread;  // Forward declaration.

class Data : public QObject
{
    Q_OBJECT

public:
    static int64_t GetItemTop() { return 0; }

    Data() = default;
    Data(const Data &) = delete;
    Data &operator=(const Data &) = delete;
    ~Data();

    const QString BackupFolder = "Backup";

    void LoadFromDisk(QString dataFolder);
    void SaveToDisk();
    void LoadFromDiskOld();
    const QMap<int64_t, Item *> &Items() { return mItems; }
    Item &operator[](int64_t itemID) { return *mItems[itemID]; }
    Item &CreateNewItem();
    void DeleteItem(int64_t itemID);
    void SetDirty(int64_t itemID);
    void JustOneMoreSave();
    int64_t GetValidItemID(int64_t defaultID);

signals:
    void DoneWithLastSave();

protected:
    const QString ItemsFolder = "Items";
    const QString ConfigFile = "config.xml";

    QString mDataFolder = "Data";
    int64_t mCurrentMaxID = -1;
    QMap<int64_t, Item *> mItems;
    QSet<int64_t> mItemsDirty;
    DataSaveThread *mSaveDataThread = nullptr;
    QMutex mMutexDirty;
    QMutex mMutexSave;

    void AfterLoad();
    void InsertItem(Item *item);
    QString ItemsFolderPath();
};

#endif  // DATA_H
