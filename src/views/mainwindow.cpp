#include "views/mainwindow.h"

#include <QAbstractItemView>
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QComboBox>
#include <QDateTime>
#include <QFrame>
#include <QGuiApplication>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QRegularExpression>
#include <QSharedPointer>
#include <QStandardPaths>
#include <QSysInfo>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QScreen>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QThread>
#include <QTabWidget>
#include <QTableView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "db/databasemanager.h"
#include "models/core/networkinterface.h"
#include "models/core/packetsummary.h"
#include "models/session/capturesession.h"
#include "services/packetcaptureservice.h"
#include "viewmodels/packetsummarytablemodel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_packetTable(new QTableView(this)),
      m_packetTableModel(new PacketSummaryTableModel(this)),
      m_captureService(new PacketCaptureService(this)),
      m_databaseManager(new DatabaseManager()),
      m_interfaceCombo(nullptr),
      m_sessionCombo(nullptr),
      m_filterInput(nullptr),
      m_captureButton(nullptr),
      m_stopCaptureButton(nullptr),
      m_loadSessionButton(nullptr),
      m_saveSelectedButton(nullptr),
      m_dbRefreshButton(nullptr),
      m_dbDeleteSessionButton(nullptr),
    m_wifiRefreshButton(nullptr),
    m_speedTestButton(nullptr),
      m_consoleLog(nullptr),
      m_throughputValue(nullptr),
      m_throughputGraph(nullptr),
    m_wifiSsidValue(nullptr),
    m_wifiSignalValue(nullptr),
    m_wifiRadioValue(nullptr),
    m_speedTestValue(nullptr),
      m_rootTabs(nullptr),
      m_dbSessionsTable(nullptr),
      m_dbPacketsTable(nullptr),
    m_wifiLog(nullptr),
      m_recentPacketCount(0),
      m_activeSessionId(-1),
      m_dbReady(false)
{
    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(12, 10, 12, 12);
    layout->setSpacing(10);
    m_rootTabs = new QTabWidget(centralWidget);
    layout->addWidget(m_rootTabs);

    auto *capturePage = new QWidget(m_rootTabs);
    auto *captureLayout = new QVBoxLayout(capturePage);
    captureLayout->setContentsMargins(0, 0, 0, 0);
    captureLayout->setSpacing(10);

    auto *topBar = new QFrame(capturePage);
    auto *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(10);

    auto *ifaceLabel = new QLabel("IFACE:", topBar);
    m_interfaceCombo = new QComboBox(topBar);
    m_interfaceCombo->setObjectName("chip");
    auto *sessionLabel = new QLabel("Session:", topBar);
    m_sessionCombo = new QComboBox(topBar);
    m_sessionCombo->setObjectName("chip");
    m_loadSessionButton = new QPushButton("Load Session", topBar);
    m_saveSelectedButton = new QPushButton("Save Selected", topBar);
    m_filterInput = new QLineEdit(topBar);
    m_filterInput->setPlaceholderText("tcp or port 443");
    m_filterInput->setText("tcp");
    m_captureButton = new QPushButton("Capture", topBar);
    m_stopCaptureButton = new QPushButton("Stop Capturing", topBar);

    topBarLayout->addWidget(ifaceLabel);
    topBarLayout->addWidget(m_interfaceCombo);
    topBarLayout->addWidget(sessionLabel);
    topBarLayout->addWidget(m_sessionCombo);
    topBarLayout->addWidget(m_loadSessionButton);
    topBarLayout->addWidget(m_saveSelectedButton);
    topBarLayout->addSpacing(8);
    topBarLayout->addWidget(m_filterInput, 1);
    topBarLayout->addWidget(m_captureButton);
    topBarLayout->addWidget(m_stopCaptureButton);

    m_packetTable->setModel(m_packetTableModel);
    m_packetTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_packetTable->setAlternatingRowColors(true);
    m_packetTable->horizontalHeader()->setStretchLastSection(true);
    m_packetTable->verticalHeader()->setVisible(false);
    m_packetTable->setShowGrid(false);
    m_packetTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_packetTable->setSortingEnabled(false);

    auto *detailTabs = new QTabWidget(capturePage);
    auto *detailsTree = new QTreeWidget(detailTabs);
    detailsTree->setHeaderHidden(true);
    auto *ethernetRoot = new QTreeWidgetItem(detailsTree, {"Ethernet II, Src: Apple_f2:31:1a, Dst: Netgear_02:21:4b"});
    new QTreeWidgetItem(ethernetRoot, {"Destination: Netgear_02:21:4b"});
    new QTreeWidgetItem(ethernetRoot, {"Source: Apple_f2:31:1a"});
    auto *ipv4Root = new QTreeWidgetItem(detailsTree, {"Internet Protocol Version 4, Src: 192.168.1.15, Dst: 104.22.3.45"});
    new QTreeWidgetItem(ipv4Root, {"Header Length: 20 bytes"});
    new QTreeWidgetItem(ipv4Root, {"Total Length: 490"});
    new QTreeWidgetItem(ipv4Root, {"Protocol: TCP (6)"});
    auto *tcpRoot = new QTreeWidgetItem(detailsTree, {"Transmission Control Protocol, Src Port: 52311, Dst Port: 443"});
    new QTreeWidgetItem(tcpRoot, {"Flags: 0x018 (PSH, ACK)"});
    new QTreeWidgetItem(tcpRoot, {"Window: 2048"});
    detailsTree->expandAll();

    auto *rawBytes = new QPlainTextEdit(detailTabs);
    rawBytes->setReadOnly(true);
    rawBytes->setPlainText(
        "0000  00 1e 2a 02 21 4b a4 83 e7 f2 31 1a 08 00 45 00\n"
        "0010  01 ea 4f 12 40 00 40 06 ae 12 c0 a8 01 0f 68 16\n"
        "0020  03 2d cc 57 01 bb 32 b8 77 f2 0d 8a 4f 31 50 18\n"
        "0030  08 00 6f af 00 00 17 03 03 00 2a 1c 8b 7d 65 94");

    auto *streamView = new QPlainTextEdit(detailTabs);
    streamView->setReadOnly(true);
    streamView->setPlainText(
        "[12:04:31] TLS Application Data\n"
        "[12:04:31] Client -> Server payload length 458\n"
        "[12:04:31] Server -> Client ACK\n"
        "[12:04:32] Stream complete");

    detailTabs->addTab(detailsTree, "Details");
    detailTabs->addTab(rawBytes, "Raw Bytes");
    detailTabs->addTab(streamView, "Streams");

    auto *mainSplitter = new QSplitter(Qt::Horizontal, capturePage);
    mainSplitter->addWidget(m_packetTable);
    mainSplitter->addWidget(detailTabs);
    mainSplitter->setStretchFactor(0, 3);
    mainSplitter->setStretchFactor(1, 2);

    auto *bottomRow = new QSplitter(Qt::Horizontal, capturePage);

    auto *throughputCard = new QFrame(bottomRow);
    auto *throughputLayout = new QVBoxLayout(throughputCard);
    throughputLayout->setContentsMargins(10, 8, 10, 10);
    throughputLayout->setSpacing(6);
    auto *throughputLabel = new QLabel("Throughput (Mbps)", throughputCard);
    m_throughputValue = new QLabel("0.0 Mbps", throughputCard);
    m_throughputValue->setObjectName("metric");
    m_throughputGraph = new QProgressBar(throughputCard);
    m_throughputGraph->setRange(0, 100);
    m_throughputGraph->setValue(0);
    m_throughputGraph->setTextVisible(false);
    throughputLayout->addWidget(throughputLabel);
    throughputLayout->addWidget(m_throughputValue);
    throughputLayout->addWidget(m_throughputGraph);

    auto *consoleCard = new QFrame(bottomRow);
    auto *consoleLayout = new QVBoxLayout(consoleCard);
    consoleLayout->setContentsMargins(10, 8, 10, 10);
    consoleLayout->setSpacing(6);
    auto *consoleLabel = new QLabel("Console Events", consoleCard);
    m_consoleLog = new QPlainTextEdit(consoleCard);
    m_consoleLog->setReadOnly(true);
    consoleLayout->addWidget(consoleLabel);
    consoleLayout->addWidget(m_consoleLog, 1);

    bottomRow->addWidget(throughputCard);
    bottomRow->addWidget(consoleCard);
    bottomRow->setStretchFactor(0, 1);
    bottomRow->setStretchFactor(1, 3);

    captureLayout->addWidget(topBar);
    captureLayout->addWidget(mainSplitter, 1);
    captureLayout->addWidget(bottomRow);
    m_rootTabs->addTab(capturePage, "Capture");

    auto *networkPage = new QWidget(m_rootTabs);
    auto *networkLayout = new QVBoxLayout(networkPage);
    networkLayout->setContentsMargins(0, 0, 0, 0);
    networkLayout->setSpacing(10);

    auto *networkToolbar = new QFrame(networkPage);
    auto *networkToolbarLayout = new QHBoxLayout(networkToolbar);
    networkToolbarLayout->setContentsMargins(0, 0, 0, 0);
    networkToolbarLayout->setSpacing(10);
    m_wifiRefreshButton = new QPushButton("Check Wi-Fi", networkToolbar);
    m_speedTestButton = new QPushButton("Run Speed Test", networkToolbar);
    networkToolbarLayout->addWidget(m_wifiRefreshButton);
    networkToolbarLayout->addWidget(m_speedTestButton);
    networkToolbarLayout->addStretch(1);

    auto *networkCards = new QSplitter(Qt::Horizontal, networkPage);

    auto *wifiCard = new QFrame(networkCards);
    auto *wifiLayout = new QVBoxLayout(wifiCard);
    wifiLayout->setContentsMargins(10, 8, 10, 10);
    wifiLayout->setSpacing(8);
    wifiLayout->addWidget(new QLabel("Wi-Fi signal", wifiCard));

    auto *wifiForm = new QFormLayout();
    wifiForm->setLabelAlignment(Qt::AlignLeft);
    m_wifiSsidValue = new QLabel("Unknown", wifiCard);
    m_wifiSignalValue = new QLabel("--", wifiCard);
    m_wifiRadioValue = new QLabel("Unknown", wifiCard);
    wifiForm->addRow("SSID:", m_wifiSsidValue);
    wifiForm->addRow("Signal:", m_wifiSignalValue);
    wifiForm->addRow("Radio:", m_wifiRadioValue);
    wifiLayout->addLayout(wifiForm);
    wifiLayout->addStretch(1);

    auto *speedCard = new QFrame(networkCards);
    auto *speedLayout = new QVBoxLayout(speedCard);
    speedLayout->setContentsMargins(10, 8, 10, 10);
    speedLayout->setSpacing(8);
    speedLayout->addWidget(new QLabel("Speed test", speedCard));
    m_speedTestValue = new QLabel("Idle", speedCard);
    m_speedTestValue->setObjectName("metric");
    speedLayout->addWidget(m_speedTestValue);
    m_wifiLog = new QPlainTextEdit(speedCard);
    m_wifiLog->setReadOnly(true);
    m_wifiLog->setPlaceholderText("Wi-Fi and speed test output will appear here.");
    speedLayout->addWidget(m_wifiLog, 1);

    networkCards->addWidget(wifiCard);
    networkCards->addWidget(speedCard);
    networkCards->setStretchFactor(0, 1);
    networkCards->setStretchFactor(1, 2);

    networkLayout->addWidget(networkToolbar);
    networkLayout->addWidget(networkCards, 1);
    m_rootTabs->addTab(networkPage, "Wi-Fi");

    auto *databasePage = new QWidget(m_rootTabs);
    auto *databaseLayout = new QVBoxLayout(databasePage);
    databaseLayout->setContentsMargins(0, 0, 0, 0);
    databaseLayout->setSpacing(10);

    auto *dbToolbar = new QFrame(databasePage);
    auto *dbToolbarLayout = new QHBoxLayout(dbToolbar);
    dbToolbarLayout->setContentsMargins(0, 0, 0, 0);
    dbToolbarLayout->setSpacing(10);
    m_dbRefreshButton = new QPushButton("Refresh DB", dbToolbar);
    m_dbDeleteSessionButton = new QPushButton("Delete Session", dbToolbar);
    dbToolbarLayout->addWidget(m_dbRefreshButton);
    dbToolbarLayout->addWidget(m_dbDeleteSessionButton);
    dbToolbarLayout->addStretch(1);

    auto *dbSplitter = new QSplitter(Qt::Vertical, databasePage);
    m_dbSessionsTable = new QTableWidget(dbSplitter);
    m_dbSessionsTable->setColumnCount(7);
    m_dbSessionsTable->setHorizontalHeaderLabels({"ID", "Interface", "Filter", "Started", "Ended", "Packets", "Active"});
    m_dbSessionsTable->horizontalHeader()->setStretchLastSection(true);
    m_dbSessionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dbSessionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dbSessionsTable->verticalHeader()->setVisible(false);
    m_dbSessionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_dbPacketsTable = new QTableWidget(dbSplitter);
    m_dbPacketsTable->setColumnCount(7);
    m_dbPacketsTable->setHorizontalHeaderLabels({"#", "Time", "Source", "Destination", "Protocol", "Length", "Info"});
    m_dbPacketsTable->horizontalHeader()->setStretchLastSection(true);
    m_dbPacketsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dbPacketsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dbPacketsTable->verticalHeader()->setVisible(false);
    m_dbPacketsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    dbSplitter->setStretchFactor(0, 1);
    dbSplitter->setStretchFactor(1, 2);
    databaseLayout->addWidget(dbToolbar);
    databaseLayout->addWidget(dbSplitter, 1);
    m_rootTabs->addTab(databasePage, "Database");

    setCentralWidget(centralWidget);

    setWindowTitle("NetScope");
    constexpr int preferredWidth = 1360;
    constexpr int preferredHeight = 760;
    const QRect availableScreen = QGuiApplication::primaryScreen()->availableGeometry();
    const int width = qMin(preferredWidth, availableScreen.width() - 32);
    const int height = qMin(preferredHeight, availableScreen.height() - 32);
    resize(width, height);

    setStyleSheet(
        "QMainWindow { background: #07131f; color: #d2e1ef; }"
        "QLabel { color: #95aec0; }"
        "QFrame { background: #0b1826; border: 1px solid #1c3447; border-radius: 8px; }"
        "QComboBox#chip { background: #123753; border: 1px solid #2f6888; border-radius: 10px; padding: 2px 10px; color: #b8def7; min-width: 190px; }"
        "QComboBox#chip QAbstractItemView { background: #0b1826; color: #d2e1ef; border: 1px solid #2f6888; selection-background-color: #194769; }"
        "QLabel#metric { color: #52d4ff; font-weight: 700; font-size: 16px; }"
        "QLineEdit { background: #0f2232; border: 1px solid #29465a; border-radius: 6px; padding: 6px 10px; color: #e2edf6; }"
        "QPushButton { background: #123048; border: 1px solid #2b5470; border-radius: 6px; padding: 6px 12px; color: #d7e7f3; }"
        "QPushButton:hover { background: #184362; }"
        "QTableView { background: #081521; alternate-background-color: #0d1d2d; border: 1px solid #1f3448; color: #d7e7f3; selection-background-color: #194769; }"
        "QHeaderView::section { background: #0f2738; color: #8fb3cc; border: none; border-right: 1px solid #193548; padding: 6px; }"
        "QTabWidget::pane { border: 1px solid #1f3448; border-radius: 8px; background: #081521; }"
        "QTabBar::tab { background: #0f2738; color: #8fb3cc; padding: 6px 12px; margin-right: 4px; border-top-left-radius: 6px; border-top-right-radius: 6px; }"
        "QTabBar::tab:selected { background: #194769; color: #d9ecfa; }"
        "QTreeWidget, QPlainTextEdit { background: #081521; border: none; color: #d7e7f3; }"
        "QProgressBar { background: #102536; border: 1px solid #28465a; border-radius: 6px; height: 10px; }"
        "QProgressBar::chunk { background: #1dc0ff; border-radius: 5px; }"
        "QSplitter::handle { background: #102638; }");

    connect(m_captureButton, &QPushButton::clicked, this, [this]() {
        const QString interfaceName = selectedInterfaceName();
        if (interfaceName.isEmpty()) {
            appendConsoleEvent("ERROR: Please select an interface before starting capture.");
            return;
        }

        m_packetTableModel->clear();
        m_recentPacketCount = 0;
        m_throughputGraph->setValue(0);
        m_throughputValue->setText("0.0 Mbps");

        const QString filterText = m_filterInput->text().trimmed();
        appendConsoleEvent(QString("INFO: Capturing on %1 ...").arg(interfaceName));
        if (!filterText.isEmpty()) {
            appendConsoleEvent(QString("INFO: Applied BPF Filter: %1").arg(filterText));
        }
        m_captureService->startCapture(interfaceName, filterText);
    });

    connect(m_stopCaptureButton, &QPushButton::clicked, this, [this]() {
        m_captureService->stopCapture();
        appendConsoleEvent("INFO: Capture stopped.");
    });

    connect(m_loadSessionButton, &QPushButton::clicked, this, [this]() {
        loadSessionIntoLiveTable(selectedSessionId());
    });

    connect(m_saveSelectedButton, &QPushButton::clicked, this, [this]() {
        saveSelectedPacketsToDatabase();
    });

    connect(m_dbRefreshButton, &QPushButton::clicked, this, [this]() {
        refreshDatabasePage();
        appendConsoleEvent("INFO: Database page refreshed.");
    });

    connect(m_dbDeleteSessionButton, &QPushButton::clicked, this, [this]() {
        if (!m_dbReady) {
            appendConsoleEvent("ERROR: Database is not connected.");
            return;
        }
        const int row = m_dbSessionsTable->currentRow();
        if (row < 0) {
            appendConsoleEvent("WARN: Select a DB session to delete.");
            return;
        }
        const qint64 sessionId = m_dbSessionsTable->item(row, 0)->text().toLongLong();
        if (!m_databaseManager->deleteSession(sessionId)) {
            appendConsoleEvent("ERROR: Failed to delete session: " + m_databaseManager->lastError());
            return;
        }
        appendConsoleEvent(QString("INFO: Session %1 deleted from DB.").arg(sessionId));
        refreshSessions();
        refreshDatabasePage();
    });

    connect(m_wifiRefreshButton, &QPushButton::clicked, this, [this]() {
        refreshWifiStatus();
    });

    connect(m_speedTestButton, &QPushButton::clicked, this, [this]() {
        runSpeedTest();
    });

    auto loadSelectedDbSessionPackets = [this]() {
        const int row = m_dbSessionsTable->currentRow();
        if (row < 0) {
            m_dbPacketsTable->setRowCount(0);
            return;
        }
        QTableWidgetItem *idItem = m_dbSessionsTable->item(row, 0);
        if (idItem == nullptr) {
            m_dbPacketsTable->setRowCount(0);
            return;
        }
        const qint64 sessionId = idItem->text().toLongLong();
        loadDatabaseSessionPackets(sessionId);
    };

    connect(m_dbSessionsTable, &QTableWidget::itemSelectionChanged, this, loadSelectedDbSessionPackets);
    connect(m_dbSessionsTable, &QTableWidget::cellClicked, this, [loadSelectedDbSessionPackets](int, int) {
        loadSelectedDbSessionPackets();
    });

    connect(m_captureService, &PacketCaptureService::packetCaptured, this, [this](const PacketSummary &packet) {
        m_packetTableModel->appendPacket(packet);
        m_packetTable->scrollToBottom();
        ++m_recentPacketCount;

        const double throughputMbps = qMin(99.9, m_recentPacketCount * 0.35);
        m_throughputValue->setText(QString::number(throughputMbps, 'f', 1) + " Mbps");
        m_throughputGraph->setValue(static_cast<int>(throughputMbps));
    });

    connect(m_captureService, &PacketCaptureService::captureError, this, [this](const QString &message) {
        appendConsoleEvent("ERROR: " + message);
    });

    connect(m_captureService, &PacketCaptureService::captureInfo, this, [this](const QString &message) {
        appendConsoleEvent("INFO: " + message);
    });

    connect(m_captureService, &PacketCaptureService::captureStateChanged, this, [this](bool active) {
        applyCaptureState(active);
        if (!active) {
            appendConsoleEvent("INFO: Capture session ended.");
        }
    });

    m_dbReady = initializeDatabase();
    refreshInterfaces();
    if (m_dbReady) {
        refreshSessions();
        refreshDatabasePage();
    }
    refreshWifiStatus();
    appendConsoleEvent("INFO: Ready. Select an interface and press Capture.");
    applyCaptureState(false);
}

