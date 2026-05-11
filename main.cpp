#include "mainwindow.h"
#include <QApplication>
#include <QFont>
#include <QString>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Deadlock Detection Tool");
    a.setOrganizationName("CS-2006 OS Project");

    QFont font("Monospace", 10);
    font.setStyleHint(QFont::Monospace);
    a.setFont(font);

    MainWindow w;
    w.show();
    return a.exec();
}
