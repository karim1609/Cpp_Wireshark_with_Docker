#include "services/packetcaptureservice.h"

#include <QNetworkInterface>
#include <QStandardPaths>
#include <QTimer>

PacketCaptureService::PacketCaptureService(QObject *parent)
    : QObject(parent)
{
    connect(&m_captureProcess, &QProcess::started, this, [this]() {
        emit captureStateChanged(true);
    });

    connect(&m_captureProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        m_stdoutBuffer.append(QString::fromUtf8(m_captureProcess.readAllStandardOutput()));
        int newlineIndex = m_stdoutBuffer.indexOf('\n');
        while (newlineIndex >= 0) {
            QString line = m_stdoutBuffer.left(newlineIndex).trimmed();
            m_stdoutBuffer.remove(0, newlineIndex + 1);
            processOutputLine(line);
            newlineIndex = m_stdoutBuffer.indexOf('\n');
        }
    });

    connect(&m_captureProcess, &QProcess::readyReadStandardError, this, [this]() {
        const QString rawText = QString::fromUtf8(m_captureProcess.readAllStandardError());
        const QStringList lines = rawText.split('\n', Qt::SkipEmptyParts);
        for (const QString &rawLine : lines) {
            const QString line = rawLine.trimmed();
            if (line.isEmpty()) {
                continue;
            }

            // tshark frequently prints non-fatal runtime notes to stderr.
            const bool isCaptureStartedMessage = line.contains("Capture started", Qt::CaseInsensitive);
            const bool isCaptureFileMessage = line.contains("File:", Qt::CaseInsensitive);
            const bool isRootWarning = line.contains("Running as user \"root\"", Qt::CaseInsensitive);
            const bool isMessagePrefix = line.contains("[Main MESSAGE]", Qt::CaseInsensitive);

            if (isCaptureStartedMessage || isCaptureFileMessage || isRootWarning || isMessagePrefix) {
                emit captureInfo(line);
                continue;
            }

            emit captureError(line);
        }
    });

    connect(&m_captureProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError processError) {
        QString message;
        switch (processError) {
        case QProcess::FailedToStart:
            message = "Unable to start capture process. Install tshark and ensure it is in PATH.";
            break;
        case QProcess::Crashed:
            message = "Capture process crashed.";
            break;
        default:
            message = "Capture process error occurred.";
            break;
        }
        emit captureError(message);
        emit captureStateChanged(false);
    });

    connect(&m_captureProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [this]() {
        emit captureStateChanged(false);
    });
}

QVector<NetworkInterface> PacketCaptureService::availableInterfaces() const
{
    QVector<NetworkInterface> result;

    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    result.reserve(interfaces.size());

    for (const QNetworkInterface &iface : interfaces) {
        NetworkInterface model;
        model.name = iface.name();
        model.displayName = iface.humanReadableName().isEmpty() ? iface.name() : iface.humanReadableName();
        model.description = iface.humanReadableName();
        model.isUp = iface.flags().testFlag(QNetworkInterface::IsUp);
        model.isLoopback = iface.flags().testFlag(QNetworkInterface::IsLoopBack);

        const QList<QNetworkAddressEntry> entries = iface.addressEntries();
        for (const QNetworkAddressEntry &entry : entries) {
            if (!entry.ip().isNull()) {
                model.addresses << entry.ip().toString();
            }
        }

        result.push_back(model);
    }

    return result;
}

bool PacketCaptureService::startCapture(const QString &interfaceName, const QString &filterExpression)
{
    if (interfaceName.trimmed().isEmpty()) {
        emit captureError("Please select a network interface.");
        return false;
    }

    if (isCapturing()) {
        stopCapture();
    }

    m_captureExecutable = resolveCaptureExecutable();
    if (m_captureExecutable.isEmpty()) {
        emit captureError("No capture binary found. Install tshark (or tcpdump) and ensure it is in PATH.");
        emit captureStateChanged(false);
        return false;
    }

    m_stdoutBuffer.clear();
    const QStringList args = buildCaptureArguments(interfaceName, filterExpression);
    m_captureProcess.start(m_captureExecutable, args);
    return true;
}

void PacketCaptureService::stopCapture()
{
    if (!isCapturing()) {
        return;
    }

    m_captureProcess.terminate();
    QTimer::singleShot(1500, this, [this]() {
        if (m_captureProcess.state() != QProcess::NotRunning) {
            m_captureProcess.kill();
        }
    });
}

bool PacketCaptureService::isCapturing() const
{
    return m_captureProcess.state() != QProcess::NotRunning;
}

void PacketCaptureService::processOutputLine(const QString &line)
{
    if (line.isEmpty()) {
        return;
    }

    const QStringList fields = line.split('\t');
    if (fields.size() < 9) {
        return;
    }

    bool packetNumberOk = false;
    bool packetLengthOk = false;

    const qint64 packetNumber = fields.at(0).toLongLong(&packetNumberOk);
    const double relativeTime = fields.at(1).toDouble();
    const QString source = fields.at(2).isEmpty() ? fields.at(3) : fields.at(2);
    const QString destination = fields.at(4).isEmpty() ? fields.at(5) : fields.at(4);
    const QString protocol = fields.at(6);
    const int length = fields.at(7).toInt(&packetLengthOk);
    const QString info = fields.at(8);

    if (!packetNumberOk || !packetLengthOk) {
        return;
    }

    PacketSummary summary;
    summary.packetNumber = packetNumber;
    summary.timeText = QString::number(relativeTime, 'f', 6);
    summary.source = source;
    summary.destination = destination;
    summary.protocol = protocol;
    summary.length = length;
    summary.info = info;
    emit packetCaptured(summary);
}

QString PacketCaptureService::resolveCaptureExecutable() const
{
    const QStringList candidates = {"tshark", "tcpdump", "dumpcap"};
    for (const QString &name : candidates) {
        const QString resolvedPath = QStandardPaths::findExecutable(name);
        if (!resolvedPath.isEmpty()) {
            return resolvedPath;
        }
    }
    return {};
}

QStringList PacketCaptureService::buildCaptureArguments(const QString &interfaceName, const QString &filterExpression) const
{
    QStringList args = {
        "-l",
        "-n",
        "-i", interfaceName,
        "-T", "fields",
        "-E", "separator=\t",
        "-e", "frame.number",
        "-e", "frame.time_relative",
        "-e", "ip.src",
        "-e", "ipv6.src",
        "-e", "ip.dst",
        "-e", "ipv6.dst",
        "-e", "_ws.col.Protocol",
        "-e", "frame.len",
        "-e", "_ws.col.Info"
    };

    if (!filterExpression.trimmed().isEmpty()) {
        args << "-f" << filterExpression.trimmed();
    }

    if (m_captureExecutable.contains("tshark")) {
        args << "-Q";
    }

    return args;
}