MainWindow::~MainWindow()
{
    if (m_captureService != nullptr && m_captureService->isCapturing()) {
        m_captureService->stopCapture();
    }
    delete m_databaseManager;
}

void MainWindow::refreshInterfaces()
{
    m_interfaces = m_captureService->availableInterfaces();
    m_interfaceCombo->clear();

    for (const NetworkInterface &iface : m_interfaces) {
        QString label = iface.displayName;
        if (label.isEmpty()) {
            label = iface.name;
        }
        if (!iface.addresses.isEmpty()) {
            label += QString(" (%1)").arg(iface.addresses.first());
        }
        m_interfaceCombo->addItem(label, iface.name);
    }

    if (m_interfaceCombo->count() == 0) {
        m_interfaceCombo->addItem("No interfaces found", "");
        m_captureButton->setEnabled(false);
        appendConsoleEvent("WARN: No network interfaces available.");
    } else {
        m_captureButton->setEnabled(true);
    }
}

void MainWindow::refreshSessions()
{
    if (!m_dbReady) {
        m_sessionCombo->clear();
        m_sessionCombo->addItem("DB unavailable", -1);
        return;
    }

    const QVector<CaptureSession> sessions = m_databaseManager->fetchSessions();
    m_sessionCombo->clear();
    for (const CaptureSession &session : sessions) {
        const QString startedAtText = session.startedAt.toLocalTime().toString("yyyy-MM-dd hh:mm:ss");
        const QString title = QString("#%1 %2 | %3 | packets: %4")
                                  .arg(session.id)
                                  .arg(session.interfaceName)
                                  .arg(startedAtText)
                                  .arg(session.packetCount);
        m_sessionCombo->addItem(title, session.id);
    }

    if (m_sessionCombo->count() == 0) {
        m_sessionCombo->addItem("No saved captures", -1);
    }
}

