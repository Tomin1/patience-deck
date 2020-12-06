/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020  Tomi Leppänen
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

#include <QDir>
#include <QObject>
#include <QJSEngine>
#include <QQmlEngine>
#include "patience.h"
#include "constants.h"
#include "gamelist.h"
#include "logging.h"

Patience* Patience::s_game = nullptr;

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
    , m_canUndo(false)
    , m_canRedo(false)
    , m_canDeal(false)
    , m_score(0)
    , m_state(UninitializedState)
{
    auto engine = Engine::instance();
    engine->moveToThread(&m_engineThread);
    connect(&m_engineThread, &QThread::started, engine, &Engine::init);
    connect(&m_engineThread, &QThread::finished, engine, &Engine::deleteLater);
    connect(engine, &Engine::gameLoaded, this, &Patience::handleGameLoaded);
    connect(engine, &Engine::gameStarted, this, &Patience::handleGameStarted);
    connect(engine, &Engine::gameOver, this, &Patience::handleGameOver);
    connect(engine, &Engine::canUndoChanged, this, &Patience::handleCanUndoChanged);
    connect(engine, &Engine::canRedoChanged, this, &Patience::handleCanRedoChanged);
    connect(engine, &Engine::canDealChanged, this, &Patience::handleCanDealChanged);
    connect(engine, &Engine::scoreChanged, this, &Patience::handleScoreChanged);
    connect(engine, &Engine::messageChanged, this, &Patience::handleMessageChanged);
    connect(engine, &Engine::engineFailure, this, &Patience::catchFailure);
    connect(this, &Patience::doStart, engine, &Engine::start);
    connect(this, &Patience::doRestart, engine, &Engine::restart);
    connect(this, &Patience::doLoad, engine, &Engine::load);
    connect(this, &Patience::doUndoMove, engine, &Engine::undoMove);
    connect(this, &Patience::doRedoMove, engine, &Engine::redoMove);
    m_engineThread.start();
}

Patience::~Patience()
{
    m_engineThread.quit();
    m_engineThread.wait();
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
    if (gameFile.isEmpty()) {
        qCritical() << "gameFile can not be empty";
        abort();
    }
    if (m_gameFile != gameFile)
        emit doLoad(gameFile);
        // TODO: Load game options from dconf
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

QString Patience::gameName() const
{
    return GameList::displayable(m_gameFile);
}

int Patience::score() const
{
    return m_score;
}

Patience::GameState Patience::state() const
{
    return m_state;
}

void Patience::setState(GameState state)
{
    if (m_state != state) {
        m_state = state;
        // TODO: Stop timer, record time
        emit stateChanged();
    }
}

QString Patience::message() const
{
    return m_message;
}

QString Patience::aisleriotAuthors() const
{
    QFile file(Constants::DataDirectory + QStringLiteral("/AUTHORS"));

    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(lcPatience) << "Can not open" << file.fileName() << "for reading";
        return QString();
    }

    QTextStream in(&file);
    QString authors;
    while (!in.atEnd()) {
        QString line = in.readLine();
        authors += line.left(line.lastIndexOf(QStringLiteral(" <")));
        authors += '\n';
    }
    return authors;
}

bool Patience::showAllGames() const
{
    return GameList::showAll();
}

void Patience::setShowAllGames(bool show)
{
    if (show != GameList::showAll()) {
        GameList::setShowAll(show);
        emit showAllGamesChanged();
    }
}

void Patience::catchFailure(QString message) {
    qCritical() << "Engine failed!" << message;
}

void Patience::handleGameLoaded(const QString &gameFile)
{
    qCDebug(lcPatience) << "Loaded game" << gameFile;
    if (m_gameFile != gameFile) {
        m_gameFile = gameFile;
        emit gameNameChanged();
    }
    setState(LoadedState);
}

void Patience::handleGameStarted()
{
    qCDebug(lcPatience) << "Game started";
    setState(RunningState);
}

void Patience::handleGameOver(bool won)
{
    qCDebug(lcPatience) << "Game" << (won ? "won" : "over");
    setState(won ? WonState : GameOverState);
}

void Patience::handleCanUndoChanged(bool canUndo)
{
    qCDebug(lcPatience) << (canUndo ? "Can" : "Can't") << "undo";
    m_canUndo = canUndo;
    emit canUndoChanged();
}

void Patience::handleCanRedoChanged(bool canRedo)
{
    qCDebug(lcPatience) << (canRedo ? "Can" : "Can't") << "redo";
    m_canRedo = canRedo;
    emit canRedoChanged();
}

void Patience::handleCanDealChanged(bool canDeal)
{
    qCDebug(lcPatience) << (canDeal ? "Can" : "Can't") << "deal";
    m_canDeal = canDeal;
    emit canDealChanged();
}

void Patience::handleScoreChanged(int score)
{
    qCDebug(lcPatience) << "Score is now" << score;
    if (m_score != score) {
        m_score = score;
        emit scoreChanged();
    }
}

void Patience::handleMessageChanged(const QString &message)
{
    qCDebug(lcPatience) << "New message" << message;
    if (m_message != message) {
        m_message = message;
        emit messageChanged();
    }
}
