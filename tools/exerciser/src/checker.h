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

#include <list>
#include <QHash>
#include <QObject>
#include <QSharedPointer>
#include <QVariant>
#include "enginedata.h"
#include "queue.h"

class Engine;
class EngineChecker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool failed READ failed NOTIFY failedChanged)

public:
    explicit EngineChecker(QObject *parent = nullptr);

    bool failed() const;

    Q_INVOKABLE void dump() const;

    void removeCards(int source, const CardList &cards);

signals:
    void failedChanged();
    void queueFinished();

private slots:
    void handleClearData();
    void handleNewSlot(int id, const CardList &cards, int type, double x, double y,
                       int expansionDepth, bool expandedDown, bool expandedRight);
    void handleGameStarted();
    void handleAction(Engine::ActionTypeFlags action, int slot, int index, const CardData &data);

private:
    struct Error {
        enum Reason {
            NoReason,
            WrongCard,
            DoubleRequeue,
            BadIndex,
            BadSlot,
            MissingCards,
        } reason;
        Action action;
        CardData data;

        Error(Reason reason, const Action &action, const CardData &data);
    };

    friend QDebug operator<<(QDebug debug, const Error &error);

    void handleImmediately(Engine::ActionType action, int slotId, int index, const CardData &data);
    bool handleQueued(const Action &action);
    void handleMoveEnded();
    void fail(Error::Reason reason, const Action &action, const CardData &data);

    int m_move;
    QHash<int, CardList> m_slots;
    Queue<QSharedPointer<CardData>> m_queue;
    std::list<Error> m_errors;
};
