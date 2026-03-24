#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "user.h"
#include "graph.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MessagingWidget;
class PostWidget;
class ConnectionWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(int userId, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void refreshAll();

private:
    void setupUI();
    void loadUserData();

    Ui::MainWindow *ui;
    int currentUserId;
    User* currentUser;
    Graph friendGraph;

    MessagingWidget* messagingWidget;
    PostWidget* postWidget;
    ConnectionWidget* connectionWidget;
};

#endif // MAINWINDOW_H
