#pragma once

#include <QString>

struct EthernetFrame
{
    QString sourceMac;
    QString destinationMac;
    quint16 etherType = 0;
};
