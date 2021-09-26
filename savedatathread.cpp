#include "savedatathread.h"

void SaveDataThread::run()
{
    while (!mDone.load())
    {
        mData.SaveToDisk();
        msleep(1000);
    }
}
