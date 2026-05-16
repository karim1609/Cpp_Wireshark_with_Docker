#pragma once

#include <QMainWindow>
#include <QString>
#include <QVector>

class DatabaseManager;
struct NetworkInterface;
struct PacketSummary;
class QTableView;
class QComboBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QPlainTextEdit;
class QProgressBar;
class QTableWidget;
class QTabWidget;
class QProcess;
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
    void refreshSessions();
    void refreshDatabasePage();
    void refreshWifiStatus();
    void runSpeedTest();
    bool initializeDatabase();
    void saveSelectedPacketsToDatabase();
    void loadSessionIntoLiveTable(qint64 sessionId);
    void loadDatabaseSessionPackets(qint64 sessionId);
    void appendConsoleEvent(const QString &message);
    void applyCaptureState(bool capturing);
    QString selectedInterfaceName() const;
    qint64 selectedSessionId() const;

    QTableView *m_packetTable;
    PacketSummaryTableModel *m_packetTableModel;
    PacketCaptureService *m_captureService;
    DatabaseManager *m_databaseManager;
    QComboBox *m_interfaceCombo;
    QComboBox *m_sessionCombo;
    QLineEdit *m_filterInput;
    QPushButton *m_captureButton;
    QPushButton *m_stopCaptureButton;
    QPushButton *m_loadSessionButton;
    QPushButton *m_saveSelectedButton;
    QPushButton *m_dbRefreshButton;
    QPushButton *m_dbDeleteSessionButton;
    QPushButton *m_wifiRefreshButton;
    QPushButton *m_speedTestButton;
    QPlainTextEdit *m_consoleLog;
    QLabel *m_throughputValue;
    QProgressBar *m_throughputGraph;
    QLabel *m_wifiSsidValue;
    QLabel *m_wifiSignalValue;
    QLabel *m_wifiRadioValue;
    QLabel *m_speedTestValue;
    QTabWidget *m_rootTabs;
    QTableWidget *m_dbSessionsTable;
    QTableWidget *m_dbPacketsTable;
    QPlainTextEdit *m_wifiLog;
    QVector<NetworkInterface> m_interfaces;
    int m_recentPacketCount;
    qint64 m_activeSessionId;
    bool m_dbReady;
};
