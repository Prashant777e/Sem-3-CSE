#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>

class User {
public:
    User();
    User(int id, const QString& username, const QString& fullName, const QDateTime& created);

    // Getters
    int getUserId() const { return userId; }
    QString getUsername() const { return username; }
    QString getFullName() const { return fullName; }
    QDateTime getCreatedAt() const { return createdAt; }

    // Setters
    void setUserId(int id) { userId = id; }
    void setUsername(const QString& name) { username = name; }
    void setFullName(const QString& name) { fullName = name; }
    void setCreatedAt(const QDateTime& dt) { createdAt = dt; }

    bool isValid() const { return userId > 0; }

private:
    int userId;
    QString username;
    QString fullName;
    QDateTime createdAt;
};

#endif // USER_H
