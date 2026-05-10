#include "db/databasemanager.h"

#include <QDateTime>
#include <QStringList>

#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <mysql_driver.h>

DatabaseManager::DatabaseManager()
    : m_driver(nullptr)
{
}

DatabaseManager::~DatabaseManager()
{
    disconnect();
}

bool DatabaseManager::connect(const QString &host, int port, const QString &dbName, const QString &user, const QString &password)
{
    try {
        const std::string endpoint = QString("tcp://%1:%2").arg(host).arg(port).toStdString();
        m_driver = sql::mysql::get_mysql_driver_instance();
        m_connection.reset(m_driver->connect(endpoint, user.toStdString(), password.toStdString()));
        m_connection->setSchema(dbName.toStdString());
    } catch (const sql::SQLException &exception) {
        m_lastError = QString::fromStdString(exception.what());
        m_connection.reset();
        return false;
    }
    return ensureSchema();
}

void DatabaseManager::disconnect()
{
    if (m_connection) {
        try {
            m_connection->close();
        } catch (...) {
        }
        m_connection.reset();
    }
}

bool DatabaseManager::isConnected() const
{
    return m_connection && !m_connection->isClosed();
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::ensureSchema()
{
    if (!isConnected()) {
        m_lastError = "Database is not connected.";
        return false;
    }

    try {
        std::unique_ptr<sql::Statement> statement(m_connection->createStatement());
        statement->execute(
            "CREATE TABLE IF NOT EXISTS capture_sessions ("
            "id BIGINT AUTO_INCREMENT PRIMARY KEY,"
            "interface_name VARCHAR(128) NOT NULL,"
            "filter_expression VARCHAR(255) NOT NULL DEFAULT '',"
            "started_at DATETIME NOT NULL,"
            "ended_at DATETIME NULL,"
            "packet_count BIGINT NOT NULL DEFAULT 0,"
            "active TINYINT(1) NOT NULL DEFAULT 1"
            ")");

        statement->execute(
            "CREATE TABLE IF NOT EXISTS packets ("
            "id BIGINT AUTO_INCREMENT PRIMARY KEY,"
            "session_id BIGINT NOT NULL,"
            "packet_number BIGINT NOT NULL,"
            "time_text VARCHAR(64) NOT NULL,"
            "source_ip VARCHAR(64) NOT NULL,"
            "destination_ip VARCHAR(64) NOT NULL,"
            "protocol VARCHAR(30) NOT NULL,"
            "length INT NOT NULL,"
            "info_text TEXT,"
            "captured_at DATETIME NOT NULL,"
            "INDEX idx_packets_session_id (session_id),"
            "CONSTRAINT fk_packets_session_id FOREIGN KEY (session_id) REFERENCES capture_sessions(id) ON DELETE CASCADE"
            ")");
    } catch (const sql::SQLException &exception) {
        m_lastError = QString::fromStdString(exception.what());
        return false;
    }

    return true;
}

QVector<CaptureSession> DatabaseManager::fetchSessions(int limit) const
{
    QVector<CaptureSession> sessions;
    if (!isConnected()) {
        m_lastError = "Database is not connected.";
        return sessions;
    }

    try {
        std::unique_ptr<sql::PreparedStatement> statement(
            m_connection->prepareStatement(
                "SELECT id, interface_name, filter_expression, started_at, ended_at, packet_count, active "
                "FROM capture_sessions ORDER BY started_at DESC LIMIT ?"));
        statement->setInt(1, limit);
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery());

        while (result->next()) {
            CaptureSession session;
            session.id = result->getInt64("id");
            session.interfaceName = QString::fromStdString(result->getString("interface_name"));
            session.filterExpression = QString::fromStdString(result->getString("filter_expression"));
            const QString started = QString::fromStdString(result->getString("started_at"));
            const QString ended = QString::fromStdString(result->getString("ended_at"));
            session.startedAt = QDateTime::fromString(started, "yyyy-MM-dd HH:mm:ss");
            session.startedAt.setTimeSpec(Qt::UTC);
            session.endedAt = QDateTime::fromString(ended, "yyyy-MM-dd HH:mm:ss");
            session.endedAt.setTimeSpec(Qt::UTC);
            session.packetCount = result->getInt64("packet_count");
            session.active = result->getBoolean("active");
            sessions.push_back(session);
        }
    } catch (const sql::SQLException &exception) {
        m_lastError = QString::fromStdString(exception.what());
        return sessions;
    }

    return sessions;
}

