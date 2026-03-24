#include "database.h"
#include <QDebug>
#include <QSqlRecord>
#include <QVariant>

Database::Database() {}
Database::~Database() { closeDatabase(); }

Database& Database::instance() {
    static Database instance;
    return instance;
}

bool Database::initDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("socialchat.db");

    if (!db.open()) {
        lastError = "Failed to open database: " + db.lastError().text();
        qDebug() << lastError;
        return false;
    }

    createTables();
    qDebug() << "Database initialized successfully!";
    return true;
}

void Database::createTables() {
    QSqlQuery query;

    // Users table
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "user_id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username TEXT UNIQUE NOT NULL, "
               "password TEXT NOT NULL, "
               "full_name TEXT NOT NULL, "
               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)");

    // Friends table (adjacency list)
    query.exec("CREATE TABLE IF NOT EXISTS friends ("
               "friendship_id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "user_id INTEGER NOT NULL, "
               "friend_id INTEGER NOT NULL, "
               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
               "UNIQUE(user_id, friend_id), "
               "FOREIGN KEY(user_id) REFERENCES users(user_id) ON DELETE CASCADE, "
               "FOREIGN KEY(friend_id) REFERENCES users(user_id) ON DELETE CASCADE)");

    // Friend requests
    query.exec("CREATE TABLE IF NOT EXISTS friend_requests ("
               "request_id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "sender_id INTEGER NOT NULL, "
               "receiver_id INTEGER NOT NULL, "
               "status TEXT DEFAULT 'pending', "
               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
               "UNIQUE(sender_id, receiver_id), "
               "FOREIGN KEY(sender_id) REFERENCES users(user_id) ON DELETE CASCADE, "
               "FOREIGN KEY(receiver_id) REFERENCES users(user_id) ON DELETE CASCADE)");

    // Messages table
    query.exec("CREATE TABLE IF NOT EXISTS messages ("
               "message_id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "sender_id INTEGER NOT NULL, "
               "receiver_id INTEGER, "
               "message_text TEXT NOT NULL, "
               "is_public INTEGER DEFAULT 0, "
               "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
               "FOREIGN KEY(sender_id) REFERENCES users(user_id) ON DELETE CASCADE, "
               "FOREIGN KEY(receiver_id) REFERENCES users(user_id) ON DELETE CASCADE)");

    // Create indexes
    query.exec("CREATE INDEX IF NOT EXISTS idx_friends_user ON friends(user_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_messages_sender ON messages(sender_id)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_messages_receiver ON messages(receiver_id)");

    qDebug() << "Tables created successfully!";
}

void Database::closeDatabase() {
    if (db.isOpen()) {
        db.close();
    }
}

// User operations
bool Database::registerUser(const QString& username, const QString& password, const QString& fullName) {
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, full_name) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.addBindValue(fullName);

    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

User* Database::authenticateUser(const QString& username, const QString& password) {
    QSqlQuery query;
    query.prepare("SELECT user_id, username, full_name, created_at FROM users "
                  "WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);

    if (query.exec() && query.next()) {
        return new User(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toDateTime()
            );
    }
    return nullptr;
}

User* Database::getUserById(int userId) {
    QSqlQuery query;
    query.prepare("SELECT user_id, username, full_name, created_at FROM users WHERE user_id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        return new User(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toDateTime()
            );
    }
    return nullptr;
}

User* Database::getUserByUsername(const QString& username) {
    QSqlQuery query;
    query.prepare("SELECT user_id, username, full_name, created_at FROM users WHERE username = ?");
    query.addBindValue(username);

    if (query.exec() && query.next()) {
        return new User(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toDateTime()
            );
    }
    return nullptr;
}

QVector<User*> Database::getAllUsers() {
    QVector<User*> users;
    QSqlQuery query("SELECT user_id, username, full_name, created_at FROM users");

    while (query.next()) {
        users.append(new User(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toString(),
            query.value(3).toDateTime()
            ));
    }
    return users;
}

// Friend operations
bool Database::sendFriendRequest(int senderId, int receiverId) {
    QSqlQuery query;
    query.prepare("INSERT INTO friend_requests (sender_id, receiver_id, status) VALUES (?, ?, 'pending')");
    query.addBindValue(senderId);
    query.addBindValue(receiverId);

    if (!query.exec()) {
        lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool Database::acceptFriendRequest(int requestId) {
    QSqlQuery query;

    // Get sender and receiver
    query.prepare("SELECT sender_id, receiver_id FROM friend_requests WHERE request_id = ?");
    query.addBindValue(requestId);

    if (!query.exec() || !query.next()) {
        return false;
    }

    int senderId = query.value(0).toInt();
    int receiverId = query.value(1).toInt();

    // Add bidirectional friendship
    query.prepare("INSERT INTO friends (user_id, friend_id) VALUES (?, ?), (?, ?)");
    query.addBindValue(senderId);
    query.addBindValue(receiverId);
    query.addBindValue(receiverId);
    query.addBindValue(senderId);

    if (!query.exec()) {
        return false;
    }

    // Update request status
    query.prepare("UPDATE friend_requests SET status = 'accepted' WHERE request_id = ?");
    query.addBindValue(requestId);
    return query.exec();
}

bool Database::rejectFriendRequest(int requestId) {
    QSqlQuery query;
    query.prepare("UPDATE friend_requests SET status = 'rejected' WHERE request_id = ?");
    query.addBindValue(requestId);
    return query.exec();
}

QVector<FriendRequest> Database::getPendingRequests(int userId) {
    QVector<FriendRequest> requests;
    QSqlQuery query;
    query.prepare("SELECT fr.request_id, fr.sender_id, fr.receiver_id, "
                  "u1.username, u2.username, fr.status, fr.created_at "
                  "FROM friend_requests fr "
                  "JOIN users u1 ON fr.sender_id = u1.user_id "
                  "JOIN users u2 ON fr.receiver_id = u2.user_id "
                  "WHERE fr.receiver_id = ? AND fr.status = 'pending' "
                  "ORDER BY fr.created_at DESC");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            FriendRequest req;
            req.requestId = query.value(0).toInt();
            req.senderId = query.value(1).toInt();
            req.receiverId = query.value(2).toInt();
            req.senderName = query.value(3).toString();
            req.receiverName = query.value(4).toString();
            req.status = query.value(5).toString();
            req.timestamp = query.value(6).toDateTime();
            requests.append(req);
        }
    }
    return requests;
}