void MainWindow::refreshDatabasePage()
{
    if (!m_dbReady) {
        m_dbSessionsTable->setRowCount(0);
        m_dbPacketsTable->setRowCount(0);
        return;
    }

    const QVector<CaptureSession> sessions = m_databaseManager->fetchSessions(500);
    m_dbSessionsTable->setRowCount(sessions.size());
    for (int row = 0; row < sessions.size(); ++row) {
        const CaptureSession &session = sessions.at(row);
        m_dbSessionsTable->setItem(row, 0, new QTableWidgetItem(QString::number(session.id)));
        m_dbSessionsTable->setItem(row, 1, new QTableWidgetItem(session.interfaceName));
        m_dbSessionsTable->setItem(row, 2, new QTableWidgetItem(session.filterExpression));
        m_dbSessionsTable->setItem(row, 3, new QTableWidgetItem(session.startedAt.toLocalTime().toString("yyyy-MM-dd hh:mm:ss")));
        m_dbSessionsTable->setItem(row, 4, new QTableWidgetItem(session.endedAt.toLocalTime().toString("yyyy-MM-dd hh:mm:ss")));
        m_dbSessionsTable->setItem(row, 5, new QTableWidgetItem(QString::number(session.packetCount)));
        m_dbSessionsTable->setItem(row, 6, new QTableWidgetItem(session.active ? "Yes" : "No"));
    }
    m_dbPacketsTable->setRowCount(0);
    if (!sessions.isEmpty()) {
        m_dbSessionsTable->setCurrentCell(0, 0);
        loadDatabaseSessionPackets(sessions.first().id);
    }
}

