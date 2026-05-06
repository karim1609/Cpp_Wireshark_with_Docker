#pragma once

struct TcpSegment
{
    int sourcePort = 0;
    int destinationPort = 0;
    quint32 sequenceNumber = 0;
    quint32 acknowledgmentNumber = 0;
    int dataOffset = 0;
    int flags = 0;
    int windowSize = 0;
};
