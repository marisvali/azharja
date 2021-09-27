#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <data.h>
#include <QSplitter>
#include <dlgitemexplore.h>
#include <itemwidget.h>
#include <itemparentswidget.h>
#include <QMessageBox>

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
    QSplitter* mSplitterMain = nullptr;
    QVector<ItemWidget*> mItemsOpen;
    DlgItemExplore* mItemExplore;
    ItemParentsWidget* mItemParents = nullptr;
    QMessageBox* mWaitForSave = nullptr;
    QTimer* mTimerSaveToMemory;
    bool mCloseInitiated = false;
    
    bool HasOnlyEmptyItem();
    ItemWidget* ItemOpenGetter(int64_t itemID);
    ItemWidget* ItemFind(int64_t itemID);
    ItemWidget* ItemFindEmpty();
    void closeEvent(QCloseEvent *event);
    void ItemParentsUpdate();
    void ItemOpen(ItemWidget* itemWidget, bool grabFocus = true);
    
private slots:
    void ItemCloseCurrent();
    void ItemCloseCurrent(bool grabFocus);
    void ItemExploreShow();
    void ItemOpen(int64_t itemID, bool grabFocus = true);
    void ItemParentsShow();
    void ParentDelete();
    void AddParent(int64_t itemID);
    void ItemCurrentFocus();
    void CloseExtraWindows();
    void SaveToMemoryTry(QPrivateSignal);
    void ItemDeleted();
    void DoneWithLastSave();
    void ItemOpenNew();
    void ItemOpenNew(bool grabFocus);
    
private:
    Ui::MainWindow *mUI;
};
#endif // MAINWINDOW_H
