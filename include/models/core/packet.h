#pragma once

#include <QByteArray>
#include <QDateTime>
#include <QString>

struct Packet
{
    qint64 id = 0;
    QString interfaceName;
    qint64 packetNumber = 0;
    QDateTime capturedAt;
    int length = 0;
    QByteArray rawData;
};
