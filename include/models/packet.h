#pragma once

#include <QDateTime>
#include <QString>

struct Packet
{
    qint64 id = 0;
    QString sourceIp;
    QString destinationIp;
    QString protocol;
    int length = 0;
    QDateTime capturedAt;
};