QVector<int> Database::getFriends(int userId) {
    QVector<int> friends;
    QSqlQuery query;
    query.prepare("SELECT friend_id FROM friends WHERE user_id = ?");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            friends.append(query.value(0).toInt());
        }
    }
    return friends;
}

bool Database::removeFriend(int userId, int friendId) {
    QSqlQuery query;
    query.prepare("DELETE FROM friends WHERE "
                  "(user_id = ? AND friend_id = ?) OR (user_id = ? AND friend_id = ?)");
    query.addBindValue(userId);
    query.addBindValue(friendId);
    query.addBindValue(friendId);
    query.addBindValue(userId);
    return query.exec();
}

// Message operations
bool Database::sendDirectMessage(int senderId, int receiverId, const QString& message) {
    QSqlQuery query;
    query.prepare("INSERT INTO messages (sender_id, receiver_id, message_text, is_public) "
                  "VALUES (?, ?, ?, 0)");
    query.addBindValue(senderId);
    query.addBindValue(receiverId);
    query.addBindValue(message);
    return query.exec();
}

bool Database::sendPublicPost(int userId, const QString& post) {
    QSqlQuery query;
    query.prepare("INSERT INTO messages (sender_id, receiver_id, message_text, is_public) "
                  "VALUES (?, NULL, ?, 1)");
    query.addBindValue(userId);
    query.addBindValue(post);
    return query.exec();
}

QVector<Message> Database::getConversation(int userId1, int userId2) {
    QVector<Message> messages;
    QSqlQuery query;
    query.prepare("SELECT m.message_id, m.sender_id, m.receiver_id, "
                  "u1.username, COALESCE(u2.username, 'Public'), "
                  "m.message_text, m.is_public, m.created_at "
                  "FROM messages m "
                  "JOIN users u1 ON m.sender_id = u1.user_id "
                  "LEFT JOIN users u2 ON m.receiver_id = u2.user_id "
                  "WHERE (m.sender_id = ? AND m.receiver_id = ?) OR "
                  "(m.sender_id = ? AND m.receiver_id = ?) "
                  "ORDER BY m.created_at ASC");
    query.addBindValue(userId1);
    query.addBindValue(userId2);
    query.addBindValue(userId2);
    query.addBindValue(userId1);

    if (query.exec()) {
        while (query.next()) {
            Message msg;
            msg.messageId = query.value(0).toInt();
            msg.senderId = query.value(1).toInt();
            msg.receiverId = query.value(2).toInt();
            msg.senderName = query.value(3).toString();
            msg.receiverName = query.value(4).toString();
            msg.messageText = query.value(5).toString();
            msg.isPublic = query.value(6).toBool();
            msg.timestamp = query.value(7).toDateTime();
            messages.append(msg);
        }
    }
    return messages;
}

QVector<Message> Database::getPublicPosts() {
    QVector<Message> posts;
    QSqlQuery query;
    query.prepare("SELECT m.message_id, m.sender_id, 0, "
                  "u.username, 'Public', m.message_text, m.is_public, m.created_at "
                  "FROM messages m "
                  "JOIN users u ON m.sender_id = u.user_id "
                  "WHERE m.is_public = 1 "
                  "ORDER BY m.created_at DESC");

    if (query.exec()) {
        while (query.next()) {
            Message msg;
            msg.messageId = query.value(0).toInt();
            msg.senderId = query.value(1).toInt();
            msg.receiverId = query.value(2).toInt();
            msg.senderName = query.value(3).toString();
            msg.receiverName = query.value(4).toString();
            msg.messageText = query.value(5).toString();
            msg.isPublic = query.value(6).toBool();
            msg.timestamp = query.value(7).toDateTime();
            posts.append(msg);
        }
    }
    return posts;
}

QVector<Message> Database::getFeedForUser(int userId) {
    QVector<Message> feed;
    QSqlQuery query;
    query.prepare("SELECT m.message_id, m.sender_id, 0, "
                  "u.username, 'Public', m.message_text, m.is_public, m.created_at "
                  "FROM messages m "
                  "JOIN users u ON m.sender_id = u.user_id "
                  "WHERE m.is_public = 1 AND "
                  "(m.sender_id IN (SELECT friend_id FROM friends WHERE user_id = ?) "
                  "OR m.sender_id = ?) "
                  "ORDER BY m.created_at DESC");
    query.addBindValue(userId);
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            Message msg;
            msg.messageId = query.value(0).toInt();
            msg.senderId = query.value(1).toInt();
            msg.receiverId = query.value(2).toInt();
            msg.senderName = query.value(3).toString();
            msg.receiverName = query.value(4).toString();
            msg.messageText = query.value(5).toString();
            msg.isPublic = query.value(6).toBool();
            msg.timestamp = query.value(7).toDateTime();
            feed.append(msg);
        }
    }
    return feed;
}
