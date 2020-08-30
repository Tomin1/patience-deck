#ifndef CARD_H
#define CARD_H

#include <QObject>
#include "enginedata.h"

class Board;
class Card : public QObject
{
    Q_OBJECT

public:
    Card(Suit suit, Rank rank, bool show, QObject *parent = nullptr);

    Suit suit() const;
    Rank rank() const;
    bool show() const;
    bool isBlack() const;
    const QString elementName() const;

    friend inline bool operator==(const Card &a, const Card &b);

private:
    Suit m_suit;
    Rank m_rank;
    bool m_show;
};

inline bool operator==(const Card &a, const Card &b)
{
    return a.m_show == b.m_show && a.m_suit == b.m_suit && a.m_rank == b.m_rank;
}

#endif // CARD_H