void MainWindow::saveSelectedPacketsToDatabase()
{
    if (!m_dbReady) {
        appendConsoleEvent("ERROR: Database is not connected.");
        return;
    }

    const QModelIndexList selectedRows = m_packetTable->selectionModel()->selectedRows();
    if (selectedRows.isEmpty()) {
        appendConsoleEvent("WARN: Select one or more packets to save.");
        return;
    }

    const QString interfaceName = selectedInterfaceName().isEmpty() ? "manual" : selectedInterfaceName();
    const QString filterText = m_filterInput->text().trimmed();
    const qint64 sessionId = m_databaseManager->createSession(interfaceName, filterText);
    if (sessionId <= 0) {
        appendConsoleEvent("ERROR: Failed to create DB session: " + m_databaseManager->lastError());
        return;
    }

    qint64 savedCount = 0;
    for (const QModelIndex &index : selectedRows) {
        const PacketSummary packet = m_packetTableModel->packetAt(index.row());
        if (m_databaseManager->insertPacket(sessionId, packet)) {
            ++savedCount;
        }
    }
    m_databaseManager->closeSession(sessionId, savedCount);
    appendConsoleEvent(QString("INFO: Saved %1 selected packets to DB session %2.").arg(savedCount).arg(sessionId));
    refreshSessions();
    refreshDatabasePage();
}

