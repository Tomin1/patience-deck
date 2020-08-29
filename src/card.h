#ifndef CARD_H
#define CARD_H

#include <QObject>
#include "enginedata.h"

class Card : public QObject
{
    Q_OBJECT

public:

    Card(Suit suit, Rank rank, bool faceDown, QObject *parent = nullptr);

    Suit suit() const { return m_suit; };
    Rank rank() const { return m_rank; };
    bool faceDown() const { return m_faceDown; };

    friend inline bool operator==(const Card &a, const Card &b);

private:
    Suit m_suit;
    Rank m_rank;
    bool m_faceDown;
};

inline bool operator==(const Card &a, const Card &b)
{
    return a.m_faceDown == b.m_faceDown && a.m_suit == b.m_suit && a.m_rank == b.m_rank;
}

#endif // CARD_H
