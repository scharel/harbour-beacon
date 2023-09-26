#ifndef HUEBRIDGE_H
#define HUEBRIDGE_H

#include <QObject>
#include <QHostInfo>
#include <QHostAddress>
#include <QGuiApplication>
#include <QQueue>
#include <QMultiMap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVersionNumber>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QDebug>

#include "resourcemodel.h"

const quint16 HUE_PORT(443);

// https://developers.meethue.com/develop/application-design-guidance/using-https/
// https://doc.qt.io/qt-6/qsslconfiguration.html#setLocalCertificate
const QByteArray HUE_CERT(
    "-----BEGIN CERTIFICATE-----"
    "MIICMjCCAdigAwIBAgIUO7FSLbaxikuXAljzVaurLXWmFw4wCgYIKoZIzj0EAwIw"
    "OTELMAkGA1UEBhMCTkwxFDASBgNVBAoMC1BoaWxpcHMgSHVlMRQwEgYDVQQDDAty"
    "b290LWJyaWRnZTAiGA8yMDE3MDEwMTAwMDAwMFoYDzIwMzgwMTE5MDMxNDA3WjA5"
    "MQswCQYDVQQGEwJOTDEUMBIGA1UECgwLUGhpbGlwcyBIdWUxFDASBgNVBAMMC3Jv"
    "b3QtYnJpZGdlMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEjNw2tx2AplOf9x86"
    "aTdvEcL1FU65QDxziKvBpW9XXSIcibAeQiKxegpq8Exbr9v6LBnYbna2VcaK0G22"
    "jOKkTqOBuTCBtjAPBgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNV"
    "HQ4EFgQUZ2ONTFrDT6o8ItRnKfqWKnHFGmQwdAYDVR0jBG0wa4AUZ2ONTFrDT6o8"
    "ItRnKfqWKnHFGmShPaQ7MDkxCzAJBgNVBAYTAk5MMRQwEgYDVQQKDAtQaGlsaXBz"
    "IEh1ZTEUMBIGA1UEAwwLcm9vdC1icmlkZ2WCFDuxUi22sYpLlwJY81Wrqy11phcO"
    "MAoGCCqGSM49BAMCA0gAMEUCIEBYYEOsa07TH7E5MJnGw557lVkORgit2Rm1h3B2"
    "sFgDAiEA1Fj/C3AN5psFMjo0//mrQebo0eKd3aWRx+pQY08mk48="
    "-----END CERTIFICATE-----");

// https://developers.meethue.com/develop/hue-api-v2/api-reference/
const QByteArray BRIDGE_CONFIG_ENDPOINT("/api/0/config");
const QByteArray BRIDGE_USER_ENDPOINT("/api");
const QByteArray RESOURCE_ENDPOINT("/clip/v2/resource");
const QByteArray EVENTSTREAM_ENDPOINT("/eventstream/clip/v2");

// https://developers.meethue.com/develop/hue-api-v2/core-concepts/#limitations
const int HUE_MAX_LIGHT_COMMANDS_PER_SEC(10);
const int HUE_MAX_GROUP_COMMANDS_PER_SEC(1);

class HueBridge : public QObject
{
    Q_OBJECT