void MainWindow::loadSessionIntoLiveTable(qint64 sessionId)
{
    if (!m_dbReady) {
        appendConsoleEvent("ERROR: Database is not connected.");
        return;
    }
    if (sessionId <= 0) {
        appendConsoleEvent("WARN: Please select a saved session to load.");
        return;
    }

    const QVector<PacketSummary> packets = m_databaseManager->fetchPackets(sessionId);
    m_packetTableModel->setPackets(packets);
    m_recentPacketCount = packets.size();
    const double throughputMbps = qMin(99.9, m_recentPacketCount * 0.35);
    m_throughputValue->setText(QString::number(throughputMbps, 'f', 1) + " Mbps");
    m_throughputGraph->setValue(static_cast<int>(throughputMbps));
    appendConsoleEvent(QString("INFO: Loaded %1 packets from session %2.").arg(packets.size()).arg(sessionId));
    m_rootTabs->setCurrentIndex(0);
}

void MainWindow::loadDatabaseSessionPackets(qint64 sessionId)
{
    if (!m_dbReady || sessionId <= 0) {
        m_dbPacketsTable->setRowCount(0);
        return;
    }
    const QVector<PacketSummary> packets = m_databaseManager->fetchPackets(sessionId, 10000);
    m_dbPacketsTable->setRowCount(packets.size());
    for (int row = 0; row < packets.size(); ++row) {
        const PacketSummary &packet = packets.at(row);
        m_dbPacketsTable->setItem(row, 0, new QTableWidgetItem(QString::number(packet.packetNumber)));
        m_dbPacketsTable->setItem(row, 1, new QTableWidgetItem(packet.timeText));
        m_dbPacketsTable->setItem(row, 2, new QTableWidgetItem(packet.source));
        m_dbPacketsTable->setItem(row, 3, new QTableWidgetItem(packet.destination));
        m_dbPacketsTable->setItem(row, 4, new QTableWidgetItem(packet.protocol));
        m_dbPacketsTable->setItem(row, 5, new QTableWidgetItem(QString::number(packet.length)));
        m_dbPacketsTable->setItem(row, 6, new QTableWidgetItem(packet.info));
    }
}

