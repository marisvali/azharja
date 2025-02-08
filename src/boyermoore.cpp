#include "boyermoore.h"

constexpr int64_t ALPHABET_LEN = 256;

void CreateDelta1(QVector<int64_t>& delta1, const QString& pat)
{
    int64_t patlen = pat.length();
    for (int64_t i = 0; i < ALPHABET_LEN; i++)
        delta1[i] = patlen;
    for (int64_t i = 0; i < patlen; i++)
        delta1[pat[i].unicode()] = patlen - 1 - i;
}

bool IsPrefix(const QString& word, int64_t wordlen, int64_t pos)
{
    int64_t suffixlen = wordlen - pos;
    for (int64_t i = 0; i < suffixlen; i++)
        if (word[i] != word[pos + i])
            return false;
    return true;
}

size_t SuffixLength(const QString& word, int64_t wordlen, int64_t pos)
{
    int64_t i = 0;
    while ((pos >= i) && (word[pos - i] == word[wordlen - 1 - i]))
        i++;
    return i;
}

void CreateDelta2(QVector<int64_t>& delta2, const QString& pat)
{
    int64_t patlen = pat.length();
    int64_t last_prefix_index = patlen - 1;
    for (int64_t p = patlen - 1; p >= 0; p--)
    {
        if (IsPrefix(pat, patlen, p + 1))
            last_prefix_index = p + 1;
        delta2[p] = last_prefix_index + (patlen - 1 - p);
    }
    for (int64_t p = 0; p < patlen - 1; p++)
    {
        int64_t slen = SuffixLength(pat, patlen, p);
        // Add boundary checks.
        if (p >= slen && p - slen < patlen && patlen - 1 - slen < patlen)
            if (pat[p - slen] != pat[patlen - 1 - slen])
                delta2[patlen - 1 - slen] = patlen - 1 - p + slen;
    }
}

QVector<int64_t> SearchStringPattern(const QString& text, const QString& pattern)
{
    QVector<int64_t> occurrences;
    QVector<int64_t> delta1(ALPHABET_LEN);
    QVector<int64_t> delta2(pattern.length());
    CreateDelta1(delta1, pattern);
    CreateDelta2(delta2, pattern);

    if (pattern.isEmpty())
        return occurrences;

    int64_t i = pattern.length() - 1;
    while (i < text.length())
    {
        int64_t j = pattern.length() - 1;
        while (j >= 0 && text[i] == pattern[j])
        {
            // Prevent negative index.
            if (i == 0)
                break;
            --i;
            --j;
        }
        if (j < 0)
        {
            occurrences.push_back(i + 1);
            i += delta2[0];
        }
        else
            i += qMax(delta1[text[i].unicode()], delta2[j]);
    }
    return occurrences;
}
