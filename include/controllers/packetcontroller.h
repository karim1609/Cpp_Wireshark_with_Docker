#pragma once

#include <QObject>

#include "db/databasemanager.h"

class PacketController : public QObject
{
    Q_OBJECT

public:
    explicit PacketController(QObject *parent = nullptr);
    bool initializeDatabase();

private:
    DatabaseManager m_databaseManager;
};
