/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021  Tomi Leppänen
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

#include <QDebug>
#include "constants.h"
#include "engine.h"
#include "engine_p.h"
#include "gameoptionmodel.h"
#include "interface.h"
#include "logging.h"

const QString Constants::GameDirectory = QStringLiteral(QUOTE(DATADIR) "/games");
const QString StateConf = QStringLiteral("/state");

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
    , m_delayedCallTimer(nullptr)
    , m_features(NoFeatures)
    , m_state(UninitializedState)
    , m_timeout(0)
    , m_canUndo(false)
    , m_canRedo(false)
    , m_canDeal(false)
    , m_seed(std::mt19937::default_seed)
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
    , m_stateConf(Constants::ConfPath + StateConf)
{
    qRegisterMetaType<CardData>();
    qRegisterMetaType<CardList>();
    qRegisterMetaType<GameOption>();
    qRegisterMetaType<GameOptionList>();
    connect(&m_stateConf, &MGConfItem::valueChanged, this, [&]() {
        qCDebug(lcEngine) << (m_stateConf.value().isValid()
                              ? "Saved engine state" : "Reset saved state");
    });
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
        d_ptr->die("Loading new game failed");
    } else {
        qCDebug(lcEngine) << "Loaded" << gameFile;
        d_ptr->m_state = EnginePrivate::LoadedState;
        d_ptr->m_gameFile = gameFile;
        GameOptionList options = d_ptr->getGameOptions();
        if (!options.isEmpty() && GameOptionModel::loadOptions(gameFile, options))
            setGameOptions(options);
        emit gameLoaded(gameFile);
    }
}

void Engine::start() {
    startEngine(d_ptr->m_state != EnginePrivate::RestoredState);
}

void Engine::startEngine(bool newSeed)
{
    qCDebug(lcEngine) << "Starting engine";
    if (d_ptr->m_state == EnginePrivate::UninitializedState) {
        d_ptr->die("Game must be initialized first");
        return;
    }
    d_ptr->resetGenerator(newSeed);
    d_ptr->m_state = EnginePrivate::BeginState;
    bool error = false;
    scm_c_catch(SCM_BOOL_T, Scheme::startNewGame, this->d_ptr,
                Scheme::catchHandler, &error, Scheme::preUnwindHandler, &error);
    if (error) {
        qCWarning(lcEngine) << "A scheme error happened while starting new game";
        d_ptr->die("Starting new game failed");
    } else {
        d_ptr->m_state = EnginePrivate::RunningState;
        emit gameStarted();
    }
}

void Engine::restart()
{
    if (d_ptr->m_state < EnginePrivate::BeginState) {
        d_ptr->die("Game has not been started yet. Can not restart!");
        return;
    }
    startEngine(false);
}

void Engine::undoMove()
{
    if (d_ptr->m_state == EnginePrivate::GameOverState) {
        d_ptr->m_state = EnginePrivate::RunningState;
        emit gameStarted();
    }
    if (!d_ptr->makeSCMCall(QStringLiteral("undo"), nullptr, 0, nullptr))
        d_ptr->die("Can not undo move");
    d_ptr->updateDealable();
}

void Engine::redoMove()
{
    if (!d_ptr->makeSCMCall(QStringLiteral("redo"), nullptr, 0, nullptr))
        d_ptr->die("Can not redo move");
    d_ptr->testGameOver();
}

void Engine::dealCard()
{
    d_ptr->recordMove(-1);
    if (!d_ptr->makeSCMCall(QStringLiteral("do-deal-next-cards"), nullptr, 0, nullptr))
        d_ptr->die("Can not deal card");
    d_ptr->endMove();
    d_ptr->testGameOver();
}

void Engine::getHint()
{
    SCM data;
    QString message = QStringLiteral("Hints are not supported");
    if (!d_ptr->makeSCMCall(EnginePrivate::HintLambda, nullptr, 0, &data))
        d_ptr->die("Can not get hint");

    scm_dynwind_begin((scm_t_dynwind_flags)0);
    if (!scm_is_false(data)) {
        int type = scm_to_int(SCM_CAR(data));
        if (type == 0) {
            SCM string = SCM_CADR(data);
            auto msg = Scheme::getUtf8String(string);
            if (!msg.isEmpty())
                message = msg;
        } else if (type == 1 || type == 2) {
            SCM string1 = SCM_CADR(data);
            SCM string2 = SCM_CADDR(data);
            auto msg1 = Scheme::getUtf8String(string1);
            auto msg2 = Scheme::getUtf8String(string2);
            if (!msg1.isEmpty() && !msg2.isEmpty())
                message = QStringLiteral("Move %1 onto %2.").arg(msg1).arg(msg2);
        }
    }
    scm_dynwind_end();
    emit hint(message);
}

