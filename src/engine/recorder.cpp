/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2022-2023 Tomi Leppänen
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

#include <QRegularExpression>
#include <QTimer>
#include "constants.h"
#include "engine.h"
#include "gameoptionmodel.h"
#include "logging.h"
#ifndef ENGINE_EXERCISER
#include "patience.h"
#endif // ENGINE_EXERCISER
#include "recorder.h"

namespace {
const auto DataVersion = QStringLiteral("0");
const auto MovesTemplate = QStringLiteral("%1:%2");
const int MovesBetweenSaves = 10;
const qint64 MoveTimeout = 30 * 1000;
const qint64 MinimumSaveInterval = 1000;
const quint32 ID = -1;

QString encode(const QString &text)
{
    return QString::fromUtf8(qCompress(text.toUtf8()).toBase64());
}

QString decode(const QString &text)
{
    return QString::fromUtf8(qUncompress(QByteArray::fromBase64(text.toUtf8())));
}

struct SavedState {
    bool valid;
    QString gameFile;
    quint32 seed;
    bool hasSeed;
    bool seedOk;
    qint64 time;
    QString moves;

    SavedState(const QString &gameFile = QString(),
               quint32 seed = 0,
               bool hasSeed = false,
               qint64 time = 0,
               QString moves = QString())
        : valid(false)
        , gameFile(gameFile)
        , seed(seed)
        , hasSeed(hasSeed)
        , seedOk(true)
        , time(time)
        , moves(moves) {}

    QString toString(bool encoded = true) const
    {
        QStringList parts;
        parts << gameFile;
        if (hasSeed)
            parts << QString::number(seed);
        if (!moves.isEmpty()) {
            auto data = MovesTemplate.arg(time).arg(moves);
            parts << DataVersion << (encoded ? encode(data) : data);
        }
        return parts.join(';');
    }

    static SavedState fromString(const QString &state)
    {
        SavedState saved;
        auto parts = state.split(';');
        saved.valid = !parts.at(0).isEmpty();
        if (saved.valid) {
            saved.gameFile = parts.at(0);
            if (parts.count() >= 2) {
                saved.hasSeed = true;
                saved.seed = parts.at(1).toULongLong(&saved.seedOk);
                if (saved.seedOk && parts.count() >= 4 && parts.at(2) == DataVersion) {
                    QString moves = decode(parts.at(3));
                    int sep = moves.indexOf(':');
                    bool ok = false;
                    saved.time = moves.left(sep).toLongLong(&ok);
                    if (ok)
                        saved.moves = moves.mid(sep + 1);
                }
            }
        }
        return saved;
    }

#ifndef ENGINE_EXERCISER
    static SavedState fromConfItem(const MGConfItem &stateConf)
    {
        auto state = stateConf.value();
        if (state.isValid())
            return fromString(state.toString());
        return SavedState();
    }
#endif // ENGINE_EXERCISER
};
} // namespace

const QString StateConf = QStringLiteral("/state");

Recorder::Recorder(Engine *engine)
    : QObject(engine)
    , m_replaying(0)
#ifndef ENGINE_EXERCISER
    , m_stateConf(Constants::ConfPath + StateConf)
#endif // ENGINE_EXERCISER
    , m_hasSeed(false)
    , m_seed(0)
    , m_moves(0)
{
    connect(engine, &Engine::gameLoaded, this, &Recorder::handleGameLoaded, Qt::DirectConnection);
    connect(engine, &Engine::gameStarted, this, &Recorder::handleGameStarted, Qt::DirectConnection);
    connect(engine, &Engine::moveEnded, this, &Recorder::handleMoveEnded, Qt::QueuedConnection);
    connect(engine, &Engine::gameOver, this, &Recorder::handleGameOver, Qt::QueuedConnection);

#ifndef ENGINE_EXERCISER
    connect(&m_stateConf, &MGConfItem::valueChanged, this, [&]() {
        qCDebug(lcRecorder) << (m_stateConf.value().isValid()
                              ? "Saved engine state" : "Reset saved state");
    });
#endif // ENGINE_EXERCISER
}