bool MainWindow::initializeDatabase()
{
    constexpr int maxAttempts = 15;
    for (int attempt = 1; attempt <= maxAttempts; ++attempt) {
        const bool connected =
            m_databaseManager->connect("mysql", 3306, "packet_capture_db", "wireshark_user", "wireshark_password");
        if (connected) {
            appendConsoleEvent("INFO: MySQL connected. Captures will be saved.");
            return true;
        }

        if (attempt == 1 || attempt % 5 == 0) {
            appendConsoleEvent(
                QString("WARN: Waiting for MySQL (%1/%2): %3")
                    .arg(attempt)
                    .arg(maxAttempts)
                    .arg(m_databaseManager->lastError()));
        }
        QThread::msleep(1000);
    }
    appendConsoleEvent("WARN: MySQL unavailable: " + m_databaseManager->lastError());
    return false;
}

void MainWindow::appendConsoleEvent(const QString &message)
{
    const QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_consoleLog->appendPlainText(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::applyCaptureState(bool capturing)
{
    m_captureButton->setEnabled(!capturing);
    m_stopCaptureButton->setEnabled(capturing);
    m_saveSelectedButton->setEnabled(!capturing && m_dbReady);
    m_interfaceCombo->setEnabled(!capturing);
    m_filterInput->setEnabled(!capturing);
    m_sessionCombo->setEnabled(!capturing);
    m_loadSessionButton->setEnabled(!capturing && m_dbReady);
}

QString MainWindow::selectedInterfaceName() const
{
    const int selectedIndex = m_interfaceCombo->currentIndex();
    if (selectedIndex < 0) {
        return {};
    }
    return m_interfaceCombo->currentData().toString();
}

qint64 MainWindow::selectedSessionId() const
{
    const int selectedIndex = m_sessionCombo->currentIndex();
    if (selectedIndex < 0) {
        return -1;
    }
    return m_sessionCombo->currentData().toLongLong();
}

void MainWindow::refreshWifiStatus()
{
    if (m_wifiLog != nullptr) {
        m_wifiLog->clear();
        m_wifiLog->appendPlainText("Checking Wi-Fi status...");
    }

    const bool isWindows = QSysInfo::productType().contains("windows", Qt::CaseInsensitive);
    const QString command = isWindows ? QStandardPaths::findExecutable("netsh")
                                      : QStandardPaths::findExecutable("iwconfig");
    const QString toolName = isWindows ? "netsh" : "iwconfig";
    if (command.isEmpty()) {
        m_wifiSsidValue->setText(toolName + " not found");
        m_wifiSignalValue->setText("Unavailable");
        m_wifiRadioValue->setText("Unavailable");
        appendConsoleEvent(QString("WARN: %1 was not found in PATH.").arg(toolName));
        return;
    }

    auto *process = new QProcess(this);
    const auto output = QSharedPointer<QString>::create();
    const auto errors = QSharedPointer<QString>::create();

    connect(process, &QProcess::readyReadStandardOutput, this, [process, output]() {
        *output += QString::fromUtf8(process->readAllStandardOutput());
    });

    connect(process, &QProcess::readyReadStandardError, this, [process, errors]() {
        *errors += QString::fromUtf8(process->readAllStandardError());
    });

        connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, process, output, errors, isWindows](int exitCode, QProcess::ExitStatus) {
                const QString combined = *output + '\n' + *errors;
                process->deleteLater();

                if (exitCode != 0 && combined.trimmed().isEmpty()) {
                    m_wifiSsidValue->setText("Unavailable");
                    m_wifiSignalValue->setText("Unavailable");
                    m_wifiRadioValue->setText("Unavailable");
                    appendConsoleEvent("ERROR: Failed to read Wi-Fi status.");
                    return;
                }

                QString ssid = "Unknown";
                QString signal = "Not available";
                QString radio = "Not available";
                QString state = "Unknown";

                if (isWindows) {
                    const QRegularExpression ssidRegex(R"(^\s*SSID(?:\s+\d+)?\s*:\s*(.+)$)",
                                                       QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption);
                    const QRegularExpression signalRegex(R"(^\s*Signal\s*:\s*(\d+)%$)",
                                                         QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption);
                    const QRegularExpression radioRegex(R"(^\s*Radio type\s*:\s*(.+)$)",
                                                        QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption);
                    const QRegularExpression stateRegex(R"(^\s*State\s*:\s*(.+)$)",
                                                        QRegularExpression::MultilineOption | QRegularExpression::CaseInsensitiveOption);

                    const QRegularExpressionMatch ssidMatch = ssidRegex.match(combined);
                    if (ssidMatch.hasMatch()) {
                        ssid = ssidMatch.captured(1).trimmed();
                    }

                    const QRegularExpressionMatch signalMatch = signalRegex.match(combined);
                    if (signalMatch.hasMatch()) {
                        signal = signalMatch.captured(1).trimmed() + "%";
                    }

                    const QRegularExpressionMatch radioMatch = radioRegex.match(combined);
                    if (radioMatch.hasMatch()) {
                        radio = radioMatch.captured(1).trimmed();
                    }

                    const QRegularExpressionMatch stateMatch = stateRegex.match(combined);
                    if (stateMatch.hasMatch()) {
                        state = stateMatch.captured(1).trimmed();
                    }
                } else {
                    const QRegularExpression ssidRegex("ESSID:\"([^\"]*)\"", QRegularExpression::CaseInsensitiveOption);
                    const QRegularExpression levelRegex("(?:Signal level|Link Quality)=([^\\r\\n]+)", QRegularExpression::CaseInsensitiveOption);
                    const QRegularExpression modeRegex("Mode:([^\\s]+)", QRegularExpression::CaseInsensitiveOption);

                    const QRegularExpressionMatch ssidMatch = ssidRegex.match(combined);
                    if (ssidMatch.hasMatch()) {
                        ssid = ssidMatch.captured(1).trimmed();
                    }

                    const QRegularExpressionMatch levelMatch = levelRegex.match(combined);
                    if (levelMatch.hasMatch()) {
                        signal = levelMatch.captured(1).trimmed();
                    }

                    const QRegularExpressionMatch modeMatch = modeRegex.match(combined);
                    if (modeMatch.hasMatch()) {
                        radio = modeMatch.captured(1).trimmed();
                    }
                    state = combined.contains("ESSID:\"off/any\"", Qt::CaseInsensitive) ? "Disconnected" : "Connected";
                }

                m_wifiSsidValue->setText(ssid);
                m_wifiSignalValue->setText(signal);
                m_wifiRadioValue->setText(QString("%1 (%2)").arg(radio, state));
                if (m_wifiLog != nullptr) {
                    m_wifiLog->appendPlainText(QString("Wi-Fi status: SSID=%1, Signal=%2, Radio=%3, State=%4")
                                                   .arg(ssid, signal, radio, state));
                }
                appendConsoleEvent(QString("INFO: Wi-Fi checked (%1, %2). ").arg(ssid, signal));
            });

    if (isWindows) {
        process->start(command, {"wlan", "show", "interfaces"});
    } else {
        process->start(command, {});
    }
}