void Engine::drag(quint32 id, int slotId, const CardList &cards)
{
    d_ptr->recordMove(slotId);

    SCM args[2];
    args[0] = scm_from_int(slotId);
    args[1] = Scheme::slotToSCM(cards);

    SCM rv;
    if (!d_ptr->makeSCMCall(EnginePrivate::ButtonPressedLambda, args, 2, &rv))
        d_ptr->die("Can not start drag");

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
    if (!d_ptr->makeSCMCall(EnginePrivate::DroppableLambda, args, 3, &rv))
        d_ptr->die("Can not check if dropping is allowed");

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
    if (!d_ptr->makeSCMCall(EnginePrivate::ButtonReleasedLambda, args, 3, &rv))
        d_ptr->die("Can not drop");

    scm_remember_upto_here(args[0], args[1], args[2]);

    if (scm_is_true(rv))
        d_ptr->testGameOver();
    else
        d_ptr->discardMove();

    emit dropped(id, scm_is_true(rv));
}

void Engine::click(quint32 id, int slotId)
{
    d_ptr->recordMove(-1);

    SCM args[1];
    args[0] = scm_from_int(slotId);

    SCM rv;
    if (!d_ptr->makeSCMCall(EnginePrivate::ButtonClickedLambda, args, 1, &rv))
        d_ptr->die("Can not click");

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
    if (!d_ptr->makeSCMCall(EnginePrivate::ButtonDoubleClickedLambda, args, 1, &rv))
        d_ptr->die("Can not double click");

    scm_remember_upto_here_1(args[0]);

    emit doubleClicked(id, scm_is_true(rv));
}

void Engine::requestGameOptions()
{
    emit gameOptions(d_ptr->getGameOptions());
}

GameOptionList EnginePrivate::getGameOptions()
{
    SCM optionsList;
    if (!makeSCMCall(GetOptionsLambda, NULL, 0, &optionsList))
        die("Can not get game options");

    if (scm_is_false(scm_list_p(optionsList))) {
        return GameOptionList();
    }

    scm_dynwind_begin((scm_t_dynwind_flags)0);

    uint length = scm_to_uint(scm_length(optionsList));
    GameOptionType type = CheckGameOption;
    GameOptionList list;

    for (uint i = 0; i < length; i++) {
        SCM entry = scm_list_ref(optionsList, scm_from_uint(i));
        if (scm_is_false(scm_list_p(entry))) {
            // Atom => change mode
            type = (type == CheckGameOption) ? RadioGameOption : CheckGameOption;
        } else {
            char *name = scm_to_utf8_string(scm_list_ref(entry, scm_from_uint(0)));
            scm_dynwind_free(name);
            if (!name) {
                qCWarning(lcEngine) << "Bad utf8 string";
                continue;
            }

            list.append({ name, type, i, scm_is_true(scm_list_ref(entry, scm_from_uint(1))) });
        }
    }

    scm_dynwind_end();

    qCDebug(lcEngine) << "Constructed a list of game options";
    return list;
}

void Engine::setGameOption(const GameOption &option)
{
    SCM optionsList;
    if (!d_ptr->makeSCMCall(EnginePrivate::GetOptionsLambda, NULL, 0, &optionsList))
        d_ptr->die("Can not get game option");

    if (scm_is_false(scm_list_p(optionsList))) {
        d_ptr->die("Game doesn't have options but Patience tried to set an option anyway");
        return;
    }

    SCM entry = scm_list_ref(optionsList, scm_from_uint(option.index));

    if (scm_is_false(scm_list_p(entry))) {
        d_ptr->die("Option is an atom, can not set it");
        return;
    }

    scm_list_set_x(entry, scm_from_uint(1), option.set ? SCM_BOOL_T : SCM_BOOL_F);

    if (!d_ptr->makeSCMCall(EnginePrivate::ApplyOptionsLambda, &optionsList, 1, NULL))
        d_ptr->die("Can not apply game option");

    scm_remember_upto_here_1(optionsList);
}

