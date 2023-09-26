#include "resourcemodel.h"

#include <QQmlEngine>

ResourceSortFilterProxyModel::ResourceSortFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent) {
}

bool ResourceSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    Q_UNUSED(source_parent)
    ResourceModel* model = (ResourceModel*)sourceModel();
    if (!m_filterPath.isEmpty()) {
        return resourceContains(model->index(source_row), m_filterPath, filterRegExp());
    }
    return true;
}

const QStringList& ResourceSortFilterProxyModel::sectionPath() const {
    ResourceModel* model = (ResourceModel*)sourceModel();
    return model->sectionPath();
}

void ResourceSortFilterProxyModel::setSectionPath(const QStringList &path) {
    ResourceModel* model = (ResourceModel*)sourceModel();
    if (path != model->sectionPath()) {
        model->setSectionPath(path);
        emit sectionPathChanged(model->sectionPath());
    }
}
void ResourceSortFilterProxyModel::setFilterPath(const QStringList &path) {
    if (path != m_filterPath) {
        m_filterPath = path;
        emit filterPathChanged(m_filterPath);
        sort(0);
    }
}

void ResourceSortFilterProxyModel::setFavoritePath(const QStringList &path) {
    if (path != m_favoritePath) {
        m_favoritePath = path;
        emit favoritePathChanged(m_favoritePath);
        sort(0);
    }
}

void ResourceSortFilterProxyModel::setFavoriteRegExp(const QRegExp &regexp) {
    if (regexp != m_favoriteRegExp) {
        m_favoriteRegExp = regexp;
        emit favoriteRegExpChanged(m_favoriteRegExp);
        sort(0);
    }
}

void ResourceSortFilterProxyModel::setResourceOrder(const QStringList &order) {
    if (order != m_resourceOrder) {
        m_resourceOrder = order;
        emit resourceOrderChanged(m_resourceOrder);
        sort(0);
    }
}

bool ResourceSortFilterProxyModel::moveUp(int source) {
    // TODO check if upper item is a favorite
    if (source > 0 && source < sourceModel()->rowCount()) {
        beginMoveRows(QModelIndex(), source, source, QModelIndex(), source-1);
        m_resourceOrder.clear();
        for (int i = 0; i < rowCount(); ++i) {
            m_resourceOrder << data(index(i, 0), RidRole).toString();
        }
        m_resourceOrder.move(source, source-1);
        emit resourceOrderChanged(m_resourceOrder);
        endMoveRows();
        return true;
    }
    return false;
}

bool ResourceSortFilterProxyModel::moveDown(int source) {
    // TODO check if item is a favorite
    if (source >= 0 && source < sourceModel()->rowCount()-1) {
        beginMoveRows(QModelIndex(), source, source, QModelIndex(), source+1);
        m_resourceOrder.clear();
        for (int i = 0; i < rowCount(); ++i) {
            m_resourceOrder << data(index(i, 0), RidRole).toString();
        }
        m_resourceOrder.move(source, source+1);
        emit resourceOrderChanged(m_resourceOrder);
        endMoveRows();
        return true;
    }
    return false;
}

bool ResourceSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const {
    if (!m_favoritePath.isEmpty() && !m_favoriteRegExp.isEmpty() &&
            (resourceContains(source_left, m_favoritePath, m_favoriteRegExp) ^ resourceContains(source_right, m_favoritePath, m_favoriteRegExp))) {
        return resourceContains(source_left, m_favoritePath, m_favoriteRegExp) && !resourceContains(source_right, m_favoritePath, m_favoriteRegExp);
    }
    if (m_resourceOrder.contains(source_left.data(RidRole).toString()) || m_resourceOrder.contains(source_right.data(RidRole).toString())) {
        //qDebug() << source_left.data(RidRole).toString() << source_right.data(RidRole).toString() << (m_resourceOrder.indexOf(source_left.data(RidRole).toString()) < m_resourceOrder.indexOf(source_right.data(RidRole).toString()));
        return m_resourceOrder.indexOf(source_left.data(RidRole).toString()) < m_resourceOrder.indexOf(source_right.data(RidRole).toString());
    }
    return source_left.data(sortRole()) < source_right.data(sortRole());
}

