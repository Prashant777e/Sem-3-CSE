#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"
#include "messagingwidget.h"
#include "postwidget.h"
#include "connectionwidget.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>

MainWindow::MainWindow(int userId, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentUserId(userId)
    , currentUser(nullptr)
{
    ui->setupUi(this);
    loadUserData();
    setupUI();

    // Rebuild friend graph
    friendGraph.rebuildFromDatabase();

    resize(659, 523);
}

MainWindow::~MainWindow() {
    delete ui;
    if (currentUser) delete currentUser;
}

void MainWindow::loadUserData() {
    currentUser = Database::instance().getUserById(currentUserId);
    if (currentUser) {
        setWindowTitle("Social Chat - " + currentUser->getFullName());
    }
}

void MainWindow::setupUI() {
    // Create menu bar
    QMenu* fileMenu = menuBar()->addMenu("&File");

    QAction* refreshAction = fileMenu->addAction("Refresh All");
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshAll);

    fileMenu->addSeparator();

    QAction* logoutAction = fileMenu->addAction("Logout");
    connect(logoutAction, &QAction::triggered, this, &QMainWindow::close);

    // Create widgets for each tab
    messagingWidget = new MessagingWidget(currentUserId, this);
    postWidget = new PostWidget(currentUserId, this);
    connectionWidget = new ConnectionWidget(currentUserId, &friendGraph, this);

    // Add to tab widget
    ui->tabWidget->clear();
    ui->tabWidget->addTab(messagingWidget, "💬 Messages");
    ui->tabWidget->addTab(postWidget, "📰 Feed");
    ui->tabWidget->addTab(connectionWidget, "👥 Friends");

    // Status bar
    statusBar()->showMessage("Welcome, " + currentUser->getFullName() + "!");
}

void MainWindow::refreshAll() {
    friendGraph.rebuildFromDatabase();
    messagingWidget->refreshMessages();
    postWidget->refreshFeed();
    connectionWidget->refreshConnections();
    statusBar()->showMessage("Refreshed!", 2000);
}
