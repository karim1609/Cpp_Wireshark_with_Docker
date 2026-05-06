#include "views/mainwindow.h"

#include <QComboBox>
#include <QDateTime>
#include <QFrame>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
#include <QTableView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "models/core/networkinterface.h"
#include "models/core/packetsummary.h"
#include "services/packetcaptureservice.h"
#include "viewmodels/packetsummarytablemodel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_packetTable(new QTableView(this)),
      m_packetTableModel(new PacketSummaryTableModel(this)),
      m_captureService(new PacketCaptureService(this)),
      m_interfaceCombo(nullptr),
      m_filterInput(nullptr),
      m_captureButton(nullptr),
      m_pauseButton(nullptr),
      m_consoleLog(nullptr),
      m_throughputValue(nullptr),
      m_throughputGraph(nullptr),
      m_recentPacketCount(0)
{
    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(12, 10, 12, 12);
    layout->setSpacing(10);

    auto *topBar = new QFrame(centralWidget);
    auto *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(10);

    auto *ifaceLabel = new QLabel("IFACE:", topBar);
    m_interfaceCombo = new QComboBox(topBar);
    m_interfaceCombo->setObjectName("chip");
    m_filterInput = new QLineEdit(topBar);
    m_filterInput->setPlaceholderText("tcp or port 443");
    m_filterInput->setText("tcp");
    m_captureButton = new QPushButton("Capture", topBar);
    m_pauseButton = new QPushButton("Stop", topBar);

    topBarLayout->addWidget(ifaceLabel);
    topBarLayout->addWidget(m_interfaceCombo);
    topBarLayout->addSpacing(8);
    topBarLayout->addWidget(m_filterInput, 1);
    topBarLayout->addWidget(m_captureButton);
    topBarLayout->addWidget(m_pauseButton);

    m_packetTable->setModel(m_packetTableModel);
    m_packetTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_packetTable->setAlternatingRowColors(true);
    m_packetTable->horizontalHeader()->setStretchLastSection(true);
    m_packetTable->verticalHeader()->setVisible(false);
    m_packetTable->setShowGrid(false);
    m_packetTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_packetTable->setSortingEnabled(false);

    auto *detailTabs = new QTabWidget(centralWidget);
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

    auto *mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    mainSplitter->addWidget(m_packetTable);
    mainSplitter->addWidget(detailTabs);
    mainSplitter->setStretchFactor(0, 3);
    mainSplitter->setStretchFactor(1, 2);

    auto *bottomRow = new QSplitter(Qt::Horizontal, centralWidget);

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

    layout->addWidget(topBar);
    layout->addWidget(mainSplitter, 1);
    layout->addWidget(bottomRow);
    setCentralWidget(centralWidget);

    setWindowTitle("WireScope");
    resize(1360, 760);

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

    connect(m_pauseButton, &QPushButton::clicked, this, [this]() {
        m_captureService->stopCapture();
        appendConsoleEvent("INFO: Capture stopped.");
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

    refreshInterfaces();
    appendConsoleEvent("INFO: Ready. Select an interface and press Capture.");
    applyCaptureState(false);
}

MainWindow::~MainWindow()
{
    if (m_captureService != nullptr && m_captureService->isCapturing()) {
        m_captureService->stopCapture();
    }
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

void MainWindow::appendConsoleEvent(const QString &message)
{
    const QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_consoleLog->appendPlainText(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::applyCaptureState(bool capturing)
{
    m_captureButton->setEnabled(!capturing);
    m_pauseButton->setEnabled(capturing);
    m_interfaceCombo->setEnabled(!capturing);
    m_filterInput->setEnabled(!capturing);
}

QString MainWindow::selectedInterfaceName() const
{
    const int selectedIndex = m_interfaceCombo->currentIndex();
    if (selectedIndex < 0) {
        return {};
    }
    return m_interfaceCombo->currentData().toString();
}
