#ifndef LIGHTOBJECT_H
#define LIGHTOBJECT_H

#include "resourceobject.h"

class LightObject : public ResourceObject {
    Q_OBJECT

    Q_PROPERTY(QString ownerRid READ ownerRid NOTIFY ownerRidChanged)
    Q_PROPERTY(QString ownerRtype READ ownerRtype NOTIFY ownerRtypeChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString archetype READ archetype NOTIFY archetypeChanged)
    Q_PROPERTY(int fixed_mired READ fixed_mired CONSTANT)
    Q_PROPERTY(bool on READ on NOTIFY onChanged)
    Q_PROPERTY(bool dimmable READ dimmable CONSTANT)
    Q_PROPERTY(int brightness READ brightness NOTIFY brightnessChanged)     // only valid if dimmable
    Q_PROPERTY(int min_dim_level READ min_dim_level CONSTANT)               // only valid if dimmable
    Q_PROPERTY(bool mirek_valid READ mirek_valid CONSTANT)
    Q_PROPERTY(int mirek READ mirek NOTIFY mirekChanged)                    // only valid if mirek_valid
    Q_PROPERTY(int mirek_minimum READ mirek_minimum CONSTANT)               // only valid if mirek_valid
    Q_PROPERTY(int mirek_maximum READ mirek_maximum CONSTANT)               // only valid if mirek_valid
    Q_PROPERTY(bool color_valid READ color_valid CONSTANT)
    Q_PROPERTY(double colorX READ colorX NOTIFY colorXChanged)              // only valid if color_valid
    Q_PROPERTY(double colorY READ colorY NOTIFY colorYChanged)              // only valid if color_valid

public:
    LightObject(ResourceObject* parent = nullptr) : ResourceObject(parent) { }

    const QString ownerRid() const { return ResourceObject::value(QStringList() << "owner" << "rid").toString(); }
    const QString ownerRtype() const { return ResourceObject::value(QStringList() << "owner" << "rtype").toString(); }
    const QString name() const { return ResourceObject::value(QStringList() << "metadata" << "name").toString(); }
    const QString archetype() const { return ResourceObject::value(QStringList() << "metadata" << "archetype").toString(); }
    int fixed_mired() const { return ResourceObject::value(QStringList() << "metadata" << "fixed_mired").toInt(); }
    bool on() const { return ResourceObject::value(QStringList() << "on" << "on").toBool(); }
    bool dimmable() const { return ResourceObject::m_rdata.contains("dimming"); }
    int brightness() const { return ResourceObject::value(QStringList() << "dimming" << "brightness").toInt(); }
    int min_dim_level() const { return ResourceObject::value(QStringList() << "dimming" << "min_dim_level").toInt(); }
    bool mirek_valid() const { return ResourceObject::value(QStringList() << "color_temperature" << "mirek_valid").toBool(); }
    int mirek() const { return ResourceObject::value(QStringList() << "color_temperature" << "mirek").toInt(); }
    int mirek_minimum() const { return ResourceObject::value(QStringList() << "color_temperature" << "mirek_schema" << "mirek_minimum").toInt(); }
    int mirek_maximum() const { return ResourceObject::value(QStringList() << "color_temperature" << "mirek_schema" << "mirek_maximum").toInt(); }
    bool color_valid() const { return ResourceObject::m_rdata.contains("color"); }
    double colorX() const { return ResourceObject::value(QStringList() << "color" << "xy" << "x").toDouble(); }
    double colorY() const { return ResourceObject::value(QStringList() << "color" << "xy" << "y").toDouble(); }

signals:
    void ownerRidChanged(const QString& ownerRid);
    void ownerRtypeChanged(const QString& ownerRtype);
    void nameChanged(const QString& name);
    void archetypeChanged(const QString& archetype);
    void onChanged(bool on);
    void brightnessChanged(int brightness);
    void mirekChanged(int mirek);
    void colorXChanged(double colorX);
    void colorYChanged(double colorY);

protected slots:
    void updateData(const QJsonObject& rdata) {
        if (rdata != m_rdata) {
            // TODO check for updated properties
            ResourceObject::updateData(rdata);
        }
    }
};

#endif // LIGHTOBJECT_H
