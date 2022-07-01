/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2022 Tomi Leppänen
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

#include <memory>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QJSEngine>
#include <QQmlEngine>
#include <QTimer>
#include "constants.h"
#include "gamelist.h"
#include "logging.h"
#include "patience.h"
#include "table.h"

const QString Constants::ConfPath = QStringLiteral("/site/tomin/apps/PatienceDeck");
const QString HistoryConf = QStringLiteral("/history");
const int TestModeDelay = 200;

Patience* Patience::s_game = nullptr;

Patience::TestModeFlags Patience::s_testMode = TestModeDisabled;

Patience* Patience::instance()
{
    if (!s_game)
        s_game = new Patience();
    return s_game;
}

QObject* Patience::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return instance();
}

Patience::Patience(QObject *parent)
    : QObject(parent)
    , m_engineFailed(false)
    , m_canUndo(false)
    , m_canRedo(false)
    , m_canDeal(false)
    , m_showDeal(false)
    , m_score(0)
    , m_showScore(false)
    , m_state(UninitializedState)
    , m_historyConf(Constants::ConfPath + HistoryConf)
{
    if (s_testMode & TestModeEnabled)
        qCInfo(lcTestMode) << "Test mode enabled.";
    auto engine = Engine::instance();
    engine->moveToThread(&m_engineThread);
    connect(&m_engineThread, &QThread::started, engine, &Engine::init);
    connect(&m_engineThread, &QThread::finished, engine, &Engine::deleteLater);
    connect(engine, &Engine::gameLoaded, this, &Patience::handleGameLoaded);
    connect(engine, &Engine::gameStarted, this, &Patience::handleGameStarted);
    connect(engine, &Engine::gameContinued, this, &Patience::handleGameContinued);
    connect(engine, &Engine::gameOver, this, &Patience::handleGameOver);
    connect(engine, &Engine::canUndo, this, &Patience::handleCanUndoChanged);
    connect(engine, &Engine::canRedo, this, &Patience::handleCanRedoChanged);
    connect(engine, &Engine::canDeal, this, &Patience::handleCanDealChanged);
    connect(engine, &Engine::score, this, &Patience::handleScoreChanged);
    connect(engine, &Engine::message, this, &Patience::handleMessageChanged);
    connect(engine, &Engine::hint, this, &Patience::hint);
    connect(engine, &Engine::showScore, this, &Patience::handleShowScore);
    connect(engine, &Engine::showDeal, this, &Patience::handleShowDeal);
    connect(engine, &Engine::moveEnded, this, &Patience::cardMoved);
    connect(engine, &Engine::restoreStarted, this, &Patience::handleRestoreStarted);
    connect(engine, &Engine::restoreCompleted, this, &Patience::handleRestoreCompleted);
    connect(engine, &Engine::engineFailure, this, &Patience::catchFailure);
    connect(this, &Patience::cardMoved, this, &Patience::handleCardMoved);
    connect(this, &Patience::doStart, engine, &Engine::start);
    connect(this, &Patience::doRestart, engine, &Engine::restart);
    connect(this, &Patience::doLoad, engine, &Engine::load);
    connect(this, &Patience::doUndoMove, engine, &Engine::undoMove);
    connect(this, &Patience::doRedoMove, engine, &Engine::redoMove);
    connect(this, &Patience::doDealCard, engine, &Engine::dealCard);
    connect(this, &Patience::doGetHint, engine, &Engine::getHint);
    connect(this, &Patience::doRestoreSavedEngineState, engine, &Engine::restoreSavedState);
    connect(this, &Patience::doSaveEngineState, engine, &Engine::saveState);
    connect(&m_historyConf, &MGConfItem::valueChanged, this, [&] {
        qCDebug(lcPatience) << "Saved history:" << m_historyConf.value().toString();
    });
    connect(&m_historyConf, &MGConfItem::valueChanged, this, &Patience::historyChanged);
    connect(&m_timer, &Timer::tick, this, &Patience::elapsedTimeChanged);
    connect(&m_timer, &Timer::statusChanged, this, &Patience::pausedChanged);
    m_engineThread.start();
}

Patience::~Patience()
{
    m_engineThread.quit();
    m_engineThread.wait();
}

void Patience::newTable(Table *table)
{
    if (s_testMode & TestModeEnabled)
        connect(table, &Table::cardTextureUpdated, this, &Patience::handleCardTextureUpdated);
}

void Patience::addArguments(QCommandLineParser *parser)
{
    parser->addOption({"test", "Run in test mode"});
}

