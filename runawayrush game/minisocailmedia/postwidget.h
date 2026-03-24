#ifndef POSTWIDGET_H
#define POSTWIDGET_H

#include <QWidget>

namespace Ui {
class PostWidget;
}

class PostWidget : public QWidget {
    Q_OBJECT

public:
    explicit PostWidget(int userId, QWidget *parent = nullptr);
    ~PostWidget();

    void refreshFeed();

private slots:
    void on_createPostButton_clicked();
    void on_refreshButton_clicked();

private:
    void loadFeed();

    Ui::PostWidget *ui;
    int currentUserId;
};

#endif // POSTWIDGET_H