Recorder::~Recorder()
{
    save();
}

void Recorder::startReplay()
{
    qCDebug(lcRecorder) << "Starting replay";
    if (!load())
        emit replayCompleted(Failed);
}

void Recorder::replayMove()
{
    qCDebug(lcRecorder) << "Replaying first move";
    m_replaying = 1;
    replaySingle();
}

bool Recorder::replaying() const
{
    return m_replaying;
}

bool Recorder::load()
{
#ifndef ENGINE_EXERCISER
    auto state = SavedState::fromConfItem(m_stateConf);
    qCDebug(lcRecorder) << "Loaded state" << state.toString(false);
    if (state.valid) {
        if (state.seedOk) {
            if (!state.moves.isEmpty()) {
                for (const QString &record : state.moves.split(','))
                    m_records.append(Record::fromString(record));
            }
            emit replayingGame(state.gameFile, state.hasSeed, state.seed, state.time);
            return true;
        }
    } else {
        qCInfo(lcRecorder) << "Engine state was not stored, not restored";
    }
#endif // ENGINE_EXERCISER
    return false;
}

void Recorder::replaySingle()
{
    if (!m_replaying) {
        qCCritical(lcRecorder) << "Tried to replay a move while not replaying";
        return;
    }

    if (m_replaying > (uint)m_records.count()) {
        emit replayCompleted(Success);
        m_replaying = 0;
        return;
    }

    const Record &record = current();
    switch (record.type) {
    case None:
        qCCritical(lcRecorder) << "Invalid Record";
        fail();
        return;
    case Deal:
        engine()->dealCard();
        qCInfo(lcRecorder) << "Replayed dealing card";
        break;
    case Move:
        {
            CardList cards = engine()->cards(record.startSlot, record.cards);
            if (cards.count() != record.cards)
                qCCritical(lcRecorder) << "Got unexpected number of cards" << cards.count()
                                       << "instead of" << record.cards;
            qCDebug(lcRecorder) << "Replaying move" << record.startSlot << record.endSlot << record.cards;
            if (!engine()->drag(ID, record.startSlot, cards)) {
                qCWarning(lcRecorder) << "Failed to drag while replaying";
                fail();
                return;
            } if (!engine()->checkDrop(ID, record.startSlot, record.endSlot, cards)) {
                qCWarning(lcRecorder) << "Failed check for dropping while replaying";
                engine()->cancelDrag(ID, record.startSlot, cards);
                fail();
                return;
            } else if (!engine()->drop(ID, record.startSlot, record.endSlot, cards)) {
                qCWarning(lcRecorder) << "Failed to drop while replaying";
                fail();
                return;
            } else {
                qCInfo(lcRecorder) << "Replayed move";
            }
            break;
        }
    case Click:
        qCDebug(lcRecorder) << "Replaying click" << record.startSlot;
        if (!engine()->click(ID, record.startSlot)) {
            qCWarning(lcRecorder) << "Failed to click while replaying";
            fail();
            return;
        } else {
            qCInfo(lcRecorder) << "Replayed click";
        }
        break;
    case DoubleClick:
        qCDebug(lcRecorder) << "Replaying double click" << record.startSlot;
        if (!engine()->doubleClick(ID, record.startSlot)) {
            qCWarning(lcRecorder) << "Failed to click while replaying";
            fail();
            return;
        } else {
            qCInfo(lcRecorder) << "Replayed double click";
        }
        break;
    }
    m_replaying++;
}

const Recorder::Record &Recorder::current() const
{
    return m_records.at(m_replaying - 1);
}

Engine *Recorder::engine() const
{
    return qobject_cast<Engine *>(parent());
}

void Recorder::clear()
{
    m_records.clear();
    m_abandoned.clear();
    m_moves++; // Count clear() as a move to force save()
}

