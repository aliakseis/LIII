#include "configurableproxyfactory.h"

#include "settings_declaration.h"

#include <QSettings>

QList<QNetworkProxy> ConfigurableProxyFactory::queryProxy(const QNetworkProxyQuery& query)
{
    Q_UNUSED(query)
    QList<QNetworkProxy> proxies;
    QSettings settings;

    using namespace app_settings;

    const bool useProxy = settings.value(UseProxy, UseProxy_Default).toBool();
    if (useProxy)
    {
        proxies.append(QNetworkProxy(
            QNetworkProxy::Socks5Proxy, 
            settings.value(ProxyAddress).toString(),
            settings.value(ProxyPort).toUInt()));
    }
    //else
    //{
    //	proxies = QNetworkProxyFactory::systemProxyForQuery(query);
    //}

    // Make sure NoProxy is in the list, so that QTcpServer can work:
    // it searches for the first proxy that can has the ListeningCapability capability
    // if none have (as is the case with HTTP proxies), it fails to bind.
    // NoProxy allows it to fallback to the 'no proxy' case and bind.
    proxies.append(QNetworkProxy::NoProxy);

    return proxies;
}
