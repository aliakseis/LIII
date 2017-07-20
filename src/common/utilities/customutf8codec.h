#pragma once


#include <QTextCodec>

#include <QByteArray>

#include "singleton.h"

namespace utilities
{

class CustomUtf8Codec: public QTextCodec
{
public:
    CustomUtf8Codec();
    ~CustomUtf8Codec();

    QString convertToUnicode(const char*, int, ConverterState*) const override;
    QByteArray convertFromUnicode(const QChar*, int, ConverterState*) const override;

    QByteArray name() const override;
    int mibEnum() const override;

    static CustomUtf8Codec* Instance();

private:
    mutable QByteArray m_b;
    mutable QByteArray m_cache[1024];
};

} // namespace utilities
