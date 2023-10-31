#ifndef RESOURCEOBJECT_H
#define RESOURCEOBJECT_H

#include <QObject>
#include <QJsonObject>
#include <QDebug>

// TODO check if deriving from QJsonObject suits better
class ResourceObject : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString rid READ rid NOTIFY ridChanged)
    Q_PROPERTY(QString rtype READ rtype NOTIFY rtypeChanged)
    Q_PROPERTY(QJsonObject rdata READ rdata NOTIFY rdataChanged)

public:
    ResourceObject(QObject* parent = nullptr) : QObject(parent) { }
    ResourceObject(const QString& rid, const QString& rtype, QObject* parent = nullptr) : QObject(parent) {
        m_rdata["id"] = rid;
        m_rdata["type"] = rtype;
    }
    ResourceObject(const QJsonObject& rdata, QObject* parent = nullptr) : QObject(parent) {
        m_rdata["id"] = rdata["id"];
        m_rdata["type"] = rdata["type"];
        updateData(rdata);
    }
    ResourceObject(const ResourceObject& other) : QObject(other.parent()) {
        m_rdata = other.rdata();
    }
    ~ResourceObject() {
        qDebug() << "Deleting" << rid() << rtype();
    }
    ResourceObject& operator= (const ResourceObject& other) {
        m_rdata = other.rdata();
        return *this;
    }
    bool operator== (const ResourceObject& other) const {
        return rid() == other.rid();
    }

    Q_INVOKABLE const QVariant value(const QStringList& keys) const {
        QJsonValue value = m_rdata;
        for (int i = 0; i < keys.size(); ++i) {
            value = value.toObject().value(keys.at(i));
        }
        return value;
    }

    const QString rid() const { return m_rdata["id"].toString(); }
    const QString rtype() const { return m_rdata["type"].toString(); }
    const QJsonObject& rdata() const { return m_rdata; }

public slots:
    void updateData(const QJsonObject& rdata) {
        if (rdata != m_rdata) {
            m_rdata = rdata;
            emit rdataChanged(m_rdata);
        }
    }

signals:
    void ridChanged(const QString& rid);
    void rtypeChanged(const QString& rtype);
    void rdataChanged(const QJsonObject& rdata);

protected:
    QJsonObject m_rdata;
};

#endif // RESOURCEOBJECT_H
