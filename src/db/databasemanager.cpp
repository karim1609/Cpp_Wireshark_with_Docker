#include "db/databasemanager.h"

#include <QSqlError>
#include <QSqlQuery>

DatabaseManager::DatabaseManager()
{
    m_db = QSqlDatabase::addDatabase("QMYSQL");
}

DatabaseManager::~DatabaseManager()
{
    disconnect();
}

bool DatabaseManager::connect(const QString &host, int port, const QString &dbName, const QString &user, const QString &password)
{
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);
    return m_db.open();
}

void DatabaseManager::disconnect()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}
