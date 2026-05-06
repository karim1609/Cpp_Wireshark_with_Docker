#pragma once

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QVector>

#include "models/core/networkinterface.h"
#include "models/core/packetsummary.h"

class PacketCaptureService : public QObject
{
    Q_OBJECT

public:
    explicit PacketCaptureService(QObject *parent = nullptr);

    QVector<NetworkInterface> availableInterfaces() const;
    bool startCapture(const QString &interfaceName, const QString &filterExpression);
    void stopCapture();
    bool isCapturing() const;

signals:
    void packetCaptured(const PacketSummary &packet);
    void captureError(const QString &message);
    void captureInfo(const QString &message);
    void captureStateChanged(bool active);

private:
    void processOutputLine(const QString &line);
    QString resolveCaptureExecutable() const;
    QStringList buildCaptureArguments(const QString &interfaceName, const QString &filterExpression) const;

    QProcess m_captureProcess;
    QString m_stdoutBuffer;
    QString m_captureExecutable;
};
