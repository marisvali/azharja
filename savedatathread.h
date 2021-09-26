#ifndef SAVEDATATHREAD_H
#define SAVEDATATHREAD_H

#include <QThread>
#include <data.h>

class SaveDataThread : public QThread
{
    Q_OBJECT
    
public:
    SaveDataThread(Data& data): mData(data) {}
    void Stop() { mDone = true; }
    
protected:
    Data& mData;
    std::atomic<bool> mDone = false;
    
    void run() override;
};

#endif // SAVEDATATHREAD_H
