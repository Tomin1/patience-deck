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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include "helper.h"
#include "engine.h"
#include "engine_p.h"

EngineHelper::EngineHelper()
    : QObject(nullptr)
{
    auto engine = Engine::instance();
    connect(engine, &Engine::clearData, this, &EngineHelper::handleClearData);
    connect(engine, &Engine::newSlot, this, &EngineHelper::handleNewSlot);

    QDir directory("games");
    engine->initWithDirectory(directory.absolutePath());
}

EngineHelper::~EngineHelper()
{
}

bool EngineHelper::parseArgs()
{

    QCommandLineParser parser;
    parser.setApplicationDescription("Tool to test Patience Deck engine");
    parser.addOptions({
        {{"g", "game"}, "Game file name to load", "filename"},
        {{"s", "seed"}, "Seed to use", "seed"},
    });
    parser.process(QCoreApplication::arguments());

    if (parser.isSet("seed")) {
        bool ok;
        EnginePrivate::instance()->m_seed = parser.value("seed").toULongLong(&ok);
        if (!ok)
            return false;
    }

    Engine::instance()->loadGame(parser.isSet("game") ? parser.value("game") : "klondike.scm",
                                 parser.isSet("seed"));
    return true;
}

Engine *EngineHelper::engine() const
{
    return Engine::instance();
}

void EngineHelper::handleClearData()
{
    m_slotTypes.clear();
}

void EngineHelper::handleNewSlot(int id, const CardList &cards, int type, double x, double y,
                   int expansionDepth, bool expandedDown, bool expandedRight)
{
    Q_UNUSED(cards)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(expansionDepth)
    Q_UNUSED(expandedDown)
    Q_UNUSED(expandedRight)

    m_slotTypes.insert(id, static_cast<Slots>(type));
}

quint32 EngineHelper::getSeed() const
{
    return static_cast<quint32>(EnginePrivate::instance()->m_seed);
}

void EngineHelper::move(const QVariantMap &from, const QVariantMap &to)
{
    auto engine = Engine::instance();
    if (isCard(from)) {
        auto card = toCard(from);
        int slot = findSlot(card);
        if (slot != -1) {
            auto cards = getCards(slot, card);
            if (engine->drag(-1, slot, cards)) {
                int target;
                if (isCard(to)) {
                    auto card2 = toCard(to);
                    target = findSlot(card2);
                    qDebug() << "Moving" << card << "from slot" << slot
                             << "onto" << card2 << "in slot" << target;
                } else {
                    target = findSlotByType(static_cast<Slots>(to.value("type").toInt()),
                                            to.value("empty").toBool());
                    qDebug() << "Moving" << card << "from slot" << slot
                             << "to slot" << target;
                }
                if (target != -1)
                    engine->drop(-1, slot, target, cards);
            }
        }
    }
}

bool EngineHelper::isCard(const QVariantMap &map)
{
    return map.contains("rank") && map.contains("suit");
}

CardData EngineHelper::toCard(const QVariantMap &map)
{
    CardData data;
    data.suit = static_cast<Suit>(map.value("suit").toInt());
    data.rank = static_cast<Rank>(map.value("rank").toInt());
    data.show = true;
    return data;
}

int EngineHelper::findSlot(const CardData &needle)
{
    auto engine = EnginePrivate::instance();
    for (auto it = engine->m_cardSlots.constBegin(); it != engine->m_cardSlots.constEnd(); it++) {
        for (const auto card: it.value()) {
            if (needle.equalValue(card))
                return it.key();
        }
    }
    return -1;
}

int EngineHelper::findSlotByType(Slots type, bool emptyRequired)
{
    auto engine = EnginePrivate::instance();
    for (auto it = m_slotTypes.constBegin(); it != m_slotTypes.constEnd(); it++) {
        if (it.value() == type) {
            if (!emptyRequired || engine->m_cardSlots[it.key()].isEmpty())
                return it.key();
        }
    }
    return -1;
}

CardList EngineHelper::getCards(int slot, const CardData &first)
{
    auto engine = EnginePrivate::instance();
    CardList cards = engine->m_cardSlots[slot];
    int i = cards.indexOf(first);
    return cards.mid(i);
}
