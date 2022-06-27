/*
 * Exerciser for Patience Deck engine class.
 * Copyright (C) 2021-2022 Tomi Leppänen
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
#include "checker.h"
#include "engine.h"
#include "engineinternals.h"

EngineHelper::EngineHelper(QObject *parent)
    : QObject(parent)
    , m_checker(nullptr)
    , m_goal(TestCurrentSeed)
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
        {{"f", "find-winnable"}, "Find new seeds until a winnable game is found"},
        {{"F", "find-any"}, "Find new seeds until a finishable game is found"},
    });
    parser.process(QCoreApplication::arguments());

    if (parser.isSet("find-any")) {
        m_goal = FindFinishableGame;
        emit goalChanged();
    } else if (parser.isSet("find-winnable")) {
        m_goal = FindWinnableGame;
        emit goalChanged();
    }

    if (parser.isSet("seed")) {
        bool ok;
        EngineInternals::instance()->m_seed = parser.value("seed").toULongLong(&ok);
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

EngineChecker *EngineHelper::checker() const
{
    return m_checker;
}

void EngineHelper::setChecker(EngineChecker *checker)
{
    if (m_checker != checker) {
        m_checker = checker;
        emit checkerChanged();
    }
}

EngineHelper::Goal EngineHelper::goal() const
{
    return m_goal;
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
    return static_cast<quint32>(EngineInternals::instance()->m_seed);
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
                if (target != -1) {
                    m_checker->removeCards(slot, cards);
                    engine->drop(-1, slot, target, cards);
                }
            }
        }
    }
}

void EngineHelper::click(const QVariantMap &clicked)
{
    auto engine = Engine::instance();
    if (isCard(clicked)) {
        auto card = toCard(clicked);
        int slot = findSlot(card);
        if (slot != -1) {
            qDebug() << "Clicking" << card << "at slot" << slot;
            engine->click(-1, slot);
        }
    } else {
        int slot = findSlotByType(static_cast<Slots>(clicked.value("type").toInt()),
                                  clicked.value("empty").toBool());
        if (slot == -1) {
            // Game probably doesn't define slot types, assume slot 0
            slot = 0;
        }
        qDebug() << "Clicking slot" << slot;
        engine->click(-1, slot);
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
    auto engine = EngineInternals::instance();
    for (auto it = engine->m_cardSlots.constBegin(); it != engine->m_cardSlots.constEnd(); it++) {
        for (const auto &card : *it) {
            if (needle.equalValue(card))
                return it - engine->m_cardSlots.constBegin();
        }
    }
    return -1;
}

int EngineHelper::findSlotByType(Slots type, bool emptyRequired) const
{
    auto engine = EngineInternals::instance();
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
    auto engine = EngineInternals::instance();
    CardList cards = engine->m_cardSlots[slot];
    int i = cards.indexOf(first);
    return cards.mid(i);
}
