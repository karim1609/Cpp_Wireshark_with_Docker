#pragma once

#include <memory>
#include <QString>
#include <QVector>

#include "models/core/packetsummary.h"
#include "models/session/capturesession.h"

namespace sql {
class Connection;
class Driver;
}

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool connect(const QString &host, int port, const QString &dbName, const QString &user, const QString &password);
    void disconnect();
    bool isConnected() const;
    QString lastError() const;

    bool ensureSchema();
    QVector<CaptureSession> fetchSessions(int limit = 100) const;
    qint64 createSession(const QString &interfaceName, const QString &filterExpression);
    bool closeSession(qint64 sessionId, qint64 packetCount);
    bool deleteSession(qint64 sessionId);
    bool insertPacket(qint64 sessionId, const PacketSummary &packet);
    QVector<PacketSummary> fetchPackets(qint64 sessionId, int limit = 5000) const;

private:
    sql::Driver *m_driver;
    std::unique_ptr<sql::Connection> m_connection;
    mutable QString m_lastError;
};
