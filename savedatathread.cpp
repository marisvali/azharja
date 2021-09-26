#include "savedatathread.h"

void SaveDataThread::run()
{
    while (true)
    {
        mData.SaveToDisk();
        
        // Wait for 1 sec but check frequently if we should be done.
        for (int idx = 0; idx < 50; ++idx)
        {
            if (mDone.load())
                return;
            
            msleep(20);
        }
    }
}