void Patience::setArguments(QCommandLineParser *parser)
{
    s_testMode = parser->isSet("test") ? TestModeEnabled : TestModeDisabled;
}

void Patience::startNewGame()
{
    qCDebug(lcPatience) << "Starting new game";
    emit doStart();
}

void Patience::restartGame()
{
    qCDebug(lcPatience) << "Restarting game";
    emit doRestart();
}

void Patience::loadGame(const QString &gameFile)
{
    qCDebug(lcPatience) << "Loading" << gameFile;
    if (gameFile.isEmpty())
        qCWarning(lcPatience) << "gameFile can not be empty";
    else if (m_gameFile != gameFile)
        emit doLoad(gameFile);
}

void Patience::undoMove()
{
    if (m_canUndo)
        emit doUndoMove();
}

void Patience::redoMove()
{
    if (m_canRedo)
        emit doRedoMove();
}

void Patience::dealCard()
{
    if (m_canDeal)
        emit doDealCard();
}

bool Patience::canUndo() const
{
    return m_canUndo;
}

bool Patience::canRedo() const
{
    return m_canRedo;
}

bool Patience::canDeal() const
{
    return m_canDeal;
}

bool Patience::showDeal() const
{
    return m_showDeal;
}

QString Patience::gameName() const
{
    if (m_gameFile.isEmpty() || m_gameFile.endsWith('-'))
        return QString();
    return GameList::translated(m_gameFile);
}

QString Patience::gameFile() const
{
    if (m_gameFile.endsWith('-'))
        return m_gameFile.left(m_gameFile.length()-1);
    return m_gameFile;
}

QString Patience::helpFile() const
{
    if (m_gameFile.isEmpty() || m_gameFile.endsWith('-'))
        return QString();
    auto file = QStringLiteral(QUOTE(DATADIR) "/help/%1.xml")
        .arg(m_gameFile.left(m_gameFile.length()-4).replace("-", "_"));
    if (!QFile::exists(file))
        return QString();
    return file;
}

void Patience::getHint()
{
    emit doGetHint();
}

int Patience::score() const
{
    return m_score;
}

bool Patience::showScore() const
{
    return m_showScore;
}

QString Patience::elapsedTime() const
{
    return m_timer.elapsed();
}

qint64 Patience::elapsedTimeMs() const
{
    return m_timer.elapsedMSecs();
}

Patience::GameState Patience::state() const
{
    return m_state;
}

void Patience::setState(GameState state)
{
    if (m_state != state) {
        qCDebug(lcPatience) << "Setting game state to" << state;
        switch (state) {
        case UninitializedState:
        case RestoringState:
        case RestartingState:
            break;
        case LoadedState:
        case StartingState:
            if (m_state != RestoringState)
                m_timer.reset();
            break;
        case RunningState: {
            if (m_state <= StartingState)
                m_timer.start();
            else
                m_timer.extend();
            break;
        }
        case GameOverState:
        case WonState:
            m_timer.stop();
            break;
        }
        m_state = state;
        emit stateChanged();
    }
}

bool Patience::paused() const
{
    return m_timer.status() == Timer::TimerPaused;
}

void Patience::setPaused(bool paused)
{
    if (state() == RunningState) {
        if (paused) {
            if (m_timer.status() == Timer::TimerRunning)
                m_timer.pause();
            emit doSaveEngineState();
        } else {
            if (m_timer.status() == Timer::TimerPaused)
                m_timer.unpause();
        }
    }
}

QString Patience::message() const
{
    return m_message;
}

QStringList Patience::history() const
{
    auto list = m_historyConf.value().toString().split(';');
    list.removeAll(QString());
    return list;
}

bool Patience::engineFailed() const
{
    return m_engineFailed;
}

void Patience::restoreSavedOrLoad(const QString &fallback)
{
    qCDebug(lcPatience) << "Asking engine to restore saved game";
    m_gameFile = fallback + '-';
    emit doRestoreSavedEngineState();
}

bool Patience::testMode() const
{
    return s_testMode;
}

void Patience::catchFailure(QString message) {
    qCCritical(lcPatience) << "Engine failed!" << message;
    m_engineFailed = true;
    emit engineFailedChanged();
    m_timer.stop();
}