void Recorder::save()
{
    if (!m_elapsed.isValid() || m_elapsed.hasExpired(MinimumSaveInterval) || m_moves) {
        QStringList records;
        for (const Record &record : m_records)
            records << record.toString();
#ifndef ENGINE_EXERCISER
        // Take elapsed time from another thread :E
        // This is fine. Trust me, I'm an engineer. ;)
        // (Patience instance is not going anywhere so we get away with this.)
        m_stateConf.set(SavedState(m_gameFile, m_seed, m_hasSeed, Patience::instance()->elapsedTimeMs(),
                                   records.join(',')).toString());
#endif // ENGINE_EXERCISER
        m_moves = 0;
        m_elapsed.start();
    }
}

void Recorder::fail()
{
    qCWarning(lcRecorder) << "Failed to restore game, abandoning state and resetting engine";
    clear();
    m_replaying = 0;
    emit replayCompleted(NeedsRestart);
    save();
}

void Recorder::setSeed(quint32 seed)
{
    qCDebug(lcRecorder) << "Storing seed";
    m_seed = seed;
    m_hasSeed = true;
}

void Recorder::invalidateState()
{
    qCDebug(lcRecorder) << "Invalidating state";
    m_hasSeed = false;
}

void Recorder::storeOldState()
{
    // Store only if there is a new state to store
    if ((m_oldState.isNull() || !m_oldState->restoring()) && !m_replaying && m_hasSeed) {
        m_oldState.reset(new OldState(m_records, m_seed, Patience::instance()->elapsedTimeMs()));
        qCDebug(lcRecorder) << "Stored old state";
        emit oldStateStored(true);
    }
}

void Recorder::restoreOldState()
{
    // Restore only if there was a state to restore
    if (!m_oldState.isNull()) {
        m_records = m_oldState->records;
        m_abandoned.clear();
        m_hasSeed = true;
        m_seed = m_oldState->seed;
        m_moves = 0;
        qint64 time = m_oldState->time;
        m_oldState->setRestoring();
        qCDebug(lcRecorder) << "Restored old state";
        emit oldStateStored(false);
        emit replayingGame(m_gameFile, true, m_oldState->seed, time);
        m_oldState.reset();
    }
}

void Recorder::dropOldState()
{
    if (!m_oldState.isNull()) {
        m_oldState.reset();
        qCDebug(lcRecorder) << "Dropped old state";
        emit oldStateStored(false);
    }
}

void Recorder::handleGameLoaded(const QString &gameFile)
{
    m_gameFile = gameFile;
}

void Recorder::handleGameStarted()
{
    if (!m_replaying) {
        qCDebug(lcRecorder) << "Game started, resetting recorded state";
        clear();
        save();
    }
}

void Recorder::handleMoveEnded()
{
    if (!m_replaying) {
        if (++m_moves >= MovesBetweenSaves || m_elapsed.hasExpired(MoveTimeout))
            save();
    } else {
        QTimer::singleShot(0, this, [this] {
            replaySingle();
        });
    }
}

void Recorder::handleGameOver()
{
    if (!m_replaying)
        save();
}

void Recorder::handleEngineFailure()
{
    if (m_replaying) {
        qCWarning(lcRecorder) << "Engine failure while replaying moves";
        fail();
    }
}

void Recorder::undo()
{
    if (!m_replaying && !m_records.empty())
        m_abandoned.append(m_records.takeLast());
}

void Recorder::redo()
{
    if (!m_replaying && !m_abandoned.empty())
        m_records.append(m_abandoned.takeLast());
}

void Recorder::recordDeal()
{
    if (!m_replaying) {
        m_records.append(Record::deal());
        m_abandoned.clear();
    }
}

void Recorder::recordDrop(int startSlotId, int endSlotId, int cards)
{
    if (!m_replaying) {
        m_records.append(Record::move(startSlotId, endSlotId, cards));
        m_abandoned.clear();
    }
}

