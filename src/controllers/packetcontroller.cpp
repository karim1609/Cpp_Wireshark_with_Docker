#include "controllers/packetcontroller.h"

PacketController::PacketController(QObject *parent)
    : QObject(parent)
{
}

bool PacketController::initializeDatabase()
{
    return m_databaseManager.connect("mysql", 3306, "packet_capture_db", "wireshark_user", "wireshark_password");
}
