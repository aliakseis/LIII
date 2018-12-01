#include "customutf8codec.h"

#include "utilities/utils.h"

#include <QVarLengthArray>



#pragma intrinsic(memcpy)


namespace utilities
{

enum
{
    LastValidCodePoint = 0x10ffff,
};

inline bool isNonCharacter(uint ucs4)
{
    return ucs4 >= 0xfdd0 && (ucs4 <= 0xfdef || (ucs4 & 0xfffe) == 0xfffe);
}

inline bool isSurrogate(uint ucs4)
{
    return (ucs4 - 0xd800u < 2048u);
}


CustomUtf8Codec::CustomUtf8Codec()
    : m_b(4096, 0)
{
}

CustomUtf8Codec::~CustomUtf8Codec()
= default;

QString CustomUtf8Codec::convertToUnicode(const char* chars, int len, ConverterState* state) const
{
    bool headerdone = false;
    ushort replacement = QChar::ReplacementCharacter;
    int need = 0;
    int error = -1;
    uint uc = 0;
    uint min_uc = 0;
    if (state)
    {
        if (state->flags & QTextCodec::IgnoreHeader)
        {
            headerdone = true;
        }
        if (state->flags & QTextCodec::ConvertInvalidToNull)
        {
            replacement = QChar::Null;
        }
        need = state->remainingChars;
        if (need)
        {
            uc = state->state_data[0];
            min_uc = state->state_data[1];
        }
    }
    if (!headerdone && len > 3
            && (uchar)chars[0] == 0xef && (uchar)chars[1] == 0xbb && (uchar)chars[2] == 0xbf)
    {
        // starts with a byte order mark
        chars += 3;
        len -= 3;
        headerdone = true;
    }

    QString result(need + len + 1, Qt::Uninitialized); // worst case
    auto* qch = (ushort*)result.unicode();
    uchar ch;
    int invalid = 0;

    for (int i = 0; i < len; ++i)
    {
        ch = chars[i];
        if (need)
        {
            if ((ch & 0xc0) == 0x80)
            {
                uc = (uc << 6) | (ch & 0x3f);
                --need;
                if (!need)
                {
                    // utf-8 bom composes into 0xfeff code point
                    bool nonCharacter;
                    if (!headerdone && uc == 0xfeff)
                    {
                        // don't do anything, just skip the BOM
                    }
                    else if (!(nonCharacter = isNonCharacter(uc)) && QChar::requiresSurrogates(uc) && uc <= LastValidCodePoint)
                    {
                        // surrogate pair
                        Q_ASSERT((qch - (ushort*)result.unicode()) + 2 < result.length());
                        *qch++ = QChar::highSurrogate(uc);
                        *qch++ = QChar::lowSurrogate(uc);
                    }
                    else if ((uc < min_uc) || isSurrogate(uc) || nonCharacter || uc > LastValidCodePoint)
                    {
                        // error: overlong sequence, UTF16 surrogate or non-character
                        *qch++ = replacement;
                        ++invalid;
                    }
                    else
                    {
                        *qch++ = uc;
                    }
                    headerdone = true;
                }
            }
            else
            {
                // error
                i = error;
                *qch++ = replacement;
                ++invalid;
                need = 0;
                headerdone = true;
            }
        }
        else
        {
            if (ch < 128)
            {
                *qch++ = ushort(ch);
                headerdone = true;
            }
            else if ((ch & 0xe0) == 0xc0)
            {
                uc = ch & 0x1f;
                need = 1;
                error = i;
                min_uc = 0x80;
                headerdone = true;
            }
            else if ((ch & 0xf0) == 0xe0)
            {
                uc = ch & 0x0f;
                need = 2;
                error = i;
                min_uc = 0x800;
            }
            else if ((ch & 0xf8) == 0xf0)
            {
                uc = ch & 0x07;
                need = 3;
                error = i;
                min_uc = 0x10000;
                headerdone = true;
            }
            else
            {
                // error
                *qch++ = replacement;
                ++invalid;
                headerdone = true;
            }
        }
    }
    if (!state && need > 0)
    {
        // unterminated UTF sequence
        for (int i = error; i < len; ++i)
        {
            *qch++ = replacement;
            ++invalid;
        }
    }
    result.truncate(qch - (ushort*)result.unicode());
    if (state)
    {
        state->invalidChars += invalid;
        state->remainingChars = need;
        if (headerdone)
        {
            state->flags |= QTextCodec::IgnoreHeader;
        }
        state->state_data[0] = need ? uc : 0;
        state->state_data[1] = need ? min_uc : 0;
    }
    return result;
}


QByteArray CustomUtf8Codec::convertFromUnicode(const QChar* uc, int len, ConverterState* state) const
{
    if (!uc)
    {
        return QByteArray();
    }
    if (len <= 0)
    {
        return QByteArray("");
    }

    uchar replacement = '?';
    int rlen = 3 * len;
    int surrogate_high = -1;
    if (state)
    {
        if (state->flags & QTextCodec::ConvertInvalidToNull)
        {
            replacement = 0;
        }
        if (!(state->flags & QTextCodec::IgnoreHeader))
        {
            rlen += 3;
        }
        if (state->remainingChars)
        {
            surrogate_high = state->state_data[0];
        }
    }

    if (m_b.size() < rlen)
    {
        m_b.resize(rlen);
    }
    auto* cursor = (uchar*)m_b.data();
    const QChar* ch = uc;
    int invalid = 0;
    if (state && !(state->flags & QTextCodec::IgnoreHeader))
    {
        *cursor++ = 0xef;
        *cursor++ = 0xbb;
        *cursor++ = 0xbf;
    }

    const QChar* end = ch + len;
    while (ch < end)
    {
        uint u = ch->unicode();
        if (surrogate_high >= 0)
        {
            if (ch->isLowSurrogate())
            {
                u = QChar::surrogateToUcs4(surrogate_high, u);
                surrogate_high = -1;
            }
            else
            {
                // high surrogate without low
                *cursor = replacement;
                ++ch;
                ++invalid;
                surrogate_high = -1;
                continue;
            }
        }
        else if (ch->isLowSurrogate())
        {
            // low surrogate without high
            *cursor = replacement;
            ++ch;
            ++invalid;
            continue;
        }
        else if (ch->isHighSurrogate())
        {
            surrogate_high = u;
            ++ch;
            continue;
        }

        if (u < 0x80)
        {
            *cursor++ = (uchar)u;
        }
        else
        {
            if (u < 0x0800)
            {
                *cursor++ = 0xc0 | ((uchar)(u >> 6));
            }
            else
            {
                // is it one of the Unicode non-characters?
                if (isNonCharacter(u))
                {
                    *cursor++ = replacement;
                    ++ch;
                    ++invalid;
                    continue;
                }

                if (QChar::requiresSurrogates(u))
                {
                    *cursor++ = 0xf0 | ((uchar)(u >> 18));
                    *cursor++ = 0x80 | (((uchar)(u >> 12)) & 0x3f);
                }
                else
                {
                    *cursor++ = 0xe0 | (((uchar)(u >> 12)) & 0x3f);
                }
                *cursor++ = 0x80 | (((uchar)(u >> 6)) & 0x3f);
            }
            *cursor++ = 0x80 | ((uchar)(u & 0x3f));
        }
        ++ch;
    }

    if (state)
    {
        state->invalidChars += invalid;
        state->flags |= QTextCodec::IgnoreHeader;
        state->remainingChars = 0;
        if (surrogate_high >= 0)
        {
            state->remainingChars = 1;
            state->state_data[0] = surrogate_high;
        }
    }

    len = cursor - (const uchar*)m_b.constData();
    if (len > 0 && len <= sizeof(m_cache) / sizeof(m_cache[0]))
    {
        QByteArray& mb = m_cache[len - 1];
        if (mb.isEmpty())
        {
            mb = QByteArray(m_b.constData(), len);
            return mb;
        }
        if (mb.isDetached())
        {
            Q_ASSERT(mb.length() == len);
            memcpy(mb.data(), m_b.constData(), len);
            Q_ASSERT(0 == mb.data()[len]);
            return mb;
        }
    }

    QByteArray mb(m_b);
    mb.resize(len);
    return mb;
}


QByteArray CustomUtf8Codec::name() const
{
    return "UTF-8";
}

int CustomUtf8Codec::mibEnum() const
{
    return 106;
}


static CustomUtf8Codec* instance;

CustomUtf8Codec* CustomUtf8Codec::Instance()
{
    if (0 == instance)
    {
        // Push the stock UTF-8 codec into cache
        VERIFY(QTextCodec::codecForMib(106));
        VERIFY(QTextCodec::codecForName("UTF-8"));

        instance = new CustomUtf8Codec();
    }

    return instance;
}

} // namespace utilities
