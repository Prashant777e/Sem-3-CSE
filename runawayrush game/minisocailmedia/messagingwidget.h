#ifndef MESSAGINGWIDGET_H
#define MESSAGINGWIDGET_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class MessagingWidget;
}

class MessagingWidget : public QWidget {
    Q_OBJECT

public:
    explicit MessagingWidget(int userId, QWidget *parent = nullptr);
    ~MessagingWidget();

    void refreshMessages();

private slots:
    void on_friendsList_currentRowChanged(int row);
    void on_sendButton_clicked();
    void autoRefresh();

private:
    void loadFriendsList();
    void loadConversation(int friendId);

    Ui::MessagingWidget *ui;
    int currentUserId;
    int selectedFriendId;
    QTimer* refreshTimer;
};

#endif // MESSAGINGWIDGET_H
