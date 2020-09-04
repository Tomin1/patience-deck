#ifndef CARD_H
#define CARD_H

#include <QObject>
#include "enginedata.h"

class Board;
class Card : public QObject
{
    Q_OBJECT

public:
    Card(const CardData &card, QObject *parent = nullptr);

    Suit suit() const;
    Rank rank() const;
    bool show() const;
    bool isBlack() const;
    const QString elementName() const;

    friend inline bool operator==(const Card &a, const Card &b);

private:
    CardData m_data;
};

inline bool operator==(const Card &a, const Card &b)
{
    return a.m_data == b.m_data;
}

#endif // CARD_H
