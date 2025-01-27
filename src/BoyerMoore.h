#ifndef BOYERMOORE_H
#define BOYERMOORE_H

#include <QString>
#include <QVector>
#include <QtGlobal>


QVector<size_t> boyer_moore_all(const QString& text, const QString& pattern);

#endif
