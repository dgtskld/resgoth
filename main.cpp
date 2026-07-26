#include <QApplication>

#include "main_window.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QApplication::setOrganizationName("Resgoth");
    QApplication::setApplicationName("Resgoth");

    MainWindow window;
    window.show();
    return QApplication::exec();
}
