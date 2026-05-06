#pragma once

#include <QString>

struct PacketSummary
{
    qint64 packetNumber = 0;
    QString timeText;
    QString source;
    QString destination;
    QString protocol;
    int length = 0;
    QString info;
};
