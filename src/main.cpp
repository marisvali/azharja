#include "mainwindow.h"

#include "qtsingleapplication.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QtSingleApplication app(argc, argv);

    if (app.isRunning())
        return !app.sendMessage("Wake up!");
    
    int result = 0;
    try
    {
        MainWindow w;
        app.setActivationWindow(&w);
        w.show();
        result = app.exec();
    }
    catch (std::exception& ex)
    {
        std::cout << ex.what();
    }

    return result;
}
