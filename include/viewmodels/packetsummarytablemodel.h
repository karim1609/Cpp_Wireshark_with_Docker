#pragma once

#include <QAbstractTableModel>
#include <QVector>

#include "models/core/packetsummary.h"

class PacketSummaryTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PacketSummaryTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setPackets(QVector<PacketSummary> packets);
    void appendPacket(const PacketSummary &packet);
    void clear();
    PacketSummary packetAt(int row) const;
    QVector<PacketSummary> packets() const;

private:
    QVector<PacketSummary> m_packets;
};
