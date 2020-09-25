#include <QDebug>
#include "constants.h"
#include "engine.h"
#include "engine_p.h"
#include "interface.h"
#include "logging.h"

const QString Constants::GameDirectory = QStringLiteral(QUOTE(DATADIR) "/games");

bool CardData::operator==(const CardData &other) const
{
    return suit == other.suit && rank == other.rank && show == other.show;
};

bool CardData::operator!=(const CardData &other) const
{
    return !(*this == other);
};

CardData::operator QString() const
{
    return QStringLiteral("Rank %1 of %2%3")
        .arg(QString::number(rank))
        .arg(QString::number(suit))
        .arg(!show ? QStringLiteral(" from behind") : QString());
};

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
    qRegisterMetaType<CardData>();
    qRegisterMetaType<CardList>();
    qCDebug(lcEngine) << "Patience Engine created";
}

void Engine::init()
{
    scm_with_guile(&Interface::init, (void *)&Constants::GameDirectory);
    qCInfo(lcEngine) << "Initialized Patience Engine";
}

Engine::~Engine()
{
    s_engine = nullptr;
}

void Engine::load(const QString &gameFile)
{
    qCDebug(lcEngine) << "Loading game from" << gameFile;
    d_ptr->clear(true);
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
    if (d_ptr->m_state == EnginePrivate::UninitializedState) {
        emit engineFailure("Game must be initialized first");
        return;
    }
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

void Engine::drag(quint32 id, int slotId, const CardList &cards)
{
    d_ptr->recordMove(slotId);

    SCM args[2];
    args[0] = scm_from_int(slotId);
    args[1] = Scheme::slotToSCM(cards);

    SCM rv;
    d_ptr->makeSCMCall(EnginePrivate::ButtonPressedLambda, args, 2, &rv);

    scm_remember_upto_here_2(args[0], args[1]);

    if (scm_is_true(rv)) {
        // Remove cards from the slot, assumes that they are removed from the end
        for (int i = cards.count(); i > 0; i--)
            d_ptr->m_cardSlots[slotId].removeLast();
    }

    emit couldDrag(id, scm_is_true(rv));
}

void Engine::cancelDrag(quint32 id, int slotId, const CardList &cards)
{
    Q_UNUSED(id) // There is no signal to send back
    d_ptr->m_cardSlots[slotId].append(cards);
    d_ptr->discardMove();
}

void Engine::checkDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards)
{
    if (!d_ptr->hasFeature(EnginePrivate::FeatureDroppable)) {
        emit couldDrop(id, false);
        return;
    }

    SCM args[3];
    args[0] = scm_from_int(startSlotId);
    args[1] = Scheme::slotToSCM(cards);
    args[2] = scm_from_int(endSlotId);

    SCM rv;
    d_ptr->makeSCMCall(EnginePrivate::DroppableLambda, args, 3, &rv);

    scm_remember_upto_here(args[0], args[1], args[2]);

    emit couldDrop(id, scm_is_true(rv));
}

void Engine::drop(quint32 id, int startSlotId, int endSlotId, const CardList &cards)
{
    SCM args[3];
    args[0] = scm_from_int(startSlotId);
    args[1] = Scheme::slotToSCM(cards);
    args[2] = scm_from_int(endSlotId);

    SCM rv;
    d_ptr->makeSCMCall(EnginePrivate::ButtonReleasedLambda, args, 3, &rv);

    scm_remember_upto_here(args[0], args[1], args[2]);

    if (scm_is_true(rv))
        d_ptr->testGameOver();
    else
        d_ptr->discardMove();

    emit dropped(id, scm_is_true(rv));
}

void Engine::click(quint32 id, int slotId)
{
    // Click implies that there is drag already started,
    // this is a deviation from the original engine,
    // which uses threshold until it starts a drag
    d_ptr->discardMove();
    d_ptr->recordMove(-1);

    SCM args[1];
    args[0] = scm_from_int(slotId);

    SCM rv;
    d_ptr->makeSCMCall(EnginePrivate::ButtonClickedLambda, args, 1, &rv);

    scm_remember_upto_here_1(args[0]);

    if (scm_is_true(rv))
        d_ptr->testGameOver();
    else
        d_ptr->discardMove();

    emit clicked(id, scm_is_true(rv));
}

