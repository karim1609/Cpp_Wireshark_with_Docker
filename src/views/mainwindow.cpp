#include "views/mainwindow.h"

#include <QLabel>
#include <Qt>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_statusLabel(new QLabel("Hello World from Qt Widget"))
{
    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);

    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("font-size: 28px; font-weight: 700;");

    layout->addWidget(m_statusLabel);
    setCentralWidget(centralWidget);

    setWindowTitle("WireScope - Hello");
    resize(900, 600);
}
