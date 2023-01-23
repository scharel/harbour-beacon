#include "huediscovery.h"

#include <netdb.h>
#include <QDebug>

HueDiscovery::HueDiscovery(QObject *parent) : QAbstractListModel(parent)
{
    m_socket = -1;
    m_sockaddr.sin_family = AF_INET;
    m_sockaddr.sin_addr.s_addr = INADDR_ANY;
    m_sockaddr.sin_port = htons((unsigned short)MDNS_PORT);
    m_buffer.resize(2048);
    m_recvTimer.setInterval(100);
    connect(&m_recvTimer, SIGNAL(timeout()), this, SLOT(recvData()));
}

HueDiscovery::~HueDiscovery() {
    abortDiscovery();
}

// QML singleton
HueDiscovery * HueDiscovery::mp_instance = nullptr;

void HueDiscovery::instantiate(QObject *parent) {
    if (mp_instance == nullptr) {
        mp_instance = new HueDiscovery(parent);
    }
}

HueDiscovery & HueDiscovery::getInstance() {
    return *mp_instance;
}

QObject * HueDiscovery::provider(QQmlEngine *, QJSEngine *) {
    return mp_instance;
}

Qt::ItemFlags HueDiscovery::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;    // | Qt::ItemIsEditable;
    }
    else {
        return Qt::NoItemFlags;
    }
}

int HueDiscovery::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_pendingBridges.size();
}

QVariant HueDiscovery::data(const QModelIndex &index, int role) const {
    //qDebug() << "Getting data on index" << index.row() << "and role" << role;
    QVariant data;
    if (index.isValid() && index.row() >= 0 && index.row() < m_pendingBridges.size()) {
        data = m_pendingBridges[index.row()]->property(role);
    }
    //qDebug() << data;
    return data;
}

HueBridge* HueDiscovery::bridge(int index) const {
    HueBridge* bridge = nullptr;
    if (index >= 0 && index < m_pendingBridges.size()) {
        bridge = m_pendingBridges[index];
    }
    return bridge;
}

/*
QMap<int, QVariant> HueDiscovery::itemData(const QModelIndex &index) const {
    qDebug() << "Getting itemData on index" << index.row();
    QMap<int, QVariant> map;
    int n = -1;
    QHashIterator<QHostAddress, HueBridge> i(m_bridges);
    while (i.hasNext() && n < index.row()) {
        i.next();
        n++;
    }
    map = i.value().data();
    return map;
}
*/
int HueDiscovery::discover(const QString& query, int pollInterval) {
    int queryId = -1;
    qDebug() << "Starting discovery for" << query;
    if (m_socket < 0) {
        m_socket = mdns_socket_open_ipv4(&m_sockaddr);
        emit busyChanged(true);
    }
    queryId = mdns_query_send(m_socket, MDNS_RECORDTYPE_ANY, query.toStdString().c_str(), query.length(), m_buffer.data(), m_buffer.size(), 0);
    m_recvTimer.start(pollInterval);
    /*for (int i = 0; i < m_pendingBridges.size(); ++i) {
        m_pendingBridges[i]->getConfig();
    }*/
    return queryId;
}

void HueDiscovery::clearBridges(const QList<int> &keep) {
    int removedBridges = 0;
    emit beginResetModel();
    abortDiscovery();
    for (int i = m_pendingBridges.size()-1; i >= 0; --i) {
        if (!keep.contains(i)) {
            delete m_pendingBridges[i];
            m_pendingBridges.removeAt(i);
            removedBridges++;
        }
    }
    emit endResetModel();
    if (removedBridges > 0) {
        emit countChanged(m_pendingBridges.count());
    }
}

void HueDiscovery::abortDiscovery() {
    m_recvTimer.stop();
    if (m_socket >= 0) {
        mdns_socket_close(m_socket);
        m_socket = -1;
        emit busyChanged(false);
    }
    qDebug() << "Aborted discovery";
}

void HueDiscovery::recvData() {
    //qDebug() << "Reveiving data...";
    void* userData = 0;
    if (m_socket >= 0) {
        mdns_query_recv(m_socket, m_buffer.data(), m_buffer.size(), recvCallback, userData, 0);
    }
}

void HueDiscovery::bridgePropertyChanged(int role) {
    //qDebug() << "Bridge changed" << role;
    int bridge = m_pendingBridges.indexOf((HueBridge*)QObject::sender());
    emit dataChanged(index(bridge), index(bridge), QVector<int>({ role }));
}

int HueDiscovery::recvCallback(int sock, const struct sockaddr *from, size_t addrlen, mdns_entry_type_t entry,
                               uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void *data,
                               size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                               size_t record_length, void *user_data) {
    Q_UNUSED(entry)
    Q_UNUSED(sock)
    Q_UNUSED(query_id)
    Q_UNUSED(rclass)
    Q_UNUSED(ttl)
    Q_UNUSED(name_length)
    Q_UNUSED(record_offset)
    Q_UNUSED(record_length)
    Q_UNUSED(user_data)
    QByteArray buf((char*)data, size);
    //qDebug() << buf;
    mdns_string_t entrystr = mdns_string_extract(data, size, &name_offset, buf.data(), buf.size());
    QByteArray service(entrystr.str, entrystr.length);
    //qDebug() << service;

    QRegExp rx(QString("^Philips Hue - [A-F0-9]{6}.%1$").arg(QString(HUE_MDNS)));
    if (rx.exactMatch(service)) {
        if (rtype == MDNS_RECORDTYPE_TXT) {
            //qDebug() << "TXT received";
            mdns_record_txt_t record[2];
            mdns_record_parse_txt(data, size, record_offset, record_length, record, 2);
            QHash<QString, QString> txt;
            for (int i = 0; i < 2; ++i) {
                txt[QByteArray(record[i].key.str, record[i].key.length)] = QByteArray(record[i].value.str, record[i].value.length);
            }
            qDebug() << txt["bridgeid"].toUpper();
            QByteArray hbuf(NI_MAXHOST, '\0');
            QByteArray sbuf(NI_MAXSERV, '\0');
            getnameinfo(from, (socklen_t)addrlen, hbuf.data(), NI_MAXHOST, sbuf.data(), NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);;
            QString address(hbuf);
            HueDiscovery::getInstance().addBridge(txt["bridgeid"].toUpper(), address, sbuf.toUInt(), txt["modelid"]);
        }
    }
    return 0;
}

void HueDiscovery::addBridge(const QString &bridgeid, const QString &address, ushort port, const QString &modelid) {
    if (!bridgeid.isEmpty() && !address.isEmpty()) {
        bool isNewBridge = true;
        for (int i = 0; i < m_pendingBridges.size() && isNewBridge; ++i) {
            isNewBridge = m_pendingBridges[i]->bridgeid() != bridgeid;
            if (!isNewBridge) {
                m_pendingBridges[i]->setAddress(QHostAddress(address));
                m_pendingBridges[i]->setPort(port);
            }
        }
        if (isNewBridge) {
            beginInsertRows(QModelIndex(), m_pendingBridges.size(), m_pendingBridges.size());
            HueBridge* bridge = new HueBridge(bridgeid, modelid, this);
            connect(bridge, SIGNAL(propertyChanged(int)), this, SLOT(bridgePropertyChanged(int)));
            bridge->setAddress(QHostAddress(address));
            bridge->setPort(port);
            //bridge->getConfig();
            m_pendingBridges.append(bridge);
            endInsertRows();
            emit countChanged(m_pendingBridges.count());
        }
    }
}