bool ResourceSortFilterProxyModel::resourceContains(const QModelIndex &source, const QStringList &path, const QRegExp &regexp) const {
    QJsonValue val = QJsonValue::fromVariant(source.data(ResourceModel::ResourceRole));
    for (int i = 0; i < path.size(); ++i) {
        val = val.toObject()[path.at(i)];
    }
    //qDebug() << val.toString() << regexp << val.toString().contains(regexp);
    return val.toString().contains(regexp);
}

ResourceModel::ResourceModel(QObject *parent) : QAbstractListModel(parent)
{

}

QHash<int, QByteArray> ResourceModel::roleNames() const {
    return QHash<int, QByteArray> {
        {RidRole, "rid"},
        {RtypeRole, "rtype"},
        {ResourceRole, "resource"},
        {SectionRole, "section"}
    };
}

Qt::ItemFlags ResourceModel::flags(const QModelIndex &index) const {
    if (index.isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable; // | Qt::ItemIsEditable;
    }
    else {
        return Qt::NoItemFlags;
    }
}

int ResourceModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_resources.count();
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const {
    QVariant data;
    if (index.isValid() && index.row() < m_resources.count() && roleNames().contains(role)) {
        QJsonValue val;
        switch (role) {
        case RidRole:
            data = m_resources[index.row()]->rid();
            break;
        case RtypeRole:
            data = m_resources[index.row()]->rtype();
            break;
        case ResourceRole:
            data = m_resources[index.row()]->rdata();
            break;
        case SectionRole:
            val = m_resources[index.row()]->rdata();
            for (int i = 0; i < m_sectionPath.size(); ++i) {
                val = val.toObject()[m_sectionPath.at(i)];
            }
            data = val.toVariant();
            break;
        }
    }
    return data;
}

ResourceObject* ResourceModel::resource(const QString &rid) {
    int row = resourceIndex(rid);
    if (row < 0) {
        if (m_resources.count() == 1) {
            row = 0;
        }
        else {
            row = updateResource(QJsonObject{{"id", rid}});
        }
    }
    if (row >=0) {
        return m_resources[row];
    }
    return nullptr;
}

int ResourceModel::updateResource(const QJsonObject &resource) {
    int row = resourceIndex(resource);
    if (row >= 0) {
        //qDebug() << "Updating resource " << row << resource["id"];
        m_resources[row]->updateData(resource);
        emit dataChanged(index(row), index(row));
    }
    else {
        row = m_resources.size();
        //qDebug() << "Adding resource " << row << resource["id"];
        emit beginInsertRows(QModelIndex(), row, row);
        m_resources << new ResourceObject(resource);
        QQmlEngine::setObjectOwnership(m_resources.last(), QQmlEngine::CppOwnership);   // prevent QML from deleting resource objects
        connect(m_resources.last(), SIGNAL(rdataChanged(const QJsonObject&)), this, SLOT(onResourceChanged(const QJsonObject&)));
        emit endInsertRows();
    }
    return row;
}

void ResourceModel::deleteResource(const QString &rid) {
    beginResetModel();
    ResourceObject tmp;
    tmp.setProperty("id", rid);
    m_resources.removeAll(&tmp);
    endResetModel();
}

void ResourceModel::onResourceChanged(const QJsonObject& rdata) {
    int row = resourceIndex(rdata);
    if (row >= 0) {
        emit dataChanged(index(row), index(row));
    }
}

int ResourceModel::resourceIndex(const QString &rid) const {
    for (int i = 0; i < m_resources.size(); ++i) {
        if (!m_resources[i]->rid().isEmpty() && m_resources[i]->rid() == rid) {
            return i;
        }
    }
    return -1;
}

int ResourceModel::resourceIndex(const QJsonObject &resource) const {
    return resourceIndex(resource["id"].toString());
}
