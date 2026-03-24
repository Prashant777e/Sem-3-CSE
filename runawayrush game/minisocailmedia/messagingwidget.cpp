#include "messagingwidget.h"
#include "ui_messagingwidget.h"
#include "database.h"
#include <QMessageBox>

MessagingWidget::MessagingWidget(int userId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessagingWidget),
    currentUserId(userId),
    selectedFriendId(-1)
{
    ui->setupUi(this);

    loadFriendsList();

    // Auto-refresh every 3 seconds
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MessagingWidget::autoRefresh);
    refreshTimer->start(3000);
}

MessagingWidget::~MessagingWidget() {
    delete ui;
}

void MessagingWidget::loadFriendsList() {
    ui->friendsList->clear();

    QVector<int> friends = Database::instance().getFriends(currentUserId);

    for (int friendId : friends) {
        User* friendUser = Database::instance().getUserById(friendId);
        if (friendUser) {
            QListWidgetItem* item = new QListWidgetItem(
                "👤 " + friendUser->getFullName() + " (@" + friendUser->getUsername() + ")"
                );
            item->setData(Qt::UserRole, friendId);
            ui->friendsList->addItem(item);
            delete friendUser;
        }
    }

    if (friends.isEmpty()) {
        ui->chatDisplay->addItem("No friends yet! Add friends from Friends tab.");
    }
}

void MessagingWidget::on_friendsList_currentRowChanged(int row) {
    if (row < 0) {
        selectedFriendId = -1;
        return;
    }

    QListWidgetItem* item = ui->friendsList->item(row);
    selectedFriendId = item->data(Qt::UserRole).toInt();

    loadConversation(selectedFriendId);
}

void MessagingWidget::loadConversation(int friendId) {
    ui->chatDisplay->clear();

    QVector<Message> messages = Database::instance().getConversation(currentUserId, friendId);

    for (const Message& msg : messages) {
        QString displayText;
        if (msg.senderId == currentUserId) {
            displayText = "You: " + msg.messageText;
        } else {
            displayText = msg.senderName + ": " + msg.messageText;
        }
        displayText += " (" + msg.timestamp.toString("hh:mm") + ")";

        QListWidgetItem* item = new QListWidgetItem(displayText);

        // Style messages differently
        if (msg.senderId == currentUserId) {
            item->setTextAlignment(Qt::AlignRight);
            item->setBackground(QColor(220, 248, 198)); // Light green
        } else {
            item->setBackground(QColor(255, 255, 255)); // White
        }

        ui->chatDisplay->addItem(item);
    }

    // Scroll to bottom
    ui->chatDisplay->scrollToBottom();
}

void MessagingWidget::on_sendButton_clicked() {
    if (selectedFriendId < 0) {
        QMessageBox::warning(this, "Error", "Please select a friend first!");
        return;
    }

    QString message = ui->messageInput->toPlainText().trimmed();
    if (message.isEmpty()) {
        QMessageBox::warning(this, "Error", "Message cannot be empty!");
        return;
    }

    if (Database::instance().sendDirectMessage(currentUserId, selectedFriendId, message)) {
        ui->messageInput->clear();
        loadConversation(selectedFriendId);
    } else {
        QMessageBox::critical(this, "Error", "Failed to send message!");
    }
}

void MessagingWidget::autoRefresh() {
    if (selectedFriendId > 0) {
        loadConversation(selectedFriendId);
    }
}

void MessagingWidget::refreshMessages() {
    loadFriendsList();
    if (selectedFriendId > 0) {
        loadConversation(selectedFriendId);
    }
}
