#include "ui/WindowManager.h"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Set application metadata
    QCoreApplication::setApplicationName("OctaDroid");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("OctaDroid Project");
    
    WindowManager win;
    win.show();
    
    return app.exec();
}
