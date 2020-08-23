#include "engine.h"
#include "engine_p.h"
#include "card.h"
#include "slot.h"
#include "logging.h"
#include "interface.h"

#include <unistd.h>
#include <QDebug>

EnginePrivate::EnginePrivate(QObject *parent)
    : QObject(parent)
    , m_generator(m_rd())
    , m_delayedCallTimer(nullptr)
    , m_features(NoFeatures)
    , m_timeout(0)
    , m_canUndo(false)
    , m_canRedo(false)
    , m_canDeal(false)
    , m_state(UninitializedState)
{
}

EnginePrivate::~EnginePrivate()
{
    if (m_delayedCallTimer) {
        m_delayedCallTimer->stop();
        delete m_delayedCallTimer;
    }
}

EnginePrivate *EnginePrivate::instance()
{
    Q_ASSERT_X(!Engine::s_engine.isNull(), Q_FUNC_INFO, "Engine must have been created when calling instance()");
    return Engine::s_engine->d_ptr;
}

QSharedPointer<Engine> Engine::s_engine;

QSharedPointer<Engine> Engine::instance(const QString &loadPath)
{
    if (s_engine.isNull())
        s_engine = QSharedPointer<Engine>(new Engine(loadPath));
    return s_engine;
}

Engine::Engine(const QString &loadPath, QObject *parent)
    : QObject(parent)
    , d_ptr(new EnginePrivate(this))
{
    scm_with_guile(&Interface::init, (void *)&loadPath);
    scm_primitive_load_path(scm_from_utf8_string("api.scm"));
    qCDebug(lcEngine) << "Initialized Aisleriot Engine";
}

Engine::~Engine()
{
    s_engine.clear();
}

bool Engine::load(const QString &gameFile)
{
    bool error = false;
    d_ptr->clear();
    scm_c_catch(SCM_BOOL_T, Scheme::loadGameFromFile, (void *)&gameFile,
                Scheme::catchHandler, nullptr, Scheme::preUnwindHandler, &error);
    if (error) {
        qCWarning(lcEngine) << "A scheme error happened while loading";
        return false;
    } else {
        d_ptr->setGameFile(gameFile);
        d_ptr->setState(EnginePrivate::LoadedState);
        qCDebug(lcEngine) << "Loaded" << gameFile;
        return true;
    }
}

bool Engine::start()
{
    Q_ASSERT_X(state() != UninitializedState, __FUNCTION__, "Game must be initialized first");
    d_ptr->setState(EnginePrivate::BeginState);
    bool error = false;
    scm_c_catch(SCM_BOOL_T, Scheme::startNewGame, this->d_ptr,
                Scheme::catchHandler, nullptr, Scheme::preUnwindHandler, &error);
    if (error) {
        qCWarning(lcEngine) << "A scheme error happened while starting new game";
        return false;
    }
    return true;
}

void Engine::undoMove()
{
    if (state() == GameOverState)
        d_ptr->setState(EnginePrivate::RunningState);
    d_ptr->makeSCMCall(QStringLiteral("undo"), nullptr, 0, nullptr);
    d_ptr->updateDealable();
}

void Engine::redoMove()
{
    d_ptr->makeSCMCall(QStringLiteral("redo"), nullptr, 0, nullptr);
    d_ptr->testGameOver();
}

void EnginePrivate::updateDealable()
{
    SCM rv;
    if (hasFeature(FeatureDealable) && makeSCMCall(EnginePrivate::DealableLambda, nullptr, 0, &rv)) {
        setCanDeal(scm_is_true(rv));
    }
}

void EnginePrivate::endMove()
{
    makeSCMCall(QStringLiteral("end-move"), nullptr, 0, nullptr);
}

bool EnginePrivate::isGameOver()
{
    SCM rv;
    return makeSCMCall(GameOverLambda, nullptr, 0, &rv) && scm_is_true(rv);
}

bool EnginePrivate::isWinningGame()
{
    SCM rv;
    return makeSCMCall(WinningGameLambda, nullptr, 0, &rv) && scm_is_true(rv);
}

void EnginePrivate::clear()
{
    setState(UninitializedState);
    setFeatures(0);
    setCanUndo(false);
    setCanRedo(false);
    setCanDeal(false);
}

void EnginePrivate::testGameOver()
{
    endMove();
    updateDealable();
    if (m_state < GameOverState) {
        if (isGameOver()) {
            setState(isWinningGame() ? WonState : GameOverState);
        }
    }
}