void Engine::setGameOptions(const GameOptionList &options)
{
    SCM optionsList;
    if (!d_ptr->makeSCMCall(EnginePrivate::GetOptionsLambda, NULL, 0, &optionsList))
        d_ptr->die("Can not get options");

    if (scm_is_false(scm_list_p(optionsList))) {
        d_ptr->die("Game doesn't have options but Patience tried to set an option anyway");
        return;
    }

    scm_dynwind_begin((scm_t_dynwind_flags)0);

    for (const GameOption &option : options) {
        SCM entry = scm_list_ref(optionsList, scm_from_uint(option.index));

        if (scm_is_false(scm_list_p(entry))) {
            d_ptr->die("Option is an atom, can not set it");
            return;
        }

        scm_list_set_x(entry, scm_from_uint(1), option.set ? SCM_BOOL_T : SCM_BOOL_F);
    }

    if (!d_ptr->makeSCMCall(EnginePrivate::ApplyOptionsLambda, &optionsList, 1, NULL))
        d_ptr->die("Can not apply options");

    scm_dynwind_end();
}

void Engine::saveState()
{
    m_stateConf.set(QStringLiteral("%1;%2").arg(d_ptr->m_gameFile).arg(d_ptr->m_seed));
}

void Engine::resetSavedState()
{
    m_stateConf.set(d_ptr->m_gameFile);
}

void Engine::restoreSavedState()
{
    if (d_ptr->m_state < EnginePrivate::GameOverState
            && d_ptr->m_state > EnginePrivate::UninitializedState) {
        qCWarning(lcEngine) << "Engine running, can not set seed";
    } else {
        auto state = m_stateConf.value();
        if (state.isValid()) {
            bool ok = true;
            auto parts = state.toString().split(';');
            auto gameFile = parts.at(0);
            if (parts.count() >= 2)
                d_ptr->m_seed = parts.at(1).toULongLong(&ok);
            if (ok) {
                load(gameFile);
                if (parts.count() >= 2)
                    d_ptr->m_state = EnginePrivate::RestoredState;
            }
            emit restoreCompleted(ok);
        } else {
            qCDebug(lcEngine) << "Engine state was not stored, not restored";
            emit restoreCompleted(false);
        }
    }
}

void EnginePrivate::updateDealable()
{
    SCM rv;
    if (hasFeature(FeatureDealable)) {
        if (!makeSCMCall(EnginePrivate::DealableLambda, nullptr, 0, &rv))
            die("Can not check dealable");
        setCanDeal(scm_is_true(rv));
    }
}

void EnginePrivate::recordMove(int slotId)
{
    qCDebug(lcEngine) << "Start recording move for slot" << slotId
                      << "with" << m_cardSlots[slotId].count() << "cards";
    SCM args[2];
    args[0] = scm_from_int(slotId);
    args[1] = Scheme::slotToSCM(m_cardSlots[slotId]);

    if (!makeSCMCall(QStringLiteral("record-move"), args, 2, nullptr)) {
        die("Can not record move");
        return;
    }

    scm_remember_upto_here_2(args[0], args[1]);
}

void EnginePrivate::endMove()
{
    qCDebug(lcEngine) << "End recorded move";
    if (!makeSCMCall(QStringLiteral("end-move"), nullptr, 0, nullptr))
        die("Can not end move");
}

void EnginePrivate::discardMove()
{
    qCDebug(lcEngine) << "Discard recorded move";
    if (!makeSCMCall(QStringLiteral("discard-move"), nullptr, 0, nullptr))
        die("Can not discard move");
}

bool EnginePrivate::isGameOver()
{
    SCM rv;
    // This is called GAME_OVER_LAMBDA in GNOME Aisleriot
    // but that doesn't really reflect its meaning
    if (!makeSCMCall(MovesLeftLambda, nullptr, 0, &rv))
        die("Can not check if game is over");
    return !scm_is_true(rv);
}

bool EnginePrivate::isWinningGame()
{
    SCM rv;
    if (!makeSCMCall(WinningGameLambda, nullptr, 0, &rv))
        die("Can not check if game is won");
    return scm_is_true(rv);
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
    if (cards.isEmpty()) {
        qCDebug(lcEngine) << "Clearing slot" << id;
        emit engine()->clearSlot(id);
        m_cardSlots[id].clear();
        return;
    }

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

quint32 EnginePrivate::getRandomValue(quint32 first, quint32 last) {
    std::uniform_int_distribution<quint32> distribution(first, last);
    return distribution(m_generator);
}

void EnginePrivate::resetGenerator(bool generateNewSeed)
{
    static std::random_device seedGenerator;
    if (generateNewSeed)
        m_seed = seedGenerator();
    m_generator = std::mt19937(m_seed);
}

void EnginePrivate::die(const char *message)
{
    emit engine()->engineFailure(QString(message));
    abort();
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
