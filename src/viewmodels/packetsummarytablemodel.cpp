#include "viewmodels/packetsummarytablemodel.h"

PacketSummaryTableModel::PacketSummaryTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int PacketSummaryTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_packets.size();
}

int PacketSummaryTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 7;
}

QVariant PacketSummaryTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole) {
        return {};
    }

    const PacketSummary &packet = m_packets.at(index.row());
    switch (index.column()) {
    case 0: return packet.packetNumber;
    case 1: return packet.timeText;
    case 2: return packet.source;
    case 3: return packet.destination;
    case 4: return packet.protocol;
    case 5: return packet.length;
    case 6: return packet.info;
    default: return {};
    }
}

QVariant PacketSummaryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return "#";
        case 1: return "Time";
        case 2: return "Source";
        case 3: return "Destination";
        case 4: return "Protocol";
        case 5: return "Length";
        case 6: return "Info";
        default: return {};
        }
    }

    return section + 1;
}

void PacketSummaryTableModel::setPackets(QVector<PacketSummary> packets)
{
    beginResetModel();
    m_packets = std::move(packets);
    endResetModel();
}

void PacketSummaryTableModel::appendPacket(const PacketSummary &packet)
{
    const int insertIndex = m_packets.size();
    beginInsertRows(QModelIndex(), insertIndex, insertIndex);
    m_packets.push_back(packet);
    endInsertRows();
}

void PacketSummaryTableModel::clear()
{
    beginResetModel();
    m_packets.clear();
    endResetModel();
}

PacketSummary PacketSummaryTableModel::packetAt(int row) const
{
    if (row < 0 || row >= m_packets.size()) {
        return {};
    }
    return m_packets.at(row);
}

QVector<PacketSummary> PacketSummaryTableModel::packets() const
{
    return m_packets;
}
