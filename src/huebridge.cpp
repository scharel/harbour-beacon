#include "huebridge.h"

HueBridge::HueBridge(const QString& bridgeid, const QString &modelid, QObject *parent) : QObject(parent), m_bridgeid(bridgeid)
{
    // Listen to signals of the QNetworkAccessManager class
    connect(&m_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(requireAuthentication(QNetworkReply*,QAuthenticator*)));
    connect(&m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
    connect(&m_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslError(QNetworkReply*,QList<QSslError>)));
    connect(&m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    m_apiLightCommandTimer.setInterval(1000 / HUE_MAX_LIGHT_COMMANDS_PER_SEC);
    m_apiGroupCommandTimer.setInterval(1000 / HUE_MAX_GROUP_COMMANDS_PER_SEC);

    connect(&m_apiLightCommandTimer, SIGNAL(timeout()), this, SLOT(handleLightCommands()));
    connect(&m_apiGroupCommandTimer, SIGNAL(timeout()), this, SLOT(handleGroupCommands()));

    m_streamReply = nullptr;
    for (int i = ResourceLight; i < ResourceAll; ++i) {
        ResourceModel* model = new ResourceModel(this);
        m_resources[i] = new ResourceSortFilterProxyModel(this);
        m_resources[i]->setSourceModel(model);
        m_resources[i]->sort(0);
    }
    m_resourcesAllReceived = false;

    connect(this, SIGNAL(propertyChanged(int)), this, SLOT(onPropertyChanged(int)));
    setProperty(ModelidRole, modelid);
}

HueBridge::~HueBridge() {
    m_manager.deleteLater();
    for (int i = ResourceLight; i < ResourceAll; ++i) {
        m_resources[i]->sourceModel()->deleteLater();
        m_resources[i]->deleteLater();
        m_resources.clear();
    }
}

const QHash<int, QByteArray> HueBridge::m_classRoleNames = QHash<int, QByteArray> (
        {
            {HueBridge::NetworkAccessibleRole, "networkAccessible"},
            {HueBridge::BusyRole, "busy"},
            {HueBridge::ReadyRole, "ready"}
        } );

const QHash<int, QByteArray> HueBridge::m_udnsRoleNames = QHash<int, QByteArray> (
        {
            {HueBridge::AddressRole, "address"},
            {HueBridge::PortRole, "port"}
        } );

const QHash<int, QByteArray> HueBridge::m_configRoleNames = QHash<int, QByteArray> (
        {
            {HueBridge::NameRole, "name"},
            {HueBridge::DatastoreversionRole, "datastoreversion"},
            {HueBridge::SwversionRole, "swversion"},
            {HueBridge::ApiversionRole, "apiversion"},
            {HueBridge::MacRole, "mac"},
            {HueBridge::BridgeidRole, "bridgeid"},
            {HueBridge::FactorynewRole, "factorynew"},
            {HueBridge::ReplacesbridgeidRole, "replacesbridgeid"},
            {HueBridge::ModelidRole, "modelid"},
            {HueBridge::StarterkitidRole, "starterkitid"}
        } );

const QHash<int, QByteArray> HueBridge::m_userRoleNames = QHash<int, QByteArray> (
        {
            {HueBridge::UsernameRole, "username"},
            {HueBridge::ClientkeyRole, "clientkey"}
        } );

const QHash<int, QByteArray> HueBridge::m_resourceEndpoints = QHash<int, QByteArray> (
        {
            {HueBridge::ResourceLight, "light"},
            {HueBridge::ResourceScene, "scene"},
            {HueBridge::ResourceRoom, "room"},
            {HueBridge::ResourceZone, "zone"},
            {HueBridge::ResourceBridgeHome, "bridge_home"},
            {HueBridge::ResourceGroupedLight, "grouped_light"},
            {HueBridge::ResourceDevice, "device"},
            {HueBridge::ResourceBridge, "bridge"},
            {HueBridge::ResourceDevicePower, "device_power"},
            {HueBridge::ResourceZigbeeConnectivity, "zigbee_connectivity"},
            {HueBridge::ResourceZgpConnectivity, "zgp_connectivity"},
            {HueBridge::ResourceMotion, "motion"},
            {HueBridge::ResourceTemperature, "temperature"},
            {HueBridge::ResourceLightLevel, "light_level"},
            {HueBridge::ResourceButton, "button"},
            {HueBridge::ResourceBehaviorScript, "behavior_script"},
            {HueBridge::ResourceBehaviorInstance, "behavior_instance"},
            {HueBridge::ResourceGeofenceClient, "geofence_client"},
            {HueBridge::ResourceGeolocation, "geolocation"},
            {HueBridge::ResourceEntertainmentConfiguration, "entertainment_configuration"},
            {HueBridge::ResourceEntertainment, "entertainment"},
            {HueBridge::ResourceHomekit, "homekit"},
            {HueBridge::ResourceAll, ""}
        } );

bool HueBridge::busy() const {
    bool busy = false;
    QList<QNetworkReply *> replies = m_manager.findChildren<QNetworkReply *>();
    for (int i = 0; i < replies.size() && !busy; ++i) {
        if (replies.at(i) != m_streamReply) {
            busy |= replies.at(i)->isRunning();
        }
    }
    return busy;
}

void HueBridge::setAddress(const QHostAddress& address) {
    if (address != m_address) {
        m_address = address;
        emit propertyChanged(AddressRole);
    }
}

void HueBridge::setPort(quint16 port) {
    if (port != m_port) {
        m_port = port;
        emit propertyChanged(PortRole);
    }
}

void HueBridge::setUsername(const QString &username) {
    if (username != m_username) {
        m_username = username;
        emit propertyChanged(UsernameRole);
    }
}

QVariant HueBridge::property(int role) const {
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

bool HueBridge::createUser(const QString& instance_name) {
    if (!m_address.isNull()) {
        QJsonDocument body( QJsonObject({ {"devicetype", QString("%1#%2").arg(QGuiApplication::applicationName()).arg(instance_name) }, { "generateclientkey", true } }));
        m_manager.post(prepareRequest(BRIDGE_USER_ENDPOINT), body.toJson());
        return true;
    }
    return false;
}

bool HueBridge::getConfig() {
    if (!m_address.isNull()) {
        m_manager.get(prepareRequest(BRIDGE_CONFIG_ENDPOINT));
        return true;
    }
    return false;
}

bool HueBridge::getResource(ResourceType resource, const QString& id) {
    if (!m_address.isNull() && !m_username.isEmpty() && m_resourceEndpoints.contains(resource)) {
        QString endpoint = RESOURCE_ENDPOINT;
        if (resource != ResourceAll) endpoint += "/" + m_resourceEndpoints[resource];
        if (!id.isEmpty())
            endpoint += "/" + id;
        m_manager.get(prepareRequest(endpoint));
        return true;
    }
    return false;
}

bool HueBridge::putResource(ResourceType resource, const QJsonObject &json, const QString& id) {
    QString myId = id;
    if (!m_address.isNull() && !m_username.isEmpty() && m_resourceEndpoints.contains(resource) && resource != ResourceAll) {
        if (myId.isEmpty()) {   // id not provided as parameter but contained in json
            myId = json["id"].toString();
        }
        if (myId.isEmpty() && m_resources[resource]->rowCount() == 1) {      // if no id provided and only one resource available, use this
            myId = m_resources[resource]->data(m_resources[resource]->index(0, 0), ResourceModel::RidRole).toString();
        }
        if (!myId.isEmpty()) {      // do the API call if any id is usable
            QString endpoint = RESOURCE_ENDPOINT + "/" + m_resourceEndpoints[resource] + "/" + myId;
            m_manager.put(prepareRequest(endpoint), QJsonDocument(json).toJson());
            return true;
        }
    }
    return false;
}

bool HueBridge::delResource(ResourceType resource, const QString& id) {
    QString myId = id;
    if (!m_address.isNull() && !m_username.isEmpty() && m_resourceEndpoints.contains(resource) && resource != ResourceAll) {
        if (myId.isEmpty() && m_resources[resource]->rowCount() == 1) {      // if no id provided and only one resource available, use this
            myId = m_resources[resource]->data(m_resources[resource]->index(0, 0), ResourceModel::RidRole).toString();
        }
        if (!myId.isEmpty()) {      // do the API call if any id is usable
            QString endpoint = RESOURCE_ENDPOINT + "/" + m_resourceEndpoints[resource] + "/" + myId;
            m_manager.deleteResource(prepareRequest(endpoint));
            return true;
        }
    }
    return false;
}

bool HueBridge::startEventStream() {
    qDebug() << "Starting event stream";
    if (!m_address.isNull() && !m_username.isEmpty() && m_streamReply == nullptr) {
        m_streamReply = m_manager.get(prepareEventStreamRequest(EVENTSTREAM_ENDPOINT));
        if (m_streamReply->error() == QNetworkReply::NoError) {
            connect(m_streamReply, SIGNAL(readyRead()), this, SLOT(streamEventReceived()));
            emit streamChanged(stream());
        }
        else {
            stopEventStream();
        }
        return true;
    }
    return false;
}

bool HueBridge::stopEventStream() {
    qDebug() << "Stopping event stream";
    if (m_streamReply != nullptr) {
        disconnect(m_streamReply, SIGNAL(readyRead()), this, SLOT(streamEventReceived()));
        m_streamReply->abort();
        m_streamReply->deleteLater();
        m_streamReply = nullptr;
        emit streamChanged(false);
        return true;
    }
    return false;
}

void HueBridge::setLight(const QString &id, const QJsonObject &json) {
    m_lightCommandQueueIDs.enqueue(id);
    m_lightCommandValues.insertMulti(id, json);
    if (!m_apiLightCommandTimer.isActive()) {
        m_apiLightCommandTimer.start();
        handleLightCommands();
    }
}

void HueBridge::setGroup(const QString &id, const QJsonObject &json) {
    m_groupCommandQueueIDs.enqueue(id);
    m_groupCommandValues.insertMulti(id, json);
    if (!m_apiGroupCommandTimer.isActive()) {
        m_apiGroupCommandTimer.start();
        handleGroupCommands();
    }
}

void HueBridge::handleLightCommands() {
    bool done = false;

    if (m_lightCommandQueueIDs.isEmpty()) {
        m_apiLightCommandTimer.stop();
    }

    while (!done && !m_lightCommandQueueIDs.isEmpty()) {
        QString id = m_lightCommandQueueIDs.dequeue();
        m_lightCommandQueueIDs.removeAll(id);
        QList<QJsonObject> values = m_lightCommandValues.values(id);
        m_lightCommandValues.remove(id);
        QVariantHash json;
        while (!values.isEmpty()) {
            json.unite(values.takeFirst().toVariantHash());
        }
        done = putResource(ResourceLight, QJsonObject::fromVariantHash(json), id);
    }
}

void HueBridge::handleGroupCommands() {
    bool done = false;

    if (m_groupCommandQueueIDs.isEmpty()) {
        m_apiGroupCommandTimer.stop();
    }

    while (!done && !m_groupCommandQueueIDs.isEmpty()) {
        QString id = m_groupCommandQueueIDs.dequeue();
        m_groupCommandQueueIDs.removeAll(id);
        QList<QJsonObject> values = m_groupCommandValues.values(id);
        m_groupCommandValues.remove(id);
        QVariantHash json;
        while (!values.isEmpty()) {
            json.unite(values.takeFirst().toVariantHash());
        }
        done = putResource(ResourceGroupedLight, QJsonObject::fromVariantHash(json), id);
    }
}

void HueBridge::requireAuthentication(QNetworkReply *reply, QAuthenticator *authenticator) {
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

void HueBridge::onNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible) {
    emit networkAccessibleChanged(accessible == QNetworkAccessManager::Accessible);
}

void HueBridge::sslError(QNetworkReply *reply, const QList<QSslError> &errors) {
    for (int i = 0; i < errors.size(); ++i) {
        if (errors[i].error() != QSslError::HostNameMismatch)
            qDebug() << errors[i].errorString() << reply->url().toDisplayString();
    }
    //emit apiError(SslHandshakeError);
}

void HueBridge::replyFinished(QNetworkReply* reply) {
    //qDebug() << reply->url().toDisplayString();
    //qDebug() << reply->readAll();

    if (reply == m_streamReply) {
        qDebug() << "Event Stream finished";
        stopEventStream();
        //startEventStream();
    }
    else {
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
            case QNetworkAccessManager::PutOperation:
                break;
            case QNetworkAccessManager::PostOperation:
                if (reply->url().path() == BRIDGE_USER_ENDPOINT) {
                    updateUser(doc);
                }
                break;
            default:
                qDebug() << "Non implementet reply operation" << reply->operation();
                qDebug() << doc;
            }
        }
        else {
            qDebug() << reply->error() << reply->errorString();
        }
    }
    reply->deleteLater();
    emit busyChanged(busy());
}

