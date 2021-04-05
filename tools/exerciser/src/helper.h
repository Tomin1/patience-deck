/*
 * Exerciser for Patience Deck engine class.
 * Copyright (C) 2021 Tomi Leppänen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QHash>
#include <QObject>
#include <QVariant>
#include "enginedata.h"

class Engine;
class EngineHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Engine *engine READ engine CONSTANT)

public:
    EngineHelper();
    ~EngineHelper();

    Engine *engine() const;
    Q_INVOKABLE quint32 getSeed() const;
    Q_INVOKABLE void move(const QVariantMap &from, const QVariantMap &to);

    enum Slots : int {
        Unknown,
        Chooser,
        Foundation,
        Reserve,
        Stock,
        Tableau,
        Waste
    };
    Q_ENUM(Slots)

private slots:
    void handleClearData();
    void handleNewSlot(int id, const CardList &cards, int type, double x, double y,
                       int expansionDepth, bool expandedDown, bool expandedRight);

private:
    bool isCard(const QVariantMap &map);
    CardData toCard(const QVariantMap &map);
    int findSlot(const CardData &needle);
    int findSlotByType(Slots type, bool emptyRequired);
    CardList getCards(int slot, const CardData &first);

    QHash<int, Slots> m_slotTypes;
};
