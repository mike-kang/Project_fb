#include <QtGui/QApplication>
#include "mainwindow.h"
#include <unistd.h>

int main(int argc, char *argv[])
{
    const char* config = NULL;
    QApplication a(argc, argv);
    int opt = getopt(argc, argv, "c:");   
    MainWindow w;
    if(opt == 'c'){
      config = optarg;
    }
    
    w.runMainDelegator(config);
    w.show();
    
    return a.exec();
}
