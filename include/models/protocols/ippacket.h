#pragma once

#include <QString>

struct IpPacket
{
    int version = 4;
    int headerLength = 0;
    int ttl = 0;
    int protocol = 0;
    int totalLength = 0;
    QString sourceAddress;
    QString destinationAddress;
};
