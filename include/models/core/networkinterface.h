#pragma once

#include <QString>
#include <QStringList>

struct NetworkInterface
{
    QString name;
    QString displayName;
    QString description;
    QStringList addresses;
    bool isUp = false;
    bool isLoopback = false;
};
