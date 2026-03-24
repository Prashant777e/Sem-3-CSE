#include "postwidget.h"
#include "ui_postwidget.h"
#include "database.h"
#include <QMessageBox>
#include <QInputDialog>

PostWidget::PostWidget(int userId, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PostWidget),
    currentUserId(userId)
{
    ui->setupUi(this);
    loadFeed();
}

PostWidget::~PostWidget() {
    delete ui;
}

void PostWidget::on_createPostButton_clicked() {
    QString post = ui->postInput->toPlainText().trimmed();

    if (post.isEmpty()) {
        QMessageBox::warning(this, "Error", "Post cannot be empty!");
        return;
    }

    if (post.length() > 500) {
        QMessageBox::warning(this, "Error", "Post too long! Maximum 500 characters.");
        return;
    }

    if (Database::instance().sendPublicPost(currentUserId, post)) {
        QMessageBox::information(this, "Success", "Post created successfully!");
        ui->postInput->clear();
        loadFeed();
    } else {
        QMessageBox::critical(this, "Error", "Failed to create post!");
    }
}

void PostWidget::loadFeed() {
    ui->feedList->clear();

    // Get posts from friends and yourself
    QVector<Message> feed = Database::instance().getFeedForUser(currentUserId);

    if (feed.isEmpty()) {
        ui->feedList->addItem("No posts yet! Create your first post or add friends!");
        return;
    }

    for (const Message& post : feed) {
        QString displayText = QString("👤 %1\n📝 %2\n🕒 %3")
                                  .arg(post.senderName)
                                  .arg(post.messageText)
                                  .arg(post.timestamp.toString("MMM dd, yyyy hh:mm"));

        QListWidgetItem* item = new QListWidgetItem(displayText);

        // Highlight your own posts
        if (post.senderId == currentUserId) {
            item->setBackground(QColor(230, 230, 255)); // Light blue
            item->setForeground(QColor(0, 0, 128)); // Dark blue text
        }

        ui->feedList->addItem(item);

        // Add separator
        QListWidgetItem* separator = new QListWidgetItem("─────────────────────────────");
        separator->setFlags(Qt::NoItemFlags); // Not selectable
        ui->feedList->addItem(separator);
    }
}

void PostWidget::on_refreshButton_clicked() {
    loadFeed();
}

void PostWidget::refreshFeed() {
    loadFeed();
}
