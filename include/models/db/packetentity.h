#pragma once

#include <QByteArray>
#include <QString>

struct PacketEntity
{
    qint64 id = 0;
    qint64 packetNumber = 0;
    QString timestampText;
    int packetLength = 0;
    QByteArray rawData;
    QString sourceAddress;
    QString destinationAddress;
    QString protocolLabel;
};
