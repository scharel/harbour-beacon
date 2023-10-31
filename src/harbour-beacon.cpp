//#define TEST    // Uncomment to test without a real Hue Bridge

#include <QtQuick>
#include <sailfishapp.h>
#include <QtQml>
#include <QObject>
#include "huediscovery.h"
#ifdef TEST
    #include "test/testbridge.h"
#else
    #include "huebridge.h"
#endif
#include "resourcemodel.h"
#include "resourceobjects/resourceobject.h"
#include "resourceobjects/lightobject.h"

int main(int argc, char *argv[])
{
    QGuiApplication* app = SailfishApp::application(argc, argv);
    app->setApplicationDisplayName("Beacon");
    app->setApplicationName("harbour-beacon");
    app->setApplicationVersion(APP_VERSION);
    app->setOrganizationDomain("https://github.com/scharel");
    app->setOrganizationName("harbour-beacon");

    qDebug() << app->applicationDisplayName() << app->applicationVersion();

    HueDiscovery::instantiate(app);
    qmlRegisterSingletonType<HueDiscovery>("harbour.beacon", 1, 0, "HueDiscovery", HueDiscovery::provider);
#ifdef TEST
    qmlRegisterType<TestBridge>("harbour.beacon", 1, 0, "HueBridge");
#else
    qmlRegisterType<HueBridge>("harbour.beacon", 1, 0, "HueBridge");
#endif
    qmlRegisterType<ResourceSortFilterProxyModel>("harbour.beacon", 1, 0, "ResourceModel");
    //qmlRegisterType<ResourceModel>("harbour.beacon", 1, 0, "ResourceModel");
    qmlRegisterType<ResourceObject>("harbour.beacon", 1, 0, "ResourceObject");
    qmlRegisterType<LightObject>("harbour.beacon", 1, 0, "LightObject");

    QQuickView* view = SailfishApp::createView();
#ifdef QT_DEBUG
    view->rootContext()->setContextProperty("debug", QVariant(true));
#else
    view->rootContext()->setContextProperty("debug", QVariant(false));
#endif

    view->setSource(SailfishApp::pathTo("qml/harbour-beacon.qml"));
    view->show();

    int retval = app->exec();

    return retval;
}