void Patience::handleGameLoaded(const QString &gameFile)
{
    qCDebug(lcPatience) << "Loaded game" << gameFile;
    if (m_gameFile != gameFile) {
        m_gameFile = gameFile;
        emit gameNameChanged();
    }
    setState(LoadedState);

    auto history = m_historyConf.value();
    if (history.isValid()) {
        auto list = history.toString().split(';').mid(0, 10);
        list.prepend(gameFile);
        list.removeDuplicates();
        m_historyConf.set(list.join(';'));
    } else {
        m_historyConf.set(gameFile);
    }
}

void Patience::handleGameStarted()
{
    qCDebug(lcPatience) << "Game started";
    setState(state() == RestoringState ? RunningState : StartingState);
    if (s_testMode & TestModeEnabled) {
        s_testMode |= TestModeReplayDone;
        qCInfo(lcTestMode) << "Game started in test mode.";
        testModeCompleted();
    }
}

void Patience::handleGameContinued()
{
    qCDebug(lcPatience) << "Game continued";
    setState(RunningState);
}

void Patience::handleCardMoved()
{
    if (state() == StartingState)
        setState(RunningState);
}

void Patience::handleGameOver(bool won)
{
    qCDebug(lcPatience) << "Game" << (won ? "won" : "over");
    setState(won ? WonState : GameOverState);
}

void Patience::handleCanUndoChanged(bool canUndo)
{
    if (m_canUndo != canUndo) {
        qCDebug(lcPatience) << (canUndo ? "Can" : "Can't") << "undo";
        m_canUndo = canUndo;
        emit canUndoChanged();
    }
}

void Patience::handleCanRedoChanged(bool canRedo)
{
    if (m_canRedo != canRedo) {
        qCDebug(lcPatience) << (canRedo ? "Can" : "Can't") << "redo";
        m_canRedo = canRedo;
        emit canRedoChanged();
    }
}

void Patience::handleCanDealChanged(bool canDeal)
{
    if (m_canDeal != canDeal) {
        qCDebug(lcPatience) << (canDeal ? "Can" : "Can't") << "deal";
        m_canDeal = canDeal;
        emit canDealChanged();
    }
}

void Patience::handleScoreChanged(int score)
{
    if (m_score != score) {
        qCDebug(lcPatience) << "Score is now" << score;
        m_score = score;
        emit scoreChanged();
    }
}

void Patience::handleMessageChanged(const QString &message)
{
    if (m_message != message) {
        qCDebug(lcPatience) << "New message" << message;
        m_message = message;
        emit messageChanged();
    }
}

void Patience::handleShowScore(bool show)
{
    if (m_showScore != show) {
        qCDebug(lcPatience) << (show ? "Showing" : "Not showing") << "score";
        m_showScore = show;
        emit showScoreChanged();
    }
}

void Patience::handleShowDeal(bool show)
{
    if (m_showDeal != show) {
        qCDebug(lcPatience) << (show ? "Showing" : "Not showing") << "deal";
        m_showDeal = show;
        emit showDealChanged();
    }
}

void Patience::handleRestoreStarted(qint64 time)
{
    qCDebug(lcPatience) << "Game restore started";
    m_timer.setElapsedMSecs(time);
    emit elapsedTimeChanged();
    setState(RestoringState);
}

void Patience::handleRestoreCompleted(bool restored, bool success)
{
    qCDebug(lcPatience) << "Game" << (restored ? "restored," : "not restored,") << "success:" << success;
    if (restored && !success)
        setState(RestartingState);
    if (!success && m_gameFile.endsWith('-')) {
        auto fallback = m_gameFile.left(m_gameFile.length()-1);
        qCDebug(lcPatience) << "Could not restore game, load instead"
                            << fallback;
        loadGame(fallback);
    }
}

void Patience::handleCardTextureUpdated()
{
    if (s_testMode & TestModeEnabled) {
        s_testMode |= TestModeTextureDrawn;
        qCInfo(lcTestMode) << "Card texture drawn in test mode.";
        testModeCompleted();
    }
}

void Patience::testModeCompleted()
{
    if (s_testMode & TestModeComplete) {
        qCInfo(lcTestMode) << "All tasks finished in test mode. Closing.";
        QTimer::singleShot(TestModeDelay, this, [this]() {
            int exitCode = 5;
            switch (m_state) {
            case UninitializedState:
            case LoadedState:
            case RestartingState:
            case RestoringState:
                break;
            case StartingState:
                exitCode = 4;
                break;
            case RunningState:
                exitCode = 3;
                break;
            case GameOverState:
                exitCode = 2;
                break;
            case WonState:
                exitCode = 0;
                break;
            }
            QCoreApplication::instance()->exit(exitCode);
        });
    }
}
