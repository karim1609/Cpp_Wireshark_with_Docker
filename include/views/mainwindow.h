#pragma once

#include <QMainWindow>
#include <QVector>

struct NetworkInterface;
struct PacketSummary;
class QTableView;
class QComboBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QPlainTextEdit;
class QProgressBar;
class PacketSummaryTableModel;
class PacketCaptureService;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void refreshInterfaces();
    void appendConsoleEvent(const QString &message);
    void applyCaptureState(bool capturing);
    QString selectedInterfaceName() const;

    QTableView *m_packetTable;
    PacketSummaryTableModel *m_packetTableModel;
    PacketCaptureService *m_captureService;
    QComboBox *m_interfaceCombo;
    QLineEdit *m_filterInput;
    QPushButton *m_captureButton;
    QPushButton *m_pauseButton;
    QPlainTextEdit *m_consoleLog;
    QLabel *m_throughputValue;
    QProgressBar *m_throughputGraph;
    QVector<NetworkInterface> m_interfaces;
    int m_recentPacketCount;
};
