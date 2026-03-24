#include "user.h"

User::User() : userId(0) {}

User::User(int id, const QString& username, const QString& fullName, const QDateTime& created)
    : userId(id), username(username), fullName(fullName), createdAt(created) {}
