#include "connectionwidget.h"
#include "ui_connectionwidget.h"
#include "database.h"
#include <QMessageBox>
#include <QInputDialog>

ConnectionWidget::ConnectionWidget(int userId, Graph* graph, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConnectionWidget),
    currentUserId(userId),
    friendGraph(graph)
{
    ui->setupUi(this);
    loadFriends();
    loadRequests();
    loadSuggestions();
}

ConnectionWidget::~ConnectionWidget() {
    delete ui;
}

void ConnectionWidget::loadFriends() {
    ui->friendsList->clear();

    QVector<int> friends = Database::instance().getFriends(currentUserId);

    for (int friendId : friends) {
        User* friendUser = Database::instance().getUserById(friendId);
        if (friendUser) {
            QString displayText = QString("👤 %1 (@%2)")
                                      .arg(friendUser->getFullName())
                                      .arg(friendUser->getUsername());

            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, friendId);
            ui->friendsList->addItem(item);
            delete friendUser;
        }
    }

    ui->friendsList->insertItem(0, QString("Total Friends: %1").arg(friends.size()));
    ui->friendsList->item(0)->setBackground(QColor(200, 200, 200));
    ui->friendsList->item(0)->setFlags(Qt::NoItemFlags);
}

void ConnectionWidget::on_searchButton_clicked() {
    QString searchTerm = ui->searchEdit->text().trimmed();

    if (searchTerm.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a username to search!");
        return;
    }

    ui->searchResults->clear();

    User* foundUser = Database::instance().getUserByUsername(searchTerm);

    if (!foundUser) {
        ui->searchResults->addItem("❌ User not found!");
        return;
    }

    if (foundUser->getUserId() == currentUserId) {
        ui->searchResults->addItem("❌ You cannot add yourself!");
        delete foundUser;
        return;
    }

    // Check if already friends
    if (friendGraph->hasEdge(currentUserId, foundUser->getUserId())) {
        ui->searchResults->addItem("✅ Already friends with " + foundUser->getFullName());
        delete foundUser;
        return;
    }

    QString displayText = QString("👤 %1 (@%2)")
                              .arg(foundUser->getFullName())
                              .arg(foundUser->getUsername());

    QListWidgetItem* item = new QListWidgetItem(displayText);
    item->setData(Qt::UserRole, foundUser->getUserId());
    ui->searchResults->addItem(item);

    // Show mutual friends
    std::vector<int> mutual = friendGraph->getMutualFriends(currentUserId, foundUser->getUserId());
    if (!mutual.empty()) {
        ui->searchResults->addItem(QString("🤝 %1 mutual friends").arg(mutual.size()));
    }

    delete foundUser;
}

void ConnectionWidget::on_sendRequestButton_clicked() {
    QListWidgetItem* item = ui->searchResults->currentItem();

    if (!item || item->data(Qt::UserRole).toInt() == 0) {
        QMessageBox::warning(this, "Error", "Please search and select a user first!");
        return;
    }

    int targetUserId = item->data(Qt::UserRole).toInt();

    if (Database::instance().sendFriendRequest(currentUserId, targetUserId)) {
        QMessageBox::information(this, "Success", "Friend request sent!");
        ui->searchResults->clear();
        ui->searchEdit->clear();
    } else {
        QMessageBox::warning(this, "Error", "Failed to send request. Maybe already sent?");
    }
}

void ConnectionWidget::loadRequests() {
    ui->requestsList->clear();

    QVector<FriendRequest> requests = Database::instance().getPendingRequests(currentUserId);

    for (const FriendRequest& req : requests) {
        QString displayText = QString("👤 %1 wants to be your friend\n🕒 %2")
                                  .arg(req.senderName)
                                  .arg(req.timestamp.toString("MMM dd, yyyy"));

        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, req.requestId);
        ui->requestsList->addItem(item);
    }

    if (requests.isEmpty()) {
        ui->requestsList->addItem("No pending friend requests");
    }
}

void ConnectionWidget::on_acceptButton_clicked() {
    QListWidgetItem* item = ui->requestsList->currentItem();

    if (!item || item->data(Qt::UserRole).toInt() == 0) {
        QMessageBox::warning(this, "Error", "Please select a request first!");
        return;
    }

    int requestId = item->data(Qt::UserRole).toInt();

    if (Database::instance().acceptFriendRequest(requestId)) {
        QMessageBox::information(this, "Success", "Friend request accepted!");
        friendGraph->rebuildFromDatabase();
        refreshConnections();
    } else {
        QMessageBox::critical(this, "Error", "Failed to accept request!");
    }
}

void ConnectionWidget::on_rejectButton_clicked() {
    QListWidgetItem* item = ui->requestsList->currentItem();

    if (!item || item->data(Qt::UserRole).toInt() == 0) {
        QMessageBox::warning(this, "Error", "Please select a request first!");
        return;
    }

    int requestId = item->data(Qt::UserRole).toInt();

    int result = QMessageBox::question(this, "Confirm",
                                       "Are you sure you want to reject this request?");

    if (result == QMessageBox::Yes) {
        if (Database::instance().rejectFriendRequest(requestId)) {
            QMessageBox::information(this, "Success", "Request rejected!");
            loadRequests();
        }
    }
}

void ConnectionWidget::on_removeFriendButton_clicked() {
    QListWidgetItem* item = ui->friendsList->currentItem();

    if (!item || item->data(Qt::UserRole).toInt() == 0) {
        QMessageBox::warning(this, "Error", "Please select a friend first!");
        return;
    }

    int friendId = item->data(Qt::UserRole).toInt();
    User* friendUser = Database::instance().getUserById(friendId);

    if (!friendUser) return;

    int result = QMessageBox::question(this, "Confirm",
                                       QString("Remove %1 from friends?").arg(friendUser->getFullName()));

    delete friendUser;

    if (result == QMessageBox::Yes) {
        if (Database::instance().removeFriend(currentUserId, friendId)) {
            QMessageBox::information(this, "Success", "Friend removed!");
            friendGraph->rebuildFromDatabase();
            refreshConnections();
        }
    }
}

void ConnectionWidget::loadSuggestions() {
    ui->suggestionsList->clear();

    std::vector<int> suggestions = friendGraph->getSuggestedFriends(currentUserId);

    for (int suggestedId : suggestions) {
        User* suggestedUser = Database::instance().getUserById(suggestedId);
        if (suggestedUser) {
            std::vector<int> mutual = friendGraph->getMutualFriends(currentUserId, suggestedId);

            QString displayText = QString("👤 %1 (@%2)\n🤝 %3 mutual friends")
                                      .arg(suggestedUser->getFullName())
                                      .arg(suggestedUser->getUsername())
                                      .arg(mutual.size());

            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, suggestedId);
            ui->suggestionsList->addItem(item);
            delete suggestedUser;
        }
    }

    if (suggestions.empty()) {
        ui->suggestionsList->addItem("No suggestions available. Add more friends!");
    }
}

void ConnectionWidget::on_tabWidget_currentChanged(int index) {
    if (index == 2) { // Requests tab
        loadRequests();
    } else if (index == 3) { // Suggestions tab
        loadSuggestions();
    }
}

void ConnectionWidget::refreshConnections() {
    loadFriends();
    loadRequests();
    loadSuggestions();
}