    // Class state properties
    Q_PROPERTY(bool networkAccessible READ networkAccessible NOTIFY networkAccessibleChanged)   // if network connectivity is accessible
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)          // when an API call is currently running
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)       // when all resources have been received
    Q_PROPERTY(bool stream READ stream NOTIFY streamChanged)    // when API event stream is active

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
    explicit HueBridge(const QString& bridgeid = "", const QString& modelid = "", QObject *parent = nullptr);
    virtual ~HueBridge();

    bool operator==(const HueBridge& other) const { return other.bridgeid() == property(BridgeidRole); }

    const static QHash<int, QByteArray> m_classRoleNames;
    enum BridgeClassRoles {
        NetworkAccessibleRole = Qt::UserRole,
        BusyRole,
        ReadyRole,
        StreamRole,
        DummyLastClassRole
    };
    static QHash<int, QByteArray> classRoleNames();

    const static QHash<int, QByteArray> m_udnsRoleNames;
    enum BridgeUdnsRoles {
        AddressRole = DummyLastClassRole,
        PortRole,
        DummyLastUdnsRole
    };
    static QHash<int, QByteArray> udnsRoleNames();

    const static QHash<int, QByteArray> m_configRoleNames;
    enum BridgeConfigRoles {
        NameRole = DummyLastUdnsRole,
        DatastoreversionRole,
        SwversionRole,
        ApiversionRole,
        MacRole,
        BridgeidRole,
        FactorynewRole,
        ReplacesbridgeidRole,
        ModelidRole,
        StarterkitidRole,
        DummyLastConfigRole
    };
    static QHash<int, QByteArray> configRoleNames();

    const static QHash<int, QByteArray> m_userRoleNames;
    enum BridgeSecuRoles {
        UsernameRole = DummyLastConfigRole,
        ClientkeyRole,
        DummyLastAuthRole
    };
    static QHash<int, QByteArray> secuRoleNames();

    static QHash<int, QByteArray> roleNames() { QHash<int, QByteArray> roleNames; return roleNames.unite(m_classRoleNames).unite(m_udnsRoleNames).unite(m_configRoleNames).unite(m_userRoleNames); }

    const static QHash<int, QByteArray> m_resourceEndpoints;
    enum ResourceType {
        ResourceLight,
        ResourceScene,
        ResourceRoom,
        ResourceZone,
        ResourceBridgeHome,
        ResourceGroupedLight,
        ResourceDevice,
        ResourceBridge,
        ResourceDevicePower,
        ResourceZigbeeConnectivity,
        ResourceZgpConnectivity,
        ResourceMotion,
        ResourceTemperature,
        ResourceLightLevel,
        ResourceButton,
        ResourceBehaviorScript,
        ResourceBehaviorInstance,
        ResourceGeofenceClient,
        ResourceGeolocation,
        ResourceEntertainmentConfiguration,
        ResourceEntertainment,
        ResourceHomekit,
        ResourceAll
    };
    Q_ENUM(ResourceType)

    bool networkAccessible() const { return m_manager.networkAccessible() == QNetworkAccessManager::Accessible; }
    bool busy() const;

    const QHostAddress& address() const { return m_address; }
    void setAddress(const QHostAddress& address);
    qint16 port() const { return m_port; }
    void setPort(quint16 port);
    bool ready() { return m_resourcesAllReceived; }
    bool stream() { return networkAccessible() && (m_streamReply ? m_streamReply->isRunning() && !m_streamReply->isFinished() : false); }

    const QString name() const { return property(NameRole).toString(); }
    const QString datastoreversion() const { return property(DatastoreversionRole).toString(); }
    const QString swversion() const { return property(SwversionRole).toString(); }
    const QString apiversion() const { return property(ApiversionRole).toString(); }
    const QString mac() const { return property(MacRole).toString(); }
    const QString bridgeid() const { return property(BridgeidRole).toString(); }
    bool factorynew() const { return property(FactorynewRole).toBool(); }
    const QString replacesbridgeid() const { return property(ReplacesbridgeidRole).toString(); }
    const QString modelid() const { return property(ModelidRole).toString(); }
    const QString starterkitid() const { return property(StarterkitidRole).toString(); }

    const QString& username() const { return m_username; }
    void setUsername(const QString& username);
    const QString& clientkey() const { return m_clientkey; }

    QVariant property(int role) const;

    Q_INVOKABLE bool createUser(const QString& instance_name = QHostInfo::localHostName());
    Q_INVOKABLE bool getConfig();
    Q_INVOKABLE bool getResource(ResourceType resource = ResourceAll, const QString& id = QString());
    Q_INVOKABLE bool putResource(ResourceType resource, const QJsonObject& json, const QString& id = QString());
    Q_INVOKABLE bool delResource(ResourceType resource, const QString& id = QString());
    Q_INVOKABLE bool startEventStream();
    Q_INVOKABLE bool stopEventStream();

    Q_INVOKABLE void setLight(const QString& id, const QJsonObject& json);
    Q_INVOKABLE void setGroup(const QString& id, const QJsonObject& json);

    Q_INVOKABLE ResourceSortFilterProxyModel* resourceModel(ResourceType type) { return m_resources[type]; }
    Q_INVOKABLE ResourceObject* resource(ResourceType type, const QString& id = "") { return ((ResourceModel*)m_resources[type]->sourceModel())->resource(id); }

signals:
    void networkAccessibleChanged(bool accessible);
    void busyChanged(bool busy);

    void addressChanged(const QHostAddress& address);
    void portChanged(quint16 port);
    void readyChanged(bool ready);
    void streamChanged(bool stream);

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
    void handleLightCommands();
    void handleGroupCommands();
    void requireAuthentication(QNetworkReply * reply, QAuthenticator * authenticator);
    void onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
    void sslError(QNetworkReply* reply, const QList<QSslError> &errors);
    void replyFinished(QNetworkReply* reply);
    void streamEventReceived();

    void onPropertyChanged(int role);

private:
    QNetworkRequest prepareRequest(const QString& endpoint) const;
    QNetworkRequest prepareEventStreamRequest(const QString& endpoint) const;
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

    QNetworkAccessManager m_manager;
    QNetworkReply* m_streamReply;
    QQueue<QString> m_lightCommandQueueIDs;
    QMultiMap<QString, QJsonObject> m_lightCommandValues;
    QQueue<QString> m_groupCommandQueueIDs;
    QMultiMap<QString, QJsonObject> m_groupCommandValues;
    QTimer m_apiLightCommandTimer;
    QTimer m_apiGroupCommandTimer;
};

#endif // HUEBRIDGE_H
