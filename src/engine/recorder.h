/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2022-2025 Tomi Leppänen
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
#include <QScopedPointer>
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
    void advanceRngState(int steps = 1);
    void invalidateState();

    void storeOldState();
    void restoreOldState();
    void dropOldState();

signals:
    void replayCompleted(CompletionStatus status);
    void replayingGame(const QString &gameFile, bool hasSeed, quint32 seed, qint64 time);
    void oldStateStored(bool stored);

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
        RngState,
    };

    class Record {
    public:
        MoveType type = None;
        int startSlot = -1;
        int endSlot = -1;
        int cards = 0;

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
        static Record rngState(int state) {
            auto record = Record(RngState);
            record.cards = state;
            return record;
        }

        static Record fromString(const QString &record);
        QString toString() const;
    };

    struct OldState {
        QVector<Record> records;
        quint32 seed;
        qint64 time;

        OldState(QVector<Record> records, quint32 seed, qint64 time)
            : records(records)
            , seed(seed)
            , time(time) {}

        bool restoring() { return time == -1; }
        void setRestoring() { time = -1; }
    };

    bool load();
    void replaySingle();
    void clear();
    void fail();

    const Record &current() const;
    Engine *engine() const;

    uint m_replaying = 0;
    QVector<Record> m_records;
    QVector<Record> m_abandoned;
#ifndef ENGINE_EXERCISER
    MGConfItem m_stateConf;
#endif // ENGINE_EXERCISER
    QString m_gameFile;
    bool m_hasSeed = false;
    quint32 m_seed = 0;
    quint32 m_rngState = 0;
    quint32 m_lastSavedRngState = 0;
    int m_moves = 0;
    QElapsedTimer m_elapsed;
    QScopedPointer<OldState> m_oldState;
};

#endif // RECORDER_H
