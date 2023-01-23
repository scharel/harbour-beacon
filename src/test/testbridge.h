#ifndef TESTBRIDGE_H
#define TESTBRIDGE_H

#include "../huebridge.h"
#include <QTimer>

class TestReply : protected QNetworkReply
{
    Q_OBJECT

    TestReply(QObject *parent = Q_NULLPTR) : QNetworkReply(parent) {
        QTimer::singleShot(1000, this, SLOT(onTimerEnd()));
    }

protected slots:
    void onTimerEnd();

private:
};

class TestBridge : public HueBridge
{
    Q_OBJECT

    // Class state properties
    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)   // if network connectivity is accessible
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)      // when an API call is currently running
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)   // when all resources have been received

    // Properties resolved by uDNS
    Q_PROPERTY(QHostAddress address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(qint16 port READ port WRITE setPort NOTIFY portChanged)

    // Hue Config API properties
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString datastoreversion READ datastoreversion NOTIFY datastoreversionChanged)
    Q_PROPERTY(QString swversion READ swversion NOTIFY swversionChanged)
    Q_PROPERTY(QString apiversion READ apiversion NOTIFY apiversionChanged)
    Q_PROPERTY(QString mac READ mac NOTIFY macChanged)
    Q_PROPERTY(QString bridgeid READ bridgeid NOTIFY bridgeidChanged)
    Q_PROPERTY(bool factorynew READ factorynew NOTIFY factorynewChanged)
    Q_PROPERTY(QString replacesbridgeid READ replacesbridgeid NOTIFY replacesbridgeidChanged)
    Q_PROPERTY(QString modelid READ modelid NOTIFY modelidChanged)
    Q_PROPERTY(QString starterkitid READ starterkitid NOTIFY starterkitidChanged)

    // Hue Authentication properties
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString clientkey READ clientkey NOTIFY clientkeyChanged)

public:
    explicit TestBridge(const QString& bridgeid = "", const QString& modelid = "", QObject *parent = nullptr);
    virtual ~TestBridge();

    bool operator==(const TestBridge& other) const { return other.bridgeid() == property(BridgeidRole); }

    bool networkAccessible() const { return true; }

    Q_INVOKABLE bool createUser(const QString& instance_name = QHostInfo::localHostName());
    Q_INVOKABLE bool getConfig();
    Q_INVOKABLE bool getResource(ResourceType resource = ResourceAll, const QString& id = QString());
    Q_INVOKABLE bool putResource(ResourceType resource, const QJsonObject& json, const QString& id = QString());
    Q_INVOKABLE bool startEventStream();
    Q_INVOKABLE bool stopEventStream();

    Q_INVOKABLE ResourceSortFilterProxyModel* resourceModel(ResourceType type) { return m_resources[type]; }
    Q_INVOKABLE ResourceObject* resource(ResourceType type, const QString& id = "") { return ((ResourceModel*)m_resources[type]->sourceModel())->resource(id); }

signals:
    void networkAccessibleChanged(bool accessible);
    void busyChanged(bool busy);

    void addressChanged(const QHostAddress& address);
    void portChanged(quint16 port);
    void readyChanged(bool ready);

    void nameChanged(const QString& name);
    void datastoreversionChanged(const QString& datastoreversion);
    void swversionChanged(const QString& swversion);
    void apiversionChanged(const QString& apiversion);
    void macChanged(const QString& mac);
    void bridgeidChanged(const QString& bridgeid);
    void factorynewChanged(bool factorynew);
    void replacesbridgeidChanged(const QString& replacesbridgeid);
    void modelidChanged(const QString& modelid);
    void starterkitidChanged(const QString& starterkitid);

    void usernameChanged(const QString& username);
    void clientkeyChanged(const QString& clientkey);

    void propertyChanged(int role);

private slots:

private:
    void setProperty(int role, const QVariant& prop);
    void updateConfig(const QJsonDocument& json);
    void updateUser(const QJsonDocument& json);
    void updateResource(const QJsonDocument& json);
    void deleteResource(const QJsonDocument& json);

    QString m_bridgeid;
    QHostAddress m_address;
    quint16 m_port;

    QVariantHash m_config;

    QString m_username;
    QString m_clientkey;

    QMap<int, ResourceSortFilterProxyModel*> m_resources;
    bool m_resourcesAllReceived;
    QVector<TestReply*> m_replies;
    TestReply* m_streamReply;
};

#endif // TESTBRIDGE_H