void Engine::doubleClick(quint32 id, int slotId)
{
    SCM args[1];
    args[0] = scm_from_int(slotId);

    SCM rv;
    d_ptr->makeSCMCall(EnginePrivate::ButtonDoubleClickedLambda, args, 1, &rv);

    scm_remember_upto_here_1(args[0]);

    emit doubleClicked(id, scm_is_true(rv));
}

void EnginePrivate::updateDealable()
{
    SCM rv;
    if (hasFeature(FeatureDealable) && makeSCMCall(EnginePrivate::DealableLambda, nullptr, 0, &rv)) {
        setCanDeal(scm_is_true(rv));
    }
}

void EnginePrivate::recordMove(int slotId)
{
    SCM args[2];
    args[0] = scm_from_int(slotId);
    args[1] = Scheme::slotToSCM(m_cardSlots[slotId]);

    if (!makeSCMCall(QStringLiteral("record-move"), args, 2, nullptr))
        return;

    scm_remember_upto_here_2(args[0], args[1]);
}

void EnginePrivate::endMove()
{
    makeSCMCall(QStringLiteral("end-move"), nullptr, 0, nullptr);
}

void EnginePrivate::discardMove()
{
    makeSCMCall(QStringLiteral("discard-move"), nullptr, 0, nullptr);
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

void EnginePrivate::clear(bool resetData)
{
    if (resetData) {
        m_state = UninitializedState;
        setFeatures(0);
        setCanUndo(false);
        setCanRedo(false);
        setCanDeal(false);
    }
    m_cardSlots.clear();
    emit engine()->clearData();
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
    qCDebug(lcEngine) << "Message changed to" << message;
    emit engine()->messageChanged(message);
}

void EnginePrivate::setWidth(double width)
{
    emit engine()->widthChanged(width);
}


void EnginePrivate::setHeight(double height)
{
    emit engine()->heightChanged(height);
}

void EnginePrivate::addSlot(int id, const CardList &cards, SlotType type,
                            double x, double y, int expansionDepth,
                            bool expandedDown, bool expandedRight)
{
    m_cardSlots.insert(id, cards);
    emit engine()->newSlot(id, cards, type, x, y, expansionDepth, expandedDown, expandedRight);
}

const CardList &EnginePrivate::getSlot(int slot)
{
    return m_cardSlots[slot];
}

void EnginePrivate::setCards(int id, const CardList &cards)
{
    auto it = cards.constBegin();
    int i = 0;
    for (; it != cards.end() && i < m_cardSlots[id].count(); it++, i++) {
        if (*it != m_cardSlots[id][i]) {
            qCDebug(lcEngine) << "Inserting" << *it << "to position" << i << "at slot" << id;
            emit engine()->insertCard(id, i, *it);
            m_cardSlots[id].insert(i, *it);
        }
    }
    for (; it != cards.end(); it++, i++) {
        qCDebug(lcEngine) << "Appending" << *it << "to slot" << id;
        emit engine()->appendCard(id, *it);
        m_cardSlots[id].append(*it);
    }
    while (i < m_cardSlots[id].count()) {
        qCDebug(lcEngine) << "Remove" << m_cardSlots[id][i] << "from slot" << id;
        emit engine()->removeCard(id, i);
        m_cardSlots[id].removeAt(i);
    }

    if (m_cardSlots[id] != cards)
        die("Cards don't match!");
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
    qCDebug(lcEngine) << "Setting features to" << static_cast<EnginePrivate::GameFeatures>(features);
    m_features = static_cast<EnginePrivate::GameFeatures>(features);
}

bool EnginePrivate::hasFeature(GameFeature feature)
{
    return m_features.testFlag(feature);
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
