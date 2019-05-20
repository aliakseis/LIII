#pragma once

#include <QNetworkProxyFactory>

class ConfigurableProxyFactory : public QNetworkProxyFactory
{
public:
    QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery& query) override;
};
