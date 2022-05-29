/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2022 Tomi Leppänen
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

#ifndef RECORDER_H
#define RECORDER_H

#ifndef ENGINE_EXERCISER
#include <MGConfItem>
#endif // ENGINE_EXERCISER
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QObject>
#include <QVector>
#include "enginedata.h"

class Engine;
class Recorder : public QObject
{
    Q_OBJECT

public:
    enum CompletionStatus {
        Failed,
        Success,
        NeedsRestart,
    };

    static void addArguments(QCommandLineParser *parser);
    static void setArguments(QCommandLineParser *parser);

    Recorder(Engine *engine);
    ~Recorder();

    void startReplay();
    void replayMove();
    bool replaying() const;

    void save();
    void undo();
    void redo();

    void recordDeal();
    void recordDrop(int startSlotId, int endSlotId, int cards);
    void recordClick(int slotId);
    void recordDoubleClick(int slotId);

    void setSeed(quint32 seed);

signals:
    void replayCompleted(CompletionStatus status);
    void replayingGame(const QString &gameFile, bool hasSeed, quint32 seed, qint64 time);

public slots:
    void handleGameLoaded(const QString &gameFile);
    void handleGameStarted();
    void handleMoveEnded();
    void handleGameOver();
    void handleEngineFailure();

private:
    enum MoveType {
        None,
        Deal,
        Move,
        Click,
        DoubleClick,
    };

    class Record {
    public:
        MoveType type;
        int startSlot;
        int endSlot;
        int cards;

        Record(MoveType type = None, int startSlot = -1, int endSlot = -1)
            : type(type)
            , startSlot(startSlot)
            , endSlot(endSlot) {}

        static Record deal() { return Record(Deal); }
        static Record move(int startSlot, int endSlot, int cards)
        {
            auto record = Record(Move, startSlot, endSlot);
            record.cards = cards;
            return record;
        }
        static Record click(int slot) { return Record(Click, slot); }
        static Record doubleClick(int slot) { return Record(DoubleClick, slot); }

        static Record fromString(const QString &record);
        QString toString() const;
    };

    bool load();
    void replaySingle();
    void clear();
    void fail();

    const Record &current() const;
    Engine *engine() const;

    uint m_replaying;
    QVector<Record> m_records;
    QVector<Record> m_abandoned;
#ifndef ENGINE_EXERCISER
    MGConfItem m_stateConf;
#endif // ENGINE_EXERCISER
    QString m_gameFile;
    bool m_hasSeed;
    quint32 m_seed;
    int m_moves;
    QElapsedTimer m_elapsed;
};

#endif // RECORDER_H
