#include "../huebridge.h"
#include "testbridge.h"

TestBridge::TestBridge(const QString& bridgeid, const QString &modelid, QObject *parent) : HueBridge(bridgeid, modelid, parent),  m_bridgeid(bridgeid)
{
    // Listen to signals of the QNetworkAccessManager class
    //connect(&m_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(requireAuthentication(QNetworkReply*,QAuthenticator*)));
    //connect(&m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    //connect(&m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslError(QNetworkReply*,QList<QSslError>)));
    //connect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    m_streamReply = nullptr;
    for (int i = ResourceLight; i < ResourceAll; ++i) {
        ResourceModel* model = new ResourceModel(this);
        m_resources[i] = new ResourceSortFilterProxyModel(this);
        m_resources[i]->setSourceModel(model);
        m_resources[i]->setSortRole(ResourceModel::SectionRole);
        m_resources[i]->sort(0);
    }
    m_resourcesAllReceived = false;

    connect(this, SIGNAL(propertyChanged(int)), this, SLOT(onPropertyChanged(int)));
    setProperty(ModelidRole, modelid);
}

TestBridge::~TestBridge() {
    while (!m_replies.empty()) {
        QNetworkReply* reply = m_replies.first();
        reply->abort();
        reply->deleteLater();
        m_replies.removeFirst();
    }
    for (int i = ResourceLight; i < ResourceAll; ++i) {
        m_resources[i]->sourceModel()->deleteLater();
        m_resources[i]->deleteLater();
        m_resources.clear();
    }
}

const QHash<int, QByteArray> TestBridge::m_udnsRoleNames = QHash<int, QByteArray> (
        {
            {TestBridge::AddressRole, "address"},
            {TestBridge::PortRole, "port"}
        } );

const QHash<int, QByteArray> TestBridge::m_configRoleNames = QHash<int, QByteArray> (
        {
            {TestBridge::NameRole, "name"},
            {TestBridge::DatastoreversionRole, "datastoreversion"},
            {TestBridge::SwversionRole, "swversion"},
            {TestBridge::ApiversionRole, "apiversion"},
            {TestBridge::MacRole, "mac"},
            {TestBridge::BridgeidRole, "bridgeid"},
            {TestBridge::FactorynewRole, "factorynew"},
            {TestBridge::ReplacesbridgeidRole, "replacesbridgeid"},
            {TestBridge::ModelidRole, "modelid"},
            {TestBridge::StarterkitidRole, "starterkitid"}
        } );

const QHash<int, QByteArray> TestBridge::m_userRoleNames = QHash<int, QByteArray> (
        {
            {TestBridge::UsernameRole, "username"},
            {TestBridge::ClientkeyRole, "clientkey"}
        } );

const QHash<int, QByteArray> TestBridge::m_resourceEndpoints = QHash<int, QByteArray> (
        {
            {TestBridge::ResourceLight, "light"},
            {TestBridge::ResourceScene, "scene"},
            {TestBridge::ResourceRoom, "room"},
            {TestBridge::ResourceZone, "zone"},
            {TestBridge::ResourceBridgeHome, "bridge_home"},
            {TestBridge::ResourceGroupedLight, "grouped_light"},
            {TestBridge::ResourceDevice, "device"},
            {TestBridge::ResourceBridge, "bridge"},
            {TestBridge::ResourceDevicePower, "device_power"},
            {TestBridge::ResourceZigbeeConnectivity, "zigbee_connectivity"},
            {TestBridge::ResourceZgpConnectivity, "zgp_connectivity"},
            {TestBridge::ResourceMotion, "motion"},
            {TestBridge::ResourceTemperature, "temperature"},
            {TestBridge::ResourceLightLevel, "light_level"},
            {TestBridge::ResourceButton, "button"},
            {TestBridge::ResourceBehaviorScript, "behavior_script"},
            {TestBridge::ResourceBehaviorInstance, "behavior_instance"},
            {TestBridge::ResourceGeofenceClient, "geofence_client"},
            {TestBridge::ResourceGeolocation, "geolocation"},
            {TestBridge::ResourceEntertainmentConfiguration, "entertainment_configuration"},
            {TestBridge::ResourceEntertainment, "entertainment"},
            {TestBridge::ResourceHomekit, "homekit"},
            {TestBridge::ResourceAll, ""}
        } );

void TestBridge::setAddress(const QHostAddress& address) {
    if (address != m_address) {
        m_address = address;
        emit propertyChanged(AddressRole);
    }
}

void TestBridge::setPort(quint16 port) {
    if (port != m_port) {
        m_port = port;
        emit propertyChanged(PortRole);
    }
}