void HueBridge::streamEventReceived() {
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

void HueBridge::onPropertyChanged(int role) {
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

QNetworkRequest HueBridge::prepareRequest(const QString& endpoint) const {
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

QNetworkRequest HueBridge::prepareEventStreamRequest(const QString &endpoint) const {
    QNetworkRequest request = prepareRequest(endpoint);

    request.setRawHeader(QByteArray("Accept"), QByteArray("text/event-stream"));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    return request;
}

void HueBridge::setProperty(int role, const QVariant &prop) {
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

void HueBridge::updateConfig(const QJsonDocument& json) {
    if (json.isObject()) {
        QJsonObject obj = json.object();
        QHashIterator<int, QByteArray> i(m_configRoleNames);
        while (i.hasNext()) {
            i.next();
            setProperty(i.key(), obj[i.value()].toVariant());
        }
    }
}

void HueBridge::updateUser(const QJsonDocument &json) {
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

void HueBridge::updateResource(const QJsonDocument &json) {
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
        if (resource != ResourceAll) {
            ((ResourceModel*)m_resources[resource]->sourceModel())->updateResource(data[d].toObject());
        }
        //qDebug() << m_resources[resource]->rowCount();
        //qDebug() << "Resource" << m_resourceEndpoints[resource] << "has" << m_resources[resource]->rowCount() << "objects";
    }
}

void HueBridge::deleteResource(const QJsonDocument &json) {
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