bool Engine::canUndo() const
{
    return d_ptr->m_canUndo;
}

void EnginePrivate::setCanUndo(bool canUndo)
{
    if (m_canUndo != canUndo) {
        m_canUndo = canUndo;
        emit engine()->canUndoChanged();
    }
}

bool Engine::canRedo() const
{
    return d_ptr->m_canRedo;
}

void EnginePrivate::setCanRedo(bool canUndo)
{
    if (m_canRedo != canUndo) {
        m_canRedo = canUndo;
        emit engine()->canRedoChanged();
    }
}

bool Engine::canDeal() const
{
    return d_ptr->m_canDeal;
}

void EnginePrivate::setCanDeal(bool canUndo)
{
    if (m_canDeal != canUndo) {
        m_canDeal = canUndo;
        emit engine()->canDealChanged();
    }
}

Engine::GameState Engine::state() const
{
    return static_cast<GameState>(d_ptr->m_state);
}

EnginePrivate::GameState EnginePrivate::getState() const
{
    return m_state;
}

void EnginePrivate::setState(GameState state)
{
    if (m_state != state) {
        m_state = state;
        // TODO: Stop timer, record time
        emit engine()->stateChanged();
    }
}

int Engine::score() const
{
    return d_ptr->m_score;
}

void EnginePrivate::setScore(int score)
{
    if (m_score != score) {
        m_score = score;
        emit engine()->scoreChanged();
    }
}

QString Engine::gameFile() const
{
    return d_ptr->m_gameFile;
}

void EnginePrivate::setGameFile(QString gameFile)
{
    if (m_gameFile != gameFile) {
        m_gameFile = gameFile;
        emit engine()->gameFileChanged();
    }
}

QString Engine::message() const
{
    return d_ptr->m_message;
}

void EnginePrivate::setMessage(QString message)
{
    qCDebug(lcEngine) << "Setting message to" << message;
    if (m_message != message) {
        m_message = message;
        emit engine()->messageChanged();
    }
}

void EnginePrivate::setWidth(double width)
{
    Q_UNUSED(width); // TODO
    qCDebug(lcEngine) << "Setting width is not yet implemented!";
}


void EnginePrivate::setHeight(double height)
{
    Q_UNUSED(height); // TODO
    qCDebug(lcEngine) << "Setting height is not yet implemented!";
}

void EnginePrivate::addSlot(QSharedPointer<Slot> slot)
{
    m_cardSlots.append(slot);
}

QSharedPointer<Slot> EnginePrivate::getSlot(int slot)
{
    Q_ASSERT_X(slot > 0 && slot < m_cardSlots.count(), __FUNCTION__, "invalid slot index");
    return m_cardSlots[slot];
}

void EnginePrivate::setLambda(EnginePrivate::Lambda lambda, SCM func)
{
    m_lambdas[lambda] = func;
}

uint EnginePrivate::getFeatures()
{
    return m_features;
}

void EnginePrivate::setFeatures(uint features)
{
    m_features = static_cast<EnginePrivate::GameFeatures>(features);
}

bool EnginePrivate::hasFeature(GameFeature feature)
{
    return (m_features & feature) != 0;
}

int EnginePrivate::getTimeout()
{
    return m_timeout;
}

void EnginePrivate::setTimeout(int timeout)
{
    if (m_timeout != timeout) {
        m_timeout = timeout;
        // TODO: Emit timeout changed signal
    }
}

bool EnginePrivate::makeSCMCall(Lambda lambda, SCM *args, size_t n, SCM *retval)
{
    return makeSCMCall(m_lambdas[lambda], args, n, retval);
}

bool EnginePrivate::makeSCMCall(SCM lambda, SCM *args, size_t n, SCM *retval)
{
    Interface::Call call = { lambda, args, n };
    bool error = false;

    SCM r = scm_c_catch(SCM_BOOL_T, Scheme::callLambda, &call,
                        Scheme::catchHandler, nullptr, Scheme::preUnwindHandler, &error);
    if (error) {
        qCWarning(lcEngine) << "Scheme reported an error";
        return false;
    }

    if (retval)
        *retval = r;
    return true;
}

bool EnginePrivate::makeSCMCall(QString name, SCM *args, size_t n, SCM *retval)
{
    SCM lambda = scm_c_eval_string(name.toUtf8().data());
    if (!makeSCMCall(lambda, args, n, retval))
        return false;
    scm_remember_upto_here_1(lambda);
    return true;
}

Engine *EnginePrivate::engine()
{
    return static_cast<Engine *>(parent());
}