void TestBridge::setUsername(const QString &username) {
    if (username != m_username) {
        m_username = username;
        emit propertyChanged(UsernameRole);
    }
}

QVariant TestBridge::property(int role) const {
    QVariant prop;
    switch (role) {
    case BridgeidRole:
        prop = m_bridgeid;
        break;
    case AddressRole:
        prop = m_address.toString();
        break;
    case PortRole:
        prop = m_port;
        break;
    case NameRole:
    case DatastoreversionRole:
    case SwversionRole:
    case ApiversionRole:
    case MacRole:
    case FactorynewRole:
    case ReplacesbridgeidRole:
    case ModelidRole:
    case StarterkitidRole:
        prop = m_config.value(m_configRoleNames[role], QString());
        break;
    case UsernameRole:
        prop = m_username;
        break;
    case ClientkeyRole:
        prop = m_clientkey;
        break;
    }
    return prop;
}

bool TestBridge::createUser(const QString& instance_name) {
    if (!m_address.isNull()) {
        QJsonDocument body( QJsonObject({ {"devicetype", QString("%1#%2").arg(QGuiApplication::applicationName()).arg(instance_name) }, { "generateclientkey", true } }));
        m_replies << m_manager.post(prepareRequest(BRIDGE_USER_ENDPOINT), body.toJson());
        if (m_replies.count() == 1)
            emit busyChanged(true);
        return true;
    }
    return false;
}

bool TestBridge::getConfig() {
    if (!m_address.isNull()) {
        m_replies << m_manager.get(prepareRequest(BRIDGE_CONFIG_ENDPOINT));
        if (m_replies.count() == 1)
            emit busyChanged(true);
        return true;
    }
    return false;
}

bool TestBridge::getResource(ResourceType resource, const QString& id) {
    if (!m_address.isNull() && !m_username.isEmpty() && m_resourceEndpoints.contains(resource)) {
        QString endpoint = RESOURCE_ENDPOINT;
        if (resource != ResourceAll) endpoint += "/" + m_resourceEndpoints[resource];
        if (!id.isEmpty())
            endpoint += "/" + id;
        m_replies << m_manager.get(prepareRequest(endpoint));
        if (m_replies.count() == 1)
            emit busyChanged(true);
        return true;
    }
    return false;
}

bool TestBridge::putResource(ResourceType resource, const QJsonObject &json, const QString& id) {
    QString myId = id;
    if (!m_address.isNull() && !m_username.isEmpty() && m_resourceEndpoints.contains(resource) && resource != ResourceAll) {
        if (myId.isEmpty()) {   // id not provided as parameter but contained in json
            myId = json["id"].toString();
        }
        if (myId.isEmpty() && m_resources[resource]->rowCount() == 1) {      // if no id provided and only one resource available, use this
            myId = m_resources[resource]->data(m_resources[resource]->index(0, 0), ResourceModel::IdRole).toString();
        }
        if (!myId.isEmpty()) {      // do the API call if any id is usable
            QString endpoint = RESOURCE_ENDPOINT + "/" + m_resourceEndpoints[resource] + "/" + myId;
            m_manager.put(prepareRequest(endpoint), QJsonDocument(json).toJson());
        }
        return true;
    }
    return false;
}

bool TestBridge::startEventStream() {
    if (!m_address.isNull() && !m_username.isEmpty() && m_streamReply == nullptr) {
        m_streamReply = m_manager.get(prepareEventStreamRequest(EVENTSTREAM_ENDPOINT));
        connect(m_streamReply, SIGNAL(readyRead()), this, SLOT(streamEventReceived()));
        return true;
    }
    return false;
}

bool TestBridge::stopEventStream() {
    if (m_streamReply != nullptr) {
        disconnect(m_streamReply, SIGNAL(readyRead()), this, SLOT(streamEventReceived()));
        m_streamReply->abort();
        m_streamReply->deleteLater();
        m_streamReply = nullptr;
        return true;
    }
    return false;
}

void TestBridge::requireAuthentication(QNetworkReply *reply, QAuthenticator *authenticator) {
    Q_UNUSED(reply)
    Q_UNUSED(authenticator)
    /*if (reply && authenticator) {
        authenticator->setUser(username());
        authenticator->setPassword(password());
    }
    else
        emit apiError(AuthenticationError);
    */
}

void TestBridge::onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
    emit networkAccessibleChanged(accessible == QNetworkAccessManager::Accessible);
}

void TestBridge::sslError(QNetworkReply *reply, const QList<QSslError> &errors) {
    for (int i = 0; i < errors.size(); ++i) {
        if (errors[i].error() != QSslError::HostNameMismatch)
            qDebug() << errors[i].errorString() << reply->url().toDisplayString();
    }
    //emit apiError(SslHandshakeError);
}

