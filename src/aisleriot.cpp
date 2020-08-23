#include <QDir>
#include <QObject>
#include <QJSEngine>
#include <QQmlEngine>
#include "aisleriot.h"
#include "constants.h"

const QString Constants::GameDirectory = QStringLiteral("/usr/share/mobile-aisleriot/games");

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
    , m_engine(Engine::instance(Constants::GameDirectory))
{
    connect(m_engine.data(), &Engine::canUndoChanged, this, &Aisleriot::canUndoChanged);
    connect(m_engine.data(), &Engine::canRedoChanged, this, &Aisleriot::canRedoChanged);
    connect(m_engine.data(), &Engine::canDealChanged, this, &Aisleriot::canDealChanged);
    connect(m_engine.data(), &Engine::stateChanged, this, &Aisleriot::stateChanged);
    connect(m_engine.data(), &Engine::scoreChanged, this, &Aisleriot::scoreChanged);
    connect(m_engine.data(), &Engine::gameFileChanged, this, &Aisleriot::gameFileChanged);
    connect(m_engine.data(), &Engine::messageChanged, this, &Aisleriot::messageChanged);
    connect(m_engine.data(), &Engine::gameLoaded, this, &Aisleriot::gameLoaded);
}

Aisleriot::~Aisleriot()
{
}

void Aisleriot::startNewGame()
{
    Q_ASSERT_X(m_engine->start(), Q_FUNC_INFO, "Could not start game");
}

void Aisleriot::restartGame()
{
    // TODO
}

bool Aisleriot::loadGame(QString gameFile)
{
    // TODO: Do this in a background thread
    Q_ASSERT_X(!gameFile.isEmpty(), Q_FUNC_INFO, "gameFile can not be empty");
    if (m_engine->gameFile() == gameFile)
        return false;
    return m_engine->load(gameFile);
}

void Aisleriot::undoMove()
{
    m_engine->undoMove();
}

void Aisleriot::redoMove()
{
    m_engine->redoMove();
}

bool Aisleriot::canUndo() const
{
    return m_engine->canUndo();
}

bool Aisleriot::canRedo() const
{
    return m_engine->canRedo();
}

bool Aisleriot::canDeal() const
{
    return m_engine->canDeal();
}

QString Aisleriot::gameFile() const
{
    return m_engine->gameFile();
}

int Aisleriot::score() const
{
    return m_engine->score();
}

Aisleriot::GameState Aisleriot::state() const
{
    return static_cast<Aisleriot::GameState>(m_engine->state());
}

QString Aisleriot::message() const
{
    return m_engine->message();
}
