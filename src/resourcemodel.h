#ifndef RESOURCEMODEL_H
#define RESOURCEMODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

#include "resourceobjects/resourceobject.h"

class ResourceSortFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

    Q_PROPERTY(QStringList filterPath READ filterPath WRITE setFilterPath NOTIFY filterPathChanged)
    Q_PROPERTY(QStringList favoritePath READ favoritePath WRITE setFavoritePath NOTIFY favoritePathChanged)
    Q_PROPERTY(QRegExp favoriteRegExp READ favoriteRegExp WRITE setFavoriteRegExp NOTIFY favoriteRegExpChanged)
    Q_PROPERTY(QStringList sectionPath READ sectionPath WRITE setSectionPath NOTIFY sectionPathChanged)
    Q_PROPERTY(QStringList sortPath READ sortPath WRITE setSortPath NOTIFY sortPathChanged)
    Q_PROPERTY(QStringList resourceOrder READ resourceOrder WRITE setResourceOrder NOTIFY resourceOrderChanged)

public:
    explicit ResourceSortFilterProxyModel(QObject *parent = nullptr);
    virtual ~ResourceSortFilterProxyModel() {}

    enum ResourceRoles {
        RidRole = Qt::UserRole,
        RtypeRole,
        ResourceRole,
        SectionRole
    };
    Q_ENUM(ResourceRoles);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

    const QStringList& sectionPath() const;
    void setSectionPath(const QStringList& path);
    const QStringList& filterPath() const { return m_filterPath; }
    void setFilterPath(const QStringList& path);
    const QStringList& favoritePath() const { return m_favoritePath; }
    void setFavoritePath(const QStringList& path);
    const QRegExp& favoriteRegExp() const { return m_favoriteRegExp; }
    void setFavoriteRegExp(const QRegExp& regexp);
    const QStringList& sortPath() const { return m_sortPath; }
    void setSortPath(const QStringList& path);
    const QStringList& resourceOrder() const { return m_resourceOrder; }
    void setResourceOrder(const QStringList& order);

    Q_INVOKABLE bool moveUp(int source);
    Q_INVOKABLE bool moveDown(int source);

signals:
    void filterPathChanged(const QStringList& path);
    void favoritePathChanged(const QStringList& path);
    void favoriteRegExpChanged(const QRegExp& regexp);
    void sectionPathChanged(const QStringList& path);
    void sortPathChanged(const QStringList& path);
    void resourceOrderChanged(const QStringList& order);

private:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const;
    QVariant resourceValue(const QModelIndex & source, const QStringList& path) const;
    bool resourceContains(const QModelIndex & source, const QStringList& path, const QRegExp& regexp = QRegExp(".+")) const;
    QStringList m_filterPath;
    QStringList m_favoritePath;
    QRegExp m_favoriteRegExp;
    QStringList m_sortPath;
    QStringList m_resourceOrder;
};

class ResourceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ResourceModel(QObject *parent = nullptr);
    virtual ~ResourceModel() {}

    enum ResourceRoles {
        RidRole = Qt::UserRole,
        RtypeRole,
        ResourceRole,
        SectionRole
    };
    QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    const QStringList& sectionPath() const { return m_sectionPath; }
    void setSectionPath(const QStringList& path) { m_sectionPath = path; }

    Q_INVOKABLE ResourceObject* resource(const QString &rid = "");
    int updateResource(const QJsonObject& resource);
    void deleteResource(const QString& rid);

public slots:
    void onResourceChanged(const QJsonObject& rdata);

private:
    QVector<ResourceObject*> m_resources;
    QStringList m_sectionPath;

    int resourceIndex(const QString& rid) const;
    int resourceIndex(const QJsonObject& resource) const;
};

#endif // RESOURCEMODEL_H
