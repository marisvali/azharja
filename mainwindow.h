#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <data.h>
#include <QSplitter>
#include <dlgitemexplore.h>
#include <itemwidget.h>
#include <itemparentswidget.h>

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
    Data mData;
    ItemWidget* mItemCurrentWidget = nullptr;
    QSplitter* mSplitterMain = nullptr;
    QVector<ItemWidget*> mItemsOpen;
    DlgItemExplore* mItemExplore;
    ItemParentsWidget* mItemParents;
    
    bool HasOnlyEmptyItem();
    ItemWidget* ItemOpenGetter(int64_t itemID);
    ItemWidget* ItemFind(int64_t itemID);
    ItemWidget* ItemFindEmpty();
    int ItemFind(ItemWidget* itemWidget);
    void closeEvent(QCloseEvent *event);
    void ItemParentsUpdate();
    
private slots:
    void ItemCloseCurrent();
    void ItemCloseCurrent(bool grabFocus);
    void ItemClose(int64_t itemID, bool grabFocus);
    void ItemNew();
    void ItemExploreShow();
    void ItemOpen(int64_t itemID, bool grabFocus = true);
    void ItemParentsShow();
    void ParentDelete();
    void AddParent(int64_t itemID);
    void ItemCurrentFocus();
    void CloseExtraWindows();
    
private:
    Ui::MainWindow *mUI;
};
#endif // MAINWINDOW_H
