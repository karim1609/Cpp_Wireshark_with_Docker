#pragma once

#include <QDateTime>
#include <QString>

struct CaptureSession
{
    qint64 id = 0;
    QString interfaceName;
    QString filterExpression;
    QDateTime startedAt;
    QDateTime endedAt;
    qint64 packetCount = 0;
    bool active = false;
};
