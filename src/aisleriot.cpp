#include <QDir>
#include <QObject>
#include <QJSEngine>
#include <QQmlEngine>
#include "aisleriot.h"
#include "constants.h"
#include "gamelist.h"
#include "logging.h"

Aisleriot* Aisleriot::s_game = nullptr;

Aisleriot* Aisleriot::instance()
{
    if (!s_game)
        s_game = new Aisleriot();
    return s_game;
}

QObject* Aisleriot::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return instance();
}

Aisleriot::Aisleriot(QObject *parent)
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
    connect(engine, &Engine::gameLoaded, this, &Aisleriot::handleGameLoaded);
    connect(engine, &Engine::gameStarted, this, &Aisleriot::handleGameStarted);
    connect(engine, &Engine::gameOver, this, &Aisleriot::handleGameOver);
    connect(engine, &Engine::canUndoChanged, this, &Aisleriot::handleCanUndoChanged);
    connect(engine, &Engine::canRedoChanged, this, &Aisleriot::handleCanRedoChanged);
    connect(engine, &Engine::canDealChanged, this, &Aisleriot::handleCanDealChanged);
    connect(engine, &Engine::scoreChanged, this, &Aisleriot::handleScoreChanged);
    connect(engine, &Engine::messageChanged, this, &Aisleriot::handleMessageChanged);
    connect(engine, &Engine::engineFailure, this, &Aisleriot::catchFailure);
    connect(this, &Aisleriot::doStart, engine, &Engine::start);
    connect(this, &Aisleriot::doRestart, engine, &Engine::restart);
    connect(this, &Aisleriot::doLoad, engine, &Engine::load);
    connect(this, &Aisleriot::doUndoMove, engine, &Engine::undoMove);
    connect(this, &Aisleriot::doRedoMove, engine, &Engine::redoMove);
    m_engineThread.start();
}

Aisleriot::~Aisleriot()
{
    m_engineThread.quit();
    m_engineThread.wait();
}

void Aisleriot::startNewGame()
{
    qCDebug(lcAisleriot) << "Starting new game";
    emit doStart();
}

void Aisleriot::restartGame()
{
    qCDebug(lcAisleriot) << "Retarting game";
    emit doRestart();
}

void Aisleriot::loadGame(const QString &gameFile)
{
    qCDebug(lcAisleriot) << "Loading" << gameFile;
    if (gameFile.isEmpty())
        catchFailure("gameFile can not be empty");
    if (m_gameFile != gameFile)
        emit doLoad(gameFile);
}

void Aisleriot::undoMove()
{
    if (m_canUndo)
        emit doUndoMove();
}

void Aisleriot::redoMove()
{
    if (m_canRedo)
        emit doRedoMove();
}

bool Aisleriot::canUndo() const
{
    return m_canUndo;
}

bool Aisleriot::canRedo() const
{
    return m_canRedo;
}

bool Aisleriot::canDeal() const
{
    return m_canDeal;
}

QString Aisleriot::gameName() const
{
    return GameList::displayable(m_gameFile);
}

int Aisleriot::score() const
{
    return m_score;
}

Aisleriot::GameState Aisleriot::state() const
{
    return m_state;
}

void Aisleriot::setState(GameState state)
{
    if (m_state != state) {
        m_state = state;
        // TODO: Stop timer, record time
        emit stateChanged();
    }
}

QString Aisleriot::message() const
{
    return m_message;
}

void Aisleriot::catchFailure(QString message) {
    qCritical() << "Engine failed!" << message;
    abort();
}

void Aisleriot::handleGameLoaded(const QString &gameFile)
{
    qCDebug(lcAisleriot) << "Loaded game" << gameFile;
    if (m_gameFile != gameFile) {
        m_gameFile = gameFile;
        emit gameNameChanged();
    }
    setState(LoadedState);
}

void Aisleriot::handleGameStarted()
{
    qCDebug(lcAisleriot) << "Game started";
    setState(RunningState);
}

void Aisleriot::handleGameOver(bool won)
{
    qCDebug(lcAisleriot) << "Game" << (won ? "won" : "over");
    setState(won ? WonState : GameOverState);
}

void Aisleriot::handleCanUndoChanged(bool canUndo)
{
    qCDebug(lcAisleriot) << (canUndo ? "Can" : "Can't") << "undo";
    m_canUndo = canUndo;
    emit canUndoChanged();
}

void Aisleriot::handleCanRedoChanged(bool canRedo)
{
    qCDebug(lcAisleriot) << (canRedo ? "Can" : "Can't") << "redo";
    m_canRedo = canRedo;
    emit canRedoChanged();
}

void Aisleriot::handleCanDealChanged(bool canDeal)
{
    qCDebug(lcAisleriot) << (canDeal ? "Can" : "Can't") << "deal";
    m_canDeal = canDeal;
    emit canDealChanged();
}

void Aisleriot::handleScoreChanged(int score)
{
    qCDebug(lcAisleriot) << "Score is now" << score;
    if (m_score != score) {
        m_score = score;
        emit scoreChanged();
    }
}

void Aisleriot::handleMessageChanged(const QString &message)
{
    qCDebug(lcAisleriot) << "New message" << message;
    if (m_message != message) {
        m_message = message;
        emit messageChanged();
    }
}
