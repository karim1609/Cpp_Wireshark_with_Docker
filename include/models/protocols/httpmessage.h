#pragma once

#include <QMap>
#include <QString>

struct HttpMessage
{
    QString method;
    int statusCode = 0;
    QString host;
    QString path;
    QMap<QString, QString> headers;
};
