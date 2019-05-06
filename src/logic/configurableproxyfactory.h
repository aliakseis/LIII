#pragma once

#include <QNetworkProxyFactory>

class ConfigurableProxyFactory : public QNetworkProxyFactory
{
public:
	virtual QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery& query) override;
};
