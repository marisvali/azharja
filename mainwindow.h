#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPlainTextEdit>
#include <data.h>
#include <ScintillaEdit.h>

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
    class ItemEditTab: public QWidget
    {
    public:
        ItemEditTab(const Data::Item& item, QFont font);
        
        QLineEdit* mNeed = nullptr;
        ScintillaEdit* mJournal = nullptr;
        ScintillaEdit* mAnswer = nullptr;
        
        int64_t ItemID() { return mItemID; }
        bool IsEmpty();
    
    protected:
        int64_t mItemID = -1;
    };
    
    Data mData;
    int64_t mItemIDCurrent;
    
    void SetItemList(int64_t itemID);
    int64_t ItemIDSelected();
    void OpenItem(int64_t itemID);
    bool HasOnlyEmptyTab();
    void OpenItemSetFocus();
    
private slots:
    void on_ItemList_itemDoubleClicked(QListWidgetItem *item);
    
    void on_ItemEdit_tabCloseRequested(int index);
    
    void CloseCurrentTab();
    void NewTab();
    
private:
    Ui::MainWindow *mUI;
};
#endif // MAINWINDOW_H
