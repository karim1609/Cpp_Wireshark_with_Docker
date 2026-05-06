#pragma once

#include <QSqlDatabase>

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool connect(const QString &host, int port, const QString &dbName, const QString &user, const QString &password);
    void disconnect();

private:
    QSqlDatabase m_db;
};
