#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QVector>
#include <QDateTime>
#include "user.h"

// Message structure
struct Message {
    int messageId;
    int senderId;
    int receiverId;  // NULL for public posts
    QString senderName;
    QString receiverName;
    QString messageText;
    bool isPublic;
    QDateTime timestamp;
};

// Friend Request structure
struct FriendRequest {
    int requestId;
    int senderId;
    int receiverId;
    QString senderName;
    QString receiverName;
    QString status;
    QDateTime timestamp;
};

class Database {
public:
    static Database& instance();

    // Database setup
    bool initDatabase();
    void closeDatabase();

    // User operations
    bool registerUser(const QString& username, const QString& password, const QString& fullName);
    User* authenticateUser(const QString& username, const QString& password);
    User* getUserById(int userId);
    User* getUserByUsername(const QString& username);
    QVector<User*> getAllUsers();

    // Friend operations
    bool sendFriendRequest(int senderId, int receiverId);
    bool acceptFriendRequest(int requestId);
    bool rejectFriendRequest(int requestId);
    QVector<FriendRequest> getPendingRequests(int userId);
    QVector<int> getFriends(int userId);
    bool removeFriend(int userId, int friendId);

    // Message operations
    bool sendDirectMessage(int senderId, int receiverId, const QString& message);
    bool sendPublicPost(int userId, const QString& post);
    QVector<Message> getConversation(int userId1, int userId2);
    QVector<Message> getPublicPosts();
    QVector<Message> getFeedForUser(int userId); // Posts from friends

    QString getLastError() const { return lastError; }

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    QSqlDatabase db;
    QString lastError;

    void createTables();
};

#endif // DATABASE_H
