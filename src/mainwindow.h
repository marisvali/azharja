#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "data.h"
#include "itemexplorer.h"
#include "itemparentswidget.h"
#include "itemwidget.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QSplitter>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  QSettings mSettings;
  Data mData;
  QSplitter *mSplitterMain = nullptr;
  QVector<ItemWidget *> mItemsOpen;
  ItemExplorer *mItemExplorer = nullptr;
  ItemExplorer *mItemExplorerUnassigned = nullptr;
  ItemParentsWidget *mItemParents = nullptr;
  QMessageBox *mWaitForSave = nullptr;
  QTimer *mTimerSaveToMemory;
  bool mCloseInitiated = false;
  bool mCloseFromSystemTray = false;

  bool HasOnlyEmptyItem();
  ItemWidget *ItemOpenGetter(int64_t itemID);
  ItemWidget *ItemFind(int64_t itemID);
  ItemWidget *ItemFindEmpty();
  void closeEvent(QCloseEvent *event);
  void ItemParentsUpdate();
  void ItemOpen(ItemWidget *itemWidget, bool grabFocus = true);

private slots:
  void ItemCloseCurrent();
  void ItemCloseCurrent(bool grabFocus);
  void ItemExplorerShow();
  void ItemExplorerUnassignedShow();
  void ItemOpen(int64_t itemID, bool grabFocus = true);
  void ItemParentsShow();
  void ParentDelete();
  void AddParent();
  void ItemCurrentFocus();
  void CloseExtraWindows();
  void SaveToMemoryTry(QPrivateSignal);
  void ItemDeleted();
  void DoneWithLastSave();
  void ItemOpenNew();
  void ItemOpenNew(bool grabFocus);
  void ItemDeleteCurrent();
  void ItemDeleteCurrent(bool grabFocus);
  void ItemSwitchTabs();
  void NeedChanged();

private:
  Ui::MainWindow *mUI;
};
#endif // MAINWINDOW_H
