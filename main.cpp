#include "mainwindow.h"

#include "ext/Qt-Frameless-Window-DarkStyle-master/DarkStyle.h"
#include "framelesswindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle(new DarkStyle);

    FramelessWindow framelessWindow;
    framelessWindow.setWindowTitle("TensorVox");
    framelessWindow.setWindowIcon(QIcon("://res/stdico.png"));

    MainWindow *mainWindow = new MainWindow;

    framelessWindow.setContent(mainWindow);
    framelessWindow.show();


    return a.exec();
}
