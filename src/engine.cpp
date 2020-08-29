#include <QDebug>
#include "constants.h"
#include "engine.h"
#include "engine_p.h"
#include "interface.h"
#include "logging.h"

const QString Constants::GameDirectory = QStringLiteral("/usr/share/mobile-aisleriot/games");

EnginePrivate::EnginePrivate(QObject *parent)
    : QObject(parent)
    , m_generator(m_rd())
    , m_delayedCallTimer(nullptr)
    , m_features(NoFeatures)
    , m_state(UninitializedState)
    , m_timeout(0)
    , m_canUndo(false)
    , m_canRedo(false)
    , m_canDeal(false)
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
    if (!Engine::s_engine)
        qCCritical(lcEngine) << "Engine must have been created when calling instance()";
    return Engine::s_engine->d_ptr;
}

Engine* Engine::s_engine = nullptr;

Engine *Engine::instance()
{
    if (!s_engine) {
        qCDebug(lcEngine) << "No engine yet, must create a new engine";
        s_engine = new Engine(nullptr);
    }
    return s_engine;
}

Engine::Engine(QObject *parent)
    : QObject(parent)
    , d_ptr(new EnginePrivate(this))
{
}

void Engine::init()
{
    scm_with_guile(&Interface::init, (void *)&Constants::GameDirectory);
    qCInfo(lcEngine) << "Initialized Aisleriot Engine";
}

Engine::~Engine()
{
    s_engine = nullptr;
}

void Engine::load(const QString &gameFile)
{
    qCDebug(lcEngine) << "Loading game from" << gameFile;
    d_ptr->clear();
    bool error = false;
    scm_c_catch(SCM_BOOL_T, Scheme::loadGameFromFile, (void *)&gameFile,
                Scheme::catchHandler, &error, Scheme::preUnwindHandler, &error);
    if (error) {
        qCWarning(lcEngine) << "A scheme error happened while loading";
        emit engineFailure("Loading new game failed");
    } else {
        qCDebug(lcEngine) << "Loaded" << gameFile;
        d_ptr->m_state = EnginePrivate::LoadedState;
        emit gameLoaded(gameFile);
    }
}

void Engine::start()
{
    qCDebug(lcEngine) << "Starting engine";
    if (d_ptr->m_state == EnginePrivate::UninitializedState)
        emit engineFailure("Game must be initialized first");
    d_ptr->m_state = EnginePrivate::BeginState;
    bool error = false;
    scm_c_catch(SCM_BOOL_T, Scheme::startNewGame, this->d_ptr,
                Scheme::catchHandler, &error, Scheme::preUnwindHandler, &error);
    if (error) {
        qCWarning(lcEngine) << "A scheme error happened while starting new game";
        emit engineFailure("Starting new game failed");
    } else {
        d_ptr->m_state = EnginePrivate::RunningState;
        emit gameStarted();
    }
}

void Engine::restart()
{
    // TODO
    qCWarning(lcEngine) << "Restarting is not implemented yet!";
}

void Engine::undoMove()
{
    if (d_ptr->m_state == EnginePrivate::GameOverState) {
        d_ptr->m_state = EnginePrivate::RunningState;
        emit gameStarted();
    }
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

bool EnginePrivate::isInitialized()
{
    return m_state > BeginState;
}

void EnginePrivate::clear()
{
    m_state = UninitializedState;
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
            m_state = GameOverState;
            emit engine()->gameOver(isWinningGame());
        }
    }
}

void EnginePrivate::setCanUndo(bool canUndo)
{
    if (m_canUndo != canUndo) {
        m_canUndo = canUndo;
        emit engine()->canUndoChanged(canUndo);
    }
}

void EnginePrivate::setCanRedo(bool canRedo)
{
    if (m_canRedo != canRedo) {
        m_canRedo = canRedo;
        emit engine()->canRedoChanged(canRedo);
    }
}

void EnginePrivate::setCanDeal(bool canDeal)
{
    if (m_canDeal != canDeal) {
        m_canDeal = canDeal;
        emit engine()->canDealChanged(canDeal);
    }
}

void EnginePrivate::setScore(int score)
{
    emit engine()->scoreChanged(score);
}

void EnginePrivate::setMessage(QString message)
{
    qCDebug(lcEngine) << "Setting message to" << message;
    emit engine()->messageChanged(message);
}

void EnginePrivate::setWidth(double width)
{
    Q_UNUSED(width); // TODO
    qCWarning(lcEngine) << "Setting width is not yet implemented!";
}


void EnginePrivate::setHeight(double height)
{
    Q_UNUSED(height); // TODO
    qCWarning(lcEngine) << "Setting height is not yet implemented!";
}

void EnginePrivate::addSlot(int id, QList<Card> cards, SlotType type,
                            double x, double y, int expansionDepth,
                            bool expandedDown, bool expandedRight)
{
    m_cardSlots.insert(id, cards);
    qCDebug(lcEngine) << "Added new slot with id" << id << "and" << cards.count() << "cards";
    emit engine()->newSlot(id, type, x, y, expansionDepth, expandedDown, expandedRight);
    for (const Card &card : cards) {
        emit engine()->newCard(id, card.suit, card.rank, card.faceDown);
    }
}

const QList<EnginePrivate::Card> EnginePrivate::getSlot(int slot)
{
    return m_cardSlots[slot];
}

void EnginePrivate::setCards(int id, QList<Card> cards)
{
    // TODO: Instead of clearing it every time,
    // check what needs to be changed and adjust
    emit engine()->clearSlot(id);
    m_cardSlots.insert(id, cards);
    qCDebug(lcEngine) << "Set" << cards.count() << "cards to slot with id" << id;
    for (const Card &card : cards) {
        emit engine()->newCard(id, card.suit, card.rank, card.faceDown);
    }
}

void EnginePrivate::setExpansionToDown(int id, double expansion)
{
    emit Engine::instance()->setExpansionToDown(id, expansion);
}

void EnginePrivate::setExpansionToRight(int id, double expansion)
{
    emit Engine::instance()->setExpansionToRight(id, expansion);
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

void EnginePrivate::die(const char *message)
{
    emit engine()->engineFailure(QString(message));
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
                        Scheme::catchHandler, &error, Scheme::preUnwindHandler, &error);
    if (error) {
        qCWarning(lcEngine) << "Scheme reported an error";
        die("Making SCM call failed!");
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
