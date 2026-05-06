#pragma once

#include <QMap>
#include <QString>

struct PacketDetail
{
    qint64 packetNumber = 0;
    QMap<QString, QString> ethernet;
    QMap<QString, QString> ip;
    QMap<QString, QString> transport;
    QMap<QString, QString> application;
};