void TestBridge::replyFinished(QNetworkReply* reply) {
    //qDebug() << reply->url().toDisplayString();
    //qDebug() << reply->readAll();

    if (reply == m_streamReply) {
        qDebug() << "Event Stream finished, restarting";
        stopEventStream();
        startEventStream();
    }
    else if (m_replies.contains(reply)) {
        if (reply->error() == QNetworkReply::NoError) {
            QString path = reply->url().path();
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            //qDebug() << path;
            //qDebug() << doc;
            switch (reply->operation()) {
            case QNetworkAccessManager::GetOperation:
                if (path == BRIDGE_CONFIG_ENDPOINT) {
                    updateConfig(doc);
                }
                else if (path.startsWith(RESOURCE_ENDPOINT)) {

                    updateResource(doc);
                    if (path.section('/', 4, 4).isEmpty()) {
                        if (!m_resourcesAllReceived) {
                            m_resourcesAllReceived = true;
                            emit readyChanged(m_resourcesAllReceived);
                        }
                    }
                }
                break;
            case QNetworkAccessManager::PostOperation:
                if (reply->url().path() == BRIDGE_USER_ENDPOINT) {
                    updateUser(doc);
                }
                break;
            default:
                qDebug() << "Non implementet reply operation" << reply->operation();
            }
        }
        else {
            qDebug() << reply->error() << reply->errorString();
        }
        m_replies.removeAll(reply);
        if (m_replies.count() == 0)
            emit busyChanged(false);
    }
    else if (reply->operation() == QNetworkAccessManager::PutOperation ||
             reply->operation() == QNetworkAccessManager::PostOperation ||
             reply->operation() == QNetworkAccessManager::DeleteOperation) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QJsonArray errors = doc.object().value("errors").toArray();
        for (int e = 0; e < errors.size(); ++e) {
            qDebug() << errors[e];
        }
    }
    else {
        qDebug() << "Unknown reply";
        qDebug() << reply->readAll();
    }
}

void TestBridge::streamEventReceived() {
    //qDebug() << "Event Stream Reveived";
    QString data = QString(m_streamReply->readAll()).remove(QRegExp("^id: \\d+:\\d+\n*data: "));
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
    if (!doc.isNull() && doc.isArray()) {
        QJsonArray arr = doc.array();
        if (arr.size() > 0) {
            QJsonArray::const_iterator i = arr.constBegin();
            QJsonObject obj;
            do {
                obj = i->toObject();
                if (obj["type"].toString() == "add" || obj["type"].toString() == "update") {
                    updateResource(QJsonDocument(obj));
                }
                else if (obj["type"].toString() == "delete") {
                    deleteResource(QJsonDocument(obj));
                }
                else if (obj["type"].toString() == "error") {
                    qDebug() << "Stream event error" << obj;
                }
                else {
                    qDebug() << "Unknown stream event";
                }
                i++;
            } while (i < arr.constEnd());
        }
    }
}

void TestBridge::onPropertyChanged(int role) {
    QString key = m_configRoleNames[role];
    switch (role) {
    case BridgeidRole:
        emit bridgeidChanged(m_bridgeid);
        break;
    case AddressRole:
        emit addressChanged(m_address);
        break;
    case PortRole:
        emit portChanged(m_port);
        break;
    case NameRole:
        emit nameChanged(property(role).toString());
        break;
    case DatastoreversionRole:
        emit datastoreversionChanged(property(role).toString());
        break;
    case SwversionRole:
        emit swversionChanged(property(role).toString());
        break;
    case ApiversionRole:
        emit apiversionChanged(property(role).toString());
        break;
    case MacRole:
        emit macChanged(property(role).toString());
        break;
    case FactorynewRole:
        emit factorynewChanged(property(role).toBool());
        break;
    case ReplacesbridgeidRole:
        emit replacesbridgeidChanged(property(role).toString());
        break;
    case ModelidRole:
        emit modelidChanged(property(role).toString());
        break;
    case StarterkitidRole:
        emit starterkitidChanged(property(role).toString());
        break;
    case UsernameRole:
        emit usernameChanged(m_username);
        break;
    case ClientkeyRole:
        emit clientkeyChanged(m_clientkey);
        break;
    }
}

