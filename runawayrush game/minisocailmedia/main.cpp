#include "mainwindow.h"
#include "loginwindow.h"
#include "database.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Initialize database
    if (!Database::instance().initDatabase()) {
        QMessageBox::critical(nullptr, "Database Error",
                              "Failed to initialize database:\n" +
                                  Database::instance().getLastError());
        return -1;
    }

    // Create login window
    LoginWindow* loginWindow = new LoginWindow();

    QObject::connect(loginWindow, &LoginWindow::loginSuccessful,
                     [loginWindow](int userId) {

                         MainWindow* mainWindow = new MainWindow(userId);
                         mainWindow->show();

                         // Close and cleanup login window
                         loginWindow->close();
                         loginWindow->deleteLater();
                     });

    // Show login window (app starts here)
    loginWindow->show();

    return a.exec();
}