void Recorder::recordClick(int slotId)
{
    if (!m_replaying) {
        m_records.append(Record::click(slotId));
        m_abandoned.clear();
    }
}

void Recorder::recordDoubleClick(int slotId)
{
    if (!m_replaying) {
        m_records.append(Record::doubleClick(slotId));
        m_abandoned.clear();
    }
}

void Recorder::addArguments(QCommandLineParser *parser)
{
    parser->addOptions({
        {{"g", "game"}, "Set game file to load", "filename"},
        {{"s", "seed"}, "Set initial seed to load", "integer"},
        {{"m", "moves"}, "Recorded moves to make", "moves"},
        {{"t", "time"}, "Recorded time to set", "time"},
        {{"o", "options"}, "Indices of options to set before loading game", "options"}
    });
}

void Recorder::setArguments(QCommandLineParser *parser)
{
#ifdef ENGINE_EXERCISER
    Q_UNUSED(parser)
#else
    MGConfItem stateConf(Constants::ConfPath + StateConf);
    SavedState state = SavedState::fromConfItem(stateConf);
    if (parser->isSet("game"))
        state.gameFile = parser->value("game");
    if (parser->isSet("seed")) {
        state.seed = parser->value("seed").toULongLong(&state.seedOk);
        state.hasSeed = true;
    }
    if (parser->isSet("moves")) {
        auto moves = parser->value("moves");
        if (!moves.contains(':') && !moves.contains(',') && moves != "D") {
            moves = decode(moves.toUtf8());
            if (moves.at(0).isDigit())
                moves = moves.mid(moves.indexOf(':') + 1);
        }
        state.moves = moves;
    }
    else if (parser->isSet("game") || parser->isSet("seed"))
        // invalidate moves
        state.moves.clear();
    if (parser->isSet("time"))
        state.time = parser->value("time").toLongLong();
    if (parser->isSet("game") || parser->isSet("seed") || parser->isSet("moves")) {
        stateConf.set(state.toString());
        stateConf.sync();
    }
    if (!state.gameFile.isEmpty() && parser->isSet("options")) {
        GameOptionList options;
        QRegularExpression sep("[;, ]");
        for (const QString &value : parser->value("options").split(sep, QString::SkipEmptyParts)) {
            bool ok = true;
            int index = value.toInt(&ok);
            if (ok) {
                GameOption option;
                option.index = index;
                option.set = true;
                options << option;
            }
        }
        GameOptionModel::saveOptions(state.gameFile, options);
    }
#endif // ENGINE_EXERCISER
}

Recorder::Record Recorder::Record::fromString(const QString &record)
{
    QStringList parts = record.split(':');
    switch (record[0].toLatin1()) {
    case 'D':
        return deal();
    case 'M':
        if (parts.length() < 4) {
            qCCritical(lcRecorder) << "Invalid stored move";
            return Record();
        }
        return move(parts.at(1).toInt(), parts.at(2).toInt(), parts.at(3).toInt());
    case 'C':
        if (parts.length() < 2) {
            qCCritical(lcRecorder) << "Invalid stored click";
            return Record();
        }
        return click(parts.at(1).toInt());
    case 'L':
        if (parts.length() < 2) {
            qCCritical(lcRecorder) << "Invalid stored double click";
            return Record();
        }
        return doubleClick(parts.at(1).toInt());
    default:
        qCCritical(lcRecorder) << "Invalid stored record";
        return Record();
    }
}

QString Recorder::Record::toString() const
{
    QStringList record;
    switch (type) {
    case Deal:
        record << "D";
        break;
    case Move:
        record << "M";
        record << QString::number(startSlot);
        record << QString::number(endSlot);
        record << QString::number(cards);
        break;
    case Click:
        record << "C";
        record << QString::number(startSlot);
        break;
    case DoubleClick:
        record << "L";
        record << QString::number(startSlot);
        break;
    case None:
        qCCritical(lcRecorder) << "Invalid record";
        break;
    }
    return record.join(':');
}