QNetworkRequest TestBridge::prepareRequest(const QString& endpoint) const {
    QUrl url("https://" + m_address.toString() + endpoint);
    QNetworkRequest request;

    // Prepare the QNetworkRequest classes
    QSslConfiguration ssl = request.sslConfiguration();
    ssl.setCaCertificates(QSslCertificate::fromData(HUE_CERT, QSsl::Pem));
    //ssl.setSslOption(QSsl::SslOptionDisableServerNameIndication, true);
    ssl.setPeerVerifyMode(QSslSocket::VerifyNone);
    request.setSslConfiguration(ssl);
    request.setHeader(QNetworkRequest::UserAgentHeader, QGuiApplication::applicationDisplayName() +  " " + QGuiApplication::applicationVersion() + " - " + QSysInfo::machineHostName());
    request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json").toUtf8());
    request.setRawHeader("hue-application-key", m_username.toUtf8());

    request.setUrl(url);
    return request;
}

QNetworkRequest TestBridge::prepareEventStreamRequest(const QString &endpoint) const {
    QNetworkRequest request = prepareRequest(endpoint);

    request.setRawHeader(QByteArray("Accept"), QByteArray("text/event-stream"));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    return request;
}

void TestBridge::setProperty(int role, const QVariant &prop) {
    QString tmp;
    //QHostInfo brideHost;
    switch (role) {
    case BridgeidRole:
        tmp = prop.toString();
        //brideHost = QHostInfo::fromName(tmp + ".local");
        //qDebug() << brideHost.addresses();
        if (m_bridgeid != tmp) {
            m_bridgeid = tmp;
            emit propertyChanged(role);
        }
        break;
    case AddressRole:
        setAddress(QHostAddress(prop.toString()));
        break;
    case PortRole:
        setPort(prop.toUInt());
        break;
    case NameRole:
    case DatastoreversionRole:
    case SwversionRole:
    case ApiversionRole:
    case MacRole:
    case FactorynewRole:
    case ReplacesbridgeidRole:
    case ModelidRole:
    case StarterkitidRole:
        tmp = m_configRoleNames[role];
        if (m_config[tmp] != prop) {
            m_config[tmp] = prop;
            emit propertyChanged(role);
        }
        break;
    case UsernameRole:
        setUsername(prop.toString());
        break;
    case ClientkeyRole:
        tmp = prop.toString();
        if (m_clientkey != tmp) {
            m_clientkey = tmp;
            emit propertyChanged(role);
        }
        break;
    }
}

void TestBridge::updateConfig(const QJsonDocument& json) {
    if (json.isObject()) {
        QJsonObject obj = json.object();
        QHashIterator<int, QByteArray> i(m_configRoleNames);
        while (i.hasNext()) {
            i.next();
            setProperty(i.key(), obj[i.value()].toVariant());
        }
    }
}

void TestBridge::updateUser(const QJsonDocument &json) {
    if (json.isArray()) {
        QJsonArray arr = json.array();
        if (arr.size() > 0) {
            QJsonArray::const_iterator i = arr.constBegin();
            QJsonObject obj;
            do {
                obj = i->toObject();
                i++;
            } while (!obj.contains("success") && i < arr.constEnd());
            if (obj.contains("success")) {
                obj = obj["success"].toObject();
                setProperty(UsernameRole, obj.value(m_userRoleNames[UsernameRole]));
                setProperty(ClientkeyRole, obj.value(m_userRoleNames[ClientkeyRole]));
            }
        }
    }
}

void TestBridge::updateResource(const QJsonDocument &json) {
    int resource = ResourceAll;
    QJsonArray errors = json.object().value("errors").toArray();
    QJsonArray data = json.object().value("data").toArray();

    for (int e = 0; e < errors.size(); ++e) {
        qDebug() << errors[e];
    }

    for (int d = 0; d < data.size(); ++d) {
        resource = m_resourceEndpoints.key(data[d].toObject()["type"].toString().toUtf8());
        //qDebug() << resource;
        //qDebug() << m_resourceEndpoints[resource];
        //qDebug() << data[d];
        if (resource != ResourceAll) ((ResourceModel*)m_resources[resource]->sourceModel())->updateResource(data[d].toObject());
        //qDebug() << m_resources[resource]->rowCount();
        //qDebug() << "Resource" << m_resourceEndpoints[resource] << "has" << m_resources[resource]->rowCount() << "objects";
    }
}

void TestBridge::deleteResource(const QJsonDocument &json) {
    int resource = ResourceAll;
    QJsonArray errors = json.object().value("errors").toArray();
    QJsonArray data = json.object().value("data").toArray();

    for (int e = 0; e < errors.size(); ++e) {
        qDebug() << errors[e];
    }

    for (int d = 0; d < data.size(); ++d) {
        resource = m_resourceEndpoints.key(data[d].toObject()["type"].toString().toUtf8());
        ((ResourceModel*)m_resources[resource]->sourceModel())->deleteResource(data[d].toObject()["id"].toString());
    }
}
