#ifndef SAVEDATATHREAD_H
#define SAVEDATATHREAD_H

#include "data.h"
#include <QThread>

class DataSaveThread : public QThread {
  Q_OBJECT

public:
  DataSaveThread(Data &data) : mData(data) {}
  void Stop() { mDone = true; }

protected:
  Data &mData;
  std::atomic<bool> mDone = false;

  void run() override;
};

#endif // SAVEDATATHREAD_H