void MainWindow::runSpeedTest()
{
    if (m_wifiLog != nullptr) {
        m_wifiLog->appendPlainText("Running speed test...");
    }
    m_speedTestValue->setText("Running...");

    const bool isWindows = QSysInfo::productType().contains("windows", Qt::CaseInsensitive);
    const QString speedtest = isWindows ? QStandardPaths::findExecutable("speedtest")
                                        : (!QStandardPaths::findExecutable("speedtest-cli").isEmpty()
                                               ? QStandardPaths::findExecutable("speedtest-cli")
                                               : QStandardPaths::findExecutable("speedtest"));
    if (speedtest.isEmpty()) {
        m_speedTestValue->setText("Speed test tool not found");
        if (m_wifiLog != nullptr) {
            m_wifiLog->appendPlainText("Speed test unavailable: no speedtest CLI was found in PATH.");
        }
        appendConsoleEvent("WARN: no speedtest CLI was found in PATH.");
        return;
    }

    auto *process = new QProcess(this);
    const auto output = QSharedPointer<QString>::create();
    const auto errors = QSharedPointer<QString>::create();

    connect(process, &QProcess::readyReadStandardOutput, this, [process, output]() {
        *output += QString::fromUtf8(process->readAllStandardOutput());
    });

    connect(process, &QProcess::readyReadStandardError, this, [process, errors]() {
        *errors += QString::fromUtf8(process->readAllStandardError());
    });

    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, process, output, errors](int exitCode, QProcess::ExitStatus) {
                const QString combined = *output + '\n' + *errors;
                process->deleteLater();

                const QJsonDocument jsonDocument = QJsonDocument::fromJson(combined.toUtf8());
                if (jsonDocument.isObject()) {
                    const QJsonObject root = jsonDocument.object();
                    double downloadMbps = 0.0;
                    double uploadMbps = 0.0;
                    double latencyMs = 0.0;

                    if (root.value("download").isObject()) {
                        const QJsonObject download = root.value("download").toObject();
                        const QJsonObject upload = root.value("upload").toObject();
                        const QJsonObject ping = root.value("ping").toObject();
                        downloadMbps = download.value("bandwidth").toDouble() * 8.0 / 1000000.0;
                        uploadMbps = upload.value("bandwidth").toDouble() * 8.0 / 1000000.0;
                        latencyMs = ping.value("latency").toDouble();
                    } else {
                        downloadMbps = root.value("download").toDouble() / 1000000.0;
                        uploadMbps = root.value("upload").toDouble() / 1000000.0;
                        latencyMs = root.value("ping").toDouble();
                    }

                    m_speedTestValue->setText(QString("Down %1 Mbps | Up %2 Mbps | Ping %3 ms")
                                                  .arg(QString::number(downloadMbps, 'f', 2),
                                                       QString::number(uploadMbps, 'f', 2),
                                                       QString::number(latencyMs, 'f', 0)));
                    if (m_wifiLog != nullptr) {
                        m_wifiLog->appendPlainText(QString("Speed test: download %1 Mbps, upload %2 Mbps, ping %3 ms")
                                                       .arg(QString::number(downloadMbps, 'f', 2),
                                                            QString::number(uploadMbps, 'f', 2),
                                                            QString::number(latencyMs, 'f', 0)));
                    }
                    appendConsoleEvent("INFO: Speed test completed.");
                    return;
                }

                const QRegularExpression downloadRegex(R"(Download:\s*([0-9.]+)\s*Mbps)", QRegularExpression::CaseInsensitiveOption);
                const QRegularExpression uploadRegex(R"(Upload:\s*([0-9.]+)\s*Mbps)", QRegularExpression::CaseInsensitiveOption);
                const QRegularExpression pingRegex(R"((?:Latency|Ping):\s*([0-9.]+)\s*ms)", QRegularExpression::CaseInsensitiveOption);

                const QString download = downloadRegex.match(combined).captured(1);
                const QString upload = uploadRegex.match(combined).captured(1);
                const QString ping = pingRegex.match(combined).captured(1);

                if (!download.isEmpty() || !upload.isEmpty() || !ping.isEmpty()) {
                    m_speedTestValue->setText(QString("Down %1 Mbps | Up %2 Mbps | Ping %3 ms")
                                                  .arg(download.isEmpty() ? "?" : download,
                                                       upload.isEmpty() ? "?" : upload,
                                                       ping.isEmpty() ? "?" : ping));
                    if (m_wifiLog != nullptr) {
                        m_wifiLog->appendPlainText(QString("Speed test: download %1 Mbps, upload %2 Mbps, ping %3 ms")
                                                       .arg(download.isEmpty() ? "?" : download,
                                                            upload.isEmpty() ? "?" : upload,
                                                            ping.isEmpty() ? "?" : ping));
                    }
                    appendConsoleEvent("INFO: Speed test completed.");
                    return;
                }

                m_speedTestValue->setText("Speed test failed");
                if (m_wifiLog != nullptr) {
                    m_wifiLog->appendPlainText(combined.trimmed().isEmpty() ? "Speed test failed." : combined.trimmed());
                }
                if (exitCode != 0) {
                    appendConsoleEvent("ERROR: Speed test command failed.");
                } else {
                    appendConsoleEvent("WARN: Speed test output could not be parsed.");
                }
            });

    const QStringList args = isWindows
                                 ? QStringList{"-f", "json", "--accept-license", "--accept-gdpr"}
                                 : QStringList{"--json"};
    process->start(speedtest, args);
}
