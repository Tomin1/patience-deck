#ifndef SLOT_H
#define SLOT_H

#include <QObject>
#include <libguile.h>

class Card;
class Aisleriot;
class AisleriotPrivate;
class Slot : public QObject
{
    Q_OBJECT

public:
    enum SlotType {
        UnknownSlot,
        ChooserSlot,
        FoundationSlot,
        ReserveSlot,
        StockSlot,
        TableauSlot,
        WasteSlot
    };

    Slot(int id, SlotType type, double x, double y,
         int expansionDepth, bool expandedDown, bool expandedRight,
         QObject *parent = nullptr);

    void setCards(SCM cards);

    SCM toSCM() const;

private:
    int m_id;
    SlotType m_type;
    QList<Card*> m_cards;
    bool m_exposed;
    double m_x;
    double m_y;
    double m_expansion;
    int m_expansionDepth;
    bool m_expandedDown;
    bool m_expandedRight;
    bool m_needsUpdate;
};

#endif // SLOT_H
