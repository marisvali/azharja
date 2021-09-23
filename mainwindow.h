#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPlainTextEdit>
#include <data.h>
#include <ScintillaEdit.h>
#include <QSplitter>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
protected:
    class ItemWidget: public QWidget
    {
    public:
        ItemWidget(const Data::Item& item, QFont font);
        
        QLineEdit* mNeed = nullptr;
        ScintillaEdit* mJournal = nullptr;
        ScintillaEdit* mAnswer = nullptr;
        
        int64_t ItemID() { return mItemID; }
        bool IsEmpty();
    
    protected:
        int64_t mItemID = -1;
    };
    
    Data mData;
    ItemWidget* mItemCurrentWidget = nullptr;
    int64_t mItemCurrentID = -1;
    QSplitter* mSplitterMain = nullptr;
    QListWidget* mItemList = nullptr;
    QVector<ItemWidget*> mItemsOpen;
    
    void ItemListUpdate(int64_t itemID);
    int64_t ItemIDSelected();
    void ItemOpen(int64_t itemID);
    bool HasOnlyEmptyItem();
    ItemWidget* ItemOpenGetter(int64_t itemID);
    ItemWidget* ItemFind(int64_t itemID);
    ItemWidget* ItemFindEmpty();
    int ItemFind(ItemWidget* itemWidget);
    
private slots:
    void on_ItemList_itemDoubleClicked(QListWidgetItem *item);
    
    void ItemCloseCurrent();
    void ItemNew();
    
private:
    Ui::MainWindow *mUI;
};
#endif // MAINWINDOW_H
