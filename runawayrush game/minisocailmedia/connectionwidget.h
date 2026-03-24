#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include "graph.h"

namespace Ui {
class ConnectionWidget;
}

class ConnectionWidget : public QWidget {
    Q_OBJECT

public:
    explicit ConnectionWidget(int userId, Graph* graph, QWidget *parent = nullptr);
    ~ConnectionWidget();

    void refreshConnections();

private slots:
    void on_searchButton_clicked();
    void on_sendRequestButton_clicked();
    void on_acceptButton_clicked();
    void on_rejectButton_clicked();
    void on_removeFriendButton_clicked();
    void on_tabWidget_currentChanged(int index);

private:
    void loadFriends();
    void loadRequests();
    void loadSuggestions();
    void showMutualFriends();

    Ui::ConnectionWidget *ui;
    int currentUserId;
    Graph* friendGraph;
};

#endif // CONNECTIONWIDGET_H