qint64 DatabaseManager::createSession(const QString &interfaceName, const QString &filterExpression)
{
    if (!isConnected()) {
        m_lastError = "Database is not connected.";
        return -1;
    }

    try {
        const QString nowUtc = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss");
        std::unique_ptr<sql::PreparedStatement> statement(
            m_connection->prepareStatement(
                "INSERT INTO capture_sessions(interface_name, filter_expression, started_at, active) "
                "VALUES(?, ?, ?, 1)"));
        statement->setString(1, interfaceName.toStdString());
        statement->setString(2, filterExpression.toStdString());
        statement->setString(3, nowUtc.toStdString());
        statement->executeUpdate();

        std::unique_ptr<sql::Statement> idStatement(m_connection->createStatement());
        std::unique_ptr<sql::ResultSet> result(idStatement->executeQuery("SELECT LAST_INSERT_ID() AS id"));
        if (result->next()) {
            return result->getInt64("id");
        }
    } catch (const sql::SQLException &exception) {
        m_lastError = QString::fromStdString(exception.what());
        return -1;
    }
    m_lastError = "Could not read created session id.";
    return -1;
}

bool DatabaseManager::closeSession(qint64 sessionId, qint64 packetCount)
{
    if (!isConnected()) {
        m_lastError = "Database is not connected.";
        return false;
    }

    try {
        const QString nowUtc = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss");
        std::unique_ptr<sql::PreparedStatement> statement(
            m_connection->prepareStatement(
                "UPDATE capture_sessions "
                "SET ended_at = ?, packet_count = ?, active = 0 "
                "WHERE id = ?"));
        statement->setString(1, nowUtc.toStdString());
        statement->setInt64(2, packetCount);
        statement->setInt64(3, sessionId);
        statement->executeUpdate();
    } catch (const sql::SQLException &exception) {
        m_lastError = QString::fromStdString(exception.what());
        return false;
    }

    return true;
}

bool DatabaseManager::deleteSession(qint64 sessionId)
{
    if (!isConnected()) {
        m_lastError = "Database is not connected.";
        return false;
    }

    try {
        std::unique_ptr<sql::PreparedStatement> statement(
            m_connection->prepareStatement("DELETE FROM capture_sessions WHERE id = ?"));
        statement->setInt64(1, sessionId);
        statement->executeUpdate();
    } catch (const sql::SQLException &exception) {
        m_lastError = QString::fromStdString(exception.what());
        return false;
    }

    return true;
}

bool DatabaseManager::insertPacket(qint64 sessionId, const PacketSummary &packet)
{
    if (!isConnected()) {
        m_lastError = "Database is not connected.";
        return false;
    }

    try {
        const QString nowUtc = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ss");
        std::unique_ptr<sql::PreparedStatement> statement(
            m_connection->prepareStatement(
                "INSERT INTO packets(session_id, packet_number, time_text, source_ip, destination_ip, protocol, length, info_text, captured_at) "
                "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)"));
        statement->setInt64(1, sessionId);
        statement->setInt64(2, packet.packetNumber);
        statement->setString(3, packet.timeText.toStdString());
        statement->setString(4, packet.source.toStdString());
        statement->setString(5, packet.destination.toStdString());
        statement->setString(6, packet.protocol.toStdString());
        statement->setInt(7, packet.length);
        statement->setString(8, packet.info.toStdString());
        statement->setString(9, nowUtc.toStdString());
        statement->executeUpdate();
    } catch (const sql::SQLException &exception) {
        m_lastError = QString::fromStdString(exception.what());
        return false;
    }

    return true;
}

QVector<PacketSummary> DatabaseManager::fetchPackets(qint64 sessionId, int limit) const
{
    QVector<PacketSummary> packets;
    if (!isConnected()) {
        m_lastError = "Database is not connected.";
        return packets;
    }

    try {
        std::unique_ptr<sql::PreparedStatement> statement(
            m_connection->prepareStatement(
                "SELECT packet_number, time_text, source_ip, destination_ip, protocol, length, info_text "
                "FROM packets WHERE session_id = ? ORDER BY packet_number ASC LIMIT ?"));
        statement->setInt64(1, sessionId);
        statement->setInt(2, limit);
        std::unique_ptr<sql::ResultSet> result(statement->executeQuery());

        while (result->next()) {
            PacketSummary packet;
            packet.packetNumber = result->getInt64("packet_number");
            packet.timeText = QString::fromStdString(result->getString("time_text"));
            packet.source = QString::fromStdString(result->getString("source_ip"));
            packet.destination = QString::fromStdString(result->getString("destination_ip"));
            packet.protocol = QString::fromStdString(result->getString("protocol"));
            packet.length = result->getInt("length");
            packet.info = QString::fromStdString(result->getString("info_text"));
            packets.push_back(packet);
        }
    } catch (const sql::SQLException &exception) {
        m_lastError = QString::fromStdString(exception.what());
        return packets;
    }

    return packets;
}
