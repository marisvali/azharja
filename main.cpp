#include "mainwindow.h"

#include "qtsingleapplication.h"

int main(int argc, char *argv[])
{
    QtSingleApplication app(argc, argv);

    if (app.isRunning())
        return !app.sendMessage("Wake up!");
    
    MainWindow w;
    app.setActivationWindow(&w);
    w.show();
    return app.exec();
}
