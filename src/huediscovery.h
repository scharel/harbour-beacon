#ifndef HUEDISCOVERY_H
#define HUEDISCOVERY_H

#include "mdns/mdns.h"
#include "huebridge.h"

#include <QObject>
#include <QAbstractListModel>
#include <QQmlEngine>
#include <QHash>
#include <QHostAddress>
#include <QTimer>

const QString HUE_MDNS("_hue._tcp.local.");

class HueDiscovery : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    explicit HueDiscovery(QObject *parent = nullptr);
    ~HueDiscovery();

    // QML singleton
    static void instantiate(QObject *parent = nullptr);
    static HueDiscovery & getInstance();
    static QObject * provider(QQmlEngine *, QJSEngine *);

    QHash<int, QByteArray> roleNames() const { return HueBridge::roleNames(); }

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    Q_INVOKABLE HueBridge* bridge(int index = 0) const;

    Q_INVOKABLE int discover(const QString& query = HUE_MDNS, int pollIntercal = 100);
    Q_INVOKABLE void clearBridges(const QList<int>& keep = QList<int>());

    bool busy() const { return m_socket >= 0; }
    int count() const { return m_pendingBridges.count(); }

protected slots:
    Q_INVOKABLE void abortDiscovery();
    void recvData();

    void bridgePropertyChanged(int role);

protected:
    void addBridge(const QString& bridgeid, const QString& address, ushort port = 443, const QString& modelid = "");

signals:
    //void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ());
    void busyChanged(bool busy);
    void countChanged(int count);

private:
    // QML singleton
    static HueDiscovery* mp_instance;

    sockaddr_in m_sockaddr;
    int m_socket;
    QByteArray m_buffer;
    QTimer m_recvTimer;

    static int recvCallback(int sock, const struct sockaddr *from, size_t addrlen, mdns_entry_type_t entry,
                            uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void *data,
                            size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                            size_t record_length, void *user_data);

    QList<HueBridge*> m_pendingBridges;
};

#endif // HUEDISCOVERY_H
