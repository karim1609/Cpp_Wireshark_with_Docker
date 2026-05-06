#pragma once

#include <QString>

struct DnsMessage
{
    QString queryName;
    quint16 queryType = 0;
    quint16 responseCode = 0;
    bool isResponse = false;
};
