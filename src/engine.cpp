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

#define MAX_RETRIES 10

const QString Constants::GameDirectory = QStringLiteral(QUOTE(DATADIR) "/games");
const QString StateConf = QStringLiteral("/state");

CardData::CardData()
    : suit(static_cast<Suit>(-1))
    , rank(static_cast<Rank>(-1))
    , show(false)
{
}

CardData::CardData(Suit suit, Rank rank, bool show)
    : suit(suit)
    , rank(rank)
    , show(show)
{
}

bool CardData::equalValue(const CardData &other) const
{
    return suit == other.suit && rank == other.rank;
}

bool CardData::operator==(const CardData &other) const
{
    return equalValue(other) && show == other.show;
}

bool CardData::operator!=(const CardData &other) const
{
    return !(*this == other);
}

CardData::operator bool() const
{
    return suit != -1 && rank != -1;
}

SuitAndRank CardData::value() const
{
    return SuitAndRank(suit, rank);
}

QDebug operator<<(QDebug debug, const CardData &data)
{
    debug.nospace() << "Rank " << data.rank << " of suit " << data.suit;
    if (!data.show)
        debug.nospace() << " from behind";
    return debug.space();
}

EnginePrivate::EnginePrivate(QObject *parent)
    : QObject(parent)
    , m_delayedCallTimer(nullptr)
    , m_features(NoFeatures)
    , m_state(UninitializedState)
    , m_timeout(0)
    , m_seed(std::mt19937::default_seed)
    , m_recordingMove(false)
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
    , m_action(0)
#ifndef ENGINE_EXERCISER
    , m_stateConf(Constants::ConfPath + StateConf)
#endif // ENGINE_EXERCISER
{
    qRegisterMetaType<CardData>();
    qRegisterMetaType<CardList>();
    qRegisterMetaType<ActionType>();
    qRegisterMetaType<GameOption>();
    qRegisterMetaType<GameOptionList>();
#ifndef ENGINE_EXERCISER
    connect(&m_stateConf, &MGConfItem::valueChanged, this, [&]() {
        qCDebug(lcEngine) << (m_stateConf.value().isValid()
                              ? "Saved engine state" : "Reset saved state");
    });
#endif // ENGINE_EXERCISER
    qCDebug(lcEngine) << "Patience Engine created";
}

void Engine::init()
{
    initWithDirectory(Constants::GameDirectory);
}

void Engine::initWithDirectory(const QString &gameDirectory)
{
    scm_with_guile(&Interface::init, (void *)&gameDirectory);
    qCInfo(lcEngine) << "Initialized Patience Engine";
}

Engine::~Engine()
{
    s_engine = nullptr;
}

void Engine::load(const QString &gameFile)
{
    loadGame(gameFile, false);
}

void Engine::loadGame(const QString &gameFile, bool restored)
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
        d_ptr->m_state = restored ? EnginePrivate::RestoredState : EnginePrivate::LoadedState;
        d_ptr->m_gameFile = gameFile;
#ifndef ENGINE_EXERCISER
        GameOptionList options = d_ptr->getGameOptions();
        if (!options.isEmpty() && GameOptionModel::loadOptions(gameFile, options) && !setGameOptions(options)) {
            qCWarning(lcEngine) << "Stored game options don't apply, clearing stored game options";
            GameOptionModel::clearOptions(gameFile);
            // Reload to reset options
            loadGame(gameFile, restored);
            return;
        }
#endif // ENGINE_EXERCISER
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

    int count = 0;

    do {
        if (count)
            qCInfo(lcEngine) << "No moves left at the beginning, starting over";

        d_ptr->resetGenerator(newSeed);
        d_ptr->m_state = EnginePrivate::BeginState;
        bool error = false;
        scm_c_catch(SCM_BOOL_T, Scheme::startNewGame, this->d_ptr,
                    Scheme::catchHandler, &error, Scheme::preUnwindHandler, &error);
        if (error) {
            qCWarning(lcEngine) << "A scheme error happened while starting new game";
            d_ptr->die("Starting new game failed");
            return;
        }

        newSeed = true; // If we need to try again, use a new seed anyway
    } while (d_ptr->isGameOver() && count++ < MAX_RETRIES);

    d_ptr->m_state = EnginePrivate::RunningState;
    emit gameStarted();

    d_ptr->testGameOver();
}

void Engine::restart()
{
    if (d_ptr->m_state < EnginePrivate::BeginState)
        d_ptr->die("Game has not been started yet. Can not restart!");
    else
        startEngine(false);
}

void Engine::undoMove()
{
    if (d_ptr->m_state == EnginePrivate::GameOverState) {
        d_ptr->m_state = EnginePrivate::RunningState;
        emit gameContinued();
    }

    if (!d_ptr->makeSCMCall(QStringLiteral("undo"), nullptr, 0, nullptr)) {
        d_ptr->die("Can not undo move");
        return;
    }

    emit moveEnded();
    d_ptr->updateDealable();
}

void Engine::redoMove()
{
    if (!d_ptr->makeSCMCall(QStringLiteral("redo"), nullptr, 0, nullptr)) {
        d_ptr->die("Can not redo move");
    } else {
        emit moveEnded();
        d_ptr->updateDealable();
        d_ptr->testGameOver();
    }
}

void Engine::dealCard()
{
    d_ptr->recordMove(-1);
    if (!d_ptr->makeSCMCall(QStringLiteral("do-deal-next-cards"), nullptr, 0, nullptr))
        d_ptr->die("Can not deal card");
    else
        d_ptr->endMove();
}

void Engine::getHint()
{
    SCM data;
    QString message = QStringLiteral("Hints are not supported");
    if (!d_ptr->makeSCMCall(EnginePrivate::HintLambda, nullptr, 0, &data)) {
        d_ptr->die("Can not get hint");
        return;
    }

    scm_dynwind_begin((scm_t_dynwind_flags)0);
    if (!scm_is_false(data)) {
        int type = scm_to_int(SCM_CAR(data));
        if (type == 0) {
            SCM string = SCM_CADR(data);
            auto msg = Scheme::getUtf8String(string);
            if (!msg.isEmpty()) {
                message = msg;
                if (message.endsWith(QChar('.')))
                    message.truncate(message.length()-1);
            }
        } else if (type == 1 || type == 2) {
            SCM string1 = SCM_CADR(data);
            SCM string2 = SCM_CADDR(data);
            auto msg1 = Scheme::getUtf8String(string1);
            auto msg2 = Scheme::getUtf8String(string2);
            if (!msg1.isEmpty() && !msg2.isEmpty())
                message = QStringLiteral("Move %1 onto %2").arg(msg1).arg(msg2);
        }
    }
    scm_dynwind_end();
    emit hint(message);
}

bool Engine::drag(quint32 id, int slotId, const CardList &cards)
{
    bool could = false;
    if (m_action != 0)
        qCWarning(lcEngine) << "Tried to start dragging while action was ongoing";
    else if (d_ptr->m_delayedCallTimer)
        qCWarning(lcEngine) << "Tried to start dragging while a delayed call was ongoing";
    else if (cards.isEmpty())
        qCWarning(lcEngine) << "Tried to drag empty stack of cards";
    else
        could = true;

    if (!could) {
        emit couldDrag(id, slotId, false);
        return false;
    }

    d_ptr->recordMove(slotId);

    SCM args[2];
    args[0] = scm_from_int(slotId);
    args[1] = Scheme::slotToSCM(cards);

    SCM rv;
    if (!d_ptr->makeSCMCall(EnginePrivate::ButtonPressedLambda, args, 2, &rv)) {
        d_ptr->die("Can not start drag");
        return false;
    }

    scm_remember_upto_here_2(args[0], args[1]);

    if (scm_is_true(rv)) {
        // Remove cards from the slot, assumes that they are removed from the end
        for (int i = cards.count(); i > 0; i--)
            d_ptr->m_cardSlots[slotId].removeLast();
    }

    could = scm_is_true(rv);
    if (could)
        m_action = id;
    emit couldDrag(id, slotId, could);
    return could;
}

void Engine::cancelDrag(quint32 id, int slotId, const CardList &cards)
{
    if (m_action != id) {
        qCWarning(lcEngine) << "Tried to cancel drag for wrong action" << id << ", current" << m_action;
        return;
    }

    Q_UNUSED(id) // There is no signal to send back
    qCDebug(lcEngine) << "Canceling move, putting back" << cards.count() << "cards to slot" << slotId;
    d_ptr->m_cardSlots[slotId].append(cards); // Put the cards back
    d_ptr->discardMove();
    m_action = 0;
}

bool Engine::checkDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards)
{
    bool could = false;
    if (m_action != id)
        qCWarning(lcEngine) << "Tried to check drop for a non-current action" << id << ", current" << m_action;
    else if (!d_ptr->hasFeature(EnginePrivate::FeatureDroppable))
        qCDebug(lcEngine) << "No droppable feature";
    else if (cards.isEmpty())
        qCWarning(lcEngine) << "Tried to drop empty stack of cards";
    else
        could = true;

    if (!could) {
        emit couldDrop(id, endSlotId, false);
        return false;
    }

    SCM args[3];
    args[0] = scm_from_int(startSlotId);
    args[1] = Scheme::slotToSCM(cards);
    args[2] = scm_from_int(endSlotId);

    SCM rv;
    if (!d_ptr->makeSCMCall(EnginePrivate::DroppableLambda, args, 3, &rv)) {
        d_ptr->die("Can not check if dropping is allowed");
        return false;
    }

    scm_remember_upto_here(args[0], args[1], args[2]);

    emit couldDrop(id, endSlotId, scm_is_true(rv));
    return scm_is_true(rv);
}

bool Engine::drop(quint32 id, int startSlotId, int endSlotId, const CardList &cards)
{
    bool could = false;
    if (m_action != id)
        qCWarning(lcEngine) << "Tried to drop cards for a non-current action" << id << ", current" << m_action;
    else if (!d_ptr->hasFeature(EnginePrivate::FeatureDroppable))
        qCDebug(lcEngine) << "No droppable feature";
    else if (cards.isEmpty())
        qCWarning(lcEngine) << "Tried to drop empty stack of cards";
    else
        could = true;

    if (!could) {
        emit dropped(id, endSlotId, false);
        if (m_action == id)
            d_ptr->discardMove();
        return false;
    }

    SCM args[3];
    args[0] = scm_from_int(startSlotId);
    args[1] = Scheme::slotToSCM(cards);
    args[2] = scm_from_int(endSlotId);

    SCM rv;
    if (!d_ptr->makeSCMCall(EnginePrivate::ButtonReleasedLambda, args, 3, &rv)) {
        d_ptr->die("Can not drop");
        return false;
    }

    scm_remember_upto_here(args[0], args[1], args[2]);

    could = scm_is_true(rv);

    emit dropped(id, endSlotId, could);

    if (could)
        d_ptr->endMove();
    else
        d_ptr->discardMove();

    m_action = 0;
    return could;
}

bool Engine::click(quint32 id, int slotId)
{
    if (m_action != 0 || d_ptr->m_delayedCallTimer) {
        qCWarning(lcEngine) << "Tried to click while an action or a delayed call was ongoing";
        return false;
    }

    d_ptr->recordMove(-1);

    SCM args[1];
    args[0] = scm_from_int(slotId);

    SCM rv;
    if (!d_ptr->makeSCMCall(EnginePrivate::ButtonClickedLambda, args, 1, &rv)) {
        d_ptr->die("Can not click");
        return false;
    }

    scm_remember_upto_here_1(args[0]);

    emit clicked(id, slotId, scm_is_true(rv));

    if (scm_is_true(rv))
        d_ptr->endMove();
    else
        d_ptr->discardMove();
    return scm_is_true(rv);
}

bool Engine::doubleClick(quint32 id, int slotId)
{
    if (m_action != 0 || d_ptr->m_delayedCallTimer) {
        qCWarning(lcEngine) << "Tried to double click while an action or a delayed call was ongoing";
        return false;
    }

    d_ptr->recordMove(-1);

    SCM args[1];
    args[0] = scm_from_int(slotId);

    SCM rv;
    if (!d_ptr->makeSCMCall(EnginePrivate::ButtonDoubleClickedLambda, args, 1, &rv)) {
        d_ptr->die("Can not double click");
        return false;
    }

    scm_remember_upto_here_1(args[0]);

    emit doubleClicked(id, slotId, scm_is_true(rv));

    if (scm_is_true(rv))
        d_ptr->endMove();
    else
        d_ptr->discardMove();
    return scm_is_true(rv);
}

void Engine::requestGameOptions()
{
    emit gameOptions(d_ptr->getGameOptions());
}

GameOptionList EnginePrivate::getGameOptions()
{
    SCM optionsList;
    if (!makeSCMCall(GetOptionsLambda, NULL, 0, &optionsList)) {
        die("Can not get game options");
        return GameOptionList();
    }

    if (scm_is_false(scm_list_p(optionsList))) {
        return GameOptionList();
    }

    scm_dynwind_begin((scm_t_dynwind_flags)0);

    uint length = scm_to_uint(scm_length(optionsList));
    uint group = NoOptionGroup;
    bool checkOption = true;
    GameOptionList list;

    for (uint i = 0; i < length; i++) {
        SCM entry = scm_list_ref(optionsList, scm_from_uint(i));
        if (scm_is_false(scm_list_p(entry))) {
            qCDebug(lcOptions) << "Atom at" << i;
            // Atom => change mode
            if (checkOption)
                group++;
            checkOption = !checkOption;
        } else {
            char *name = scm_to_utf8_string(scm_list_ref(entry, scm_from_uint(0)));
            scm_dynwind_free(name);
            if (!name) {
                qCWarning(lcEngine) << "Bad utf8 string";
                continue;
            }

            qCDebug(lcOptions) << (checkOption ? "Checkbox" : "Radio") << "option" << name << "at" << i;
            list.append({
                name,
                checkOption ? NoOptionGroup : group,
                i,
                scm_is_true(scm_list_ref(entry, scm_from_uint(1)))
            });
        }
    }

    scm_dynwind_end();

    qCDebug(lcEngine) << "Constructed a list of game options";
    return list;
}

bool Engine::setGameOption(const GameOption &option)
{
    qCDebug(lcOptions) << "Setting" << option.displayName << "at" << option.index << "to" << option.set;

    SCM optionsList;
    if (!d_ptr->makeSCMCall(EnginePrivate::GetOptionsLambda, NULL, 0, &optionsList)) {
        d_ptr->die("Can not get game option");
        return false;
    }

    if (scm_is_false(scm_list_p(optionsList))) {
        qCWarning(lcEngine) << "Game doesn't have options but Patience tried to set an option anyway";
        return false;
    }

    SCM entry = scm_list_ref(optionsList, scm_from_uint(option.index));

    if (scm_is_false(scm_list_p(entry))) {
        qCWarning(lcEngine) << "Option is an atom, can not set it! Not setting game options";
        return false;
    }

    scm_list_set_x(entry, scm_from_uint(1), option.set ? SCM_BOOL_T : SCM_BOOL_F);

    if (!d_ptr->makeSCMCall(EnginePrivate::ApplyOptionsLambda, &optionsList, 1, NULL)) {
        qCWarning(lcEngine) << "Can not apply option! Not setting game options";
        return false;
    }

    scm_remember_upto_here_1(optionsList);
    return true;
}

bool Engine::setGameOptions(const GameOptionList &options)
{
    qCDebug(lcOptions) << "Setting" << options.count() << "options";
    SCM optionsList;
    if (!d_ptr->makeSCMCall(EnginePrivate::GetOptionsLambda, NULL, 0, &optionsList)) {
        d_ptr->die("Can not get options");
        return false;
    }

    if (scm_is_false(scm_list_p(optionsList))) {
        qCWarning(lcEngine) << "Game doesn't have options but Patience tried to set options anyway";
        return false;
    }

    scm_dynwind_begin((scm_t_dynwind_flags)0);

    for (const GameOption &option : options) {
        qCDebug(lcOptions) << "Setting" << option.displayName << "at" << option.index << "to" << option.set;
        SCM entry = scm_list_ref(optionsList, scm_from_uint(option.index));

        if (scm_is_false(scm_list_p(entry))) {
            qCWarning(lcEngine) << "Option is an atom, can not set it! Not setting game options";
            scm_dynwind_end();
            return false;
        }

        scm_list_set_x(entry, scm_from_uint(1), option.set ? SCM_BOOL_T : SCM_BOOL_F);
    }

    if (!d_ptr->makeSCMCall(EnginePrivate::ApplyOptionsLambda, &optionsList, 1, NULL)) {
        qCWarning(lcEngine) << "Can not apply options! Not setting game options";
        scm_dynwind_end();
        return false;
    }

    scm_dynwind_end();
    return true;
}

#ifndef ENGINE_EXERCISER
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
                loadGame(gameFile, parts.count() >= 2);
            }
            emit restoreCompleted(ok);
        } else {
            qCDebug(lcEngine) << "Engine state was not stored, not restored";
            emit restoreCompleted(false);
        }
    }
}
#endif // ENGINE_EXERCISER

void EnginePrivate::updateDealable()
{
    SCM rv;
    if (hasFeature(FeatureDealable)) {
        if (!makeSCMCall(EnginePrivate::DealableLambda, nullptr, 0, &rv))
            die("Can not check dealable");
        else
            setCanDeal(scm_is_true(rv));
    }
}

void EnginePrivate::recordMove(int slotId)
{
    qCDebug(lcEngine) << "Start recording move for slot" << slotId
                      << "with" << m_cardSlots[slotId].count() << "cards";

    if (m_recordingMove)
        qCCritical(lcEngine) << "There was already a move ongoing";
    m_recordingMove = true;

    SCM args[2];
    args[0] = scm_from_int(slotId);
    args[1] = Scheme::slotToSCM(m_cardSlots[slotId]);

    if (!makeSCMCall(QStringLiteral("record-move"), args, 2, nullptr))
        die("Can not record move");

    scm_remember_upto_here_2(args[0], args[1]);
}

void EnginePrivate::endMove(bool fromDelayedCall)
{
    qCDebug(lcEngine) << "End recorded move";
    if (!makeSCMCall(QStringLiteral("end-move"), nullptr, 0, nullptr))
        die("Can not end move");
    else
        emit engine()->moveEnded();

    if (!fromDelayedCall) {
        if (!m_recordingMove)
            qCWarning(lcEngine) << "There was no move ongoing when ending move";
        m_recordingMove = false;
    }

    updateDealable();
    testGameOver();
}

void EnginePrivate::discardMove()
{
    qCDebug(lcEngine) << "Discard recorded move";
    if (!makeSCMCall(QStringLiteral("discard-move"), nullptr, 0, nullptr))
        die("Can not discard move");

    if (!m_recordingMove)
        qCWarning(lcEngine) << "There was no move ongoing when discarding move";
    m_recordingMove = false;
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
    if (m_state < GameOverState) {
        if (isGameOver()) {
            m_state = GameOverState;
            emit engine()->gameOver(isWinningGame());
        }
    }
}

void EnginePrivate::setCanUndo(bool canUndo)
{
    qCDebug(lcEngine) << (canUndo ? "Can" : "Can't") << "undo";
    emit engine()->canUndo(canUndo);
}

void EnginePrivate::setCanRedo(bool canRedo)
{
    qCDebug(lcEngine) << (canRedo ? "Can" : "Can't") << "redo";
    emit engine()->canRedo(canRedo);
}

void EnginePrivate::setCanDeal(bool canDeal)
{
    qCDebug(lcEngine) << (canDeal ? "Can" : "Can't") << "deal";
    emit engine()->canDeal(canDeal);
}

void EnginePrivate::setScore(int score)
{
    qCDebug(lcEngine) << "Score updated to" << score;
    emit engine()->score(score);
}

void EnginePrivate::setMessage(QString message)
{
    qCDebug(lcEngine) << "Message changed to" << message;
    emit engine()->message(message);
}

void EnginePrivate::setWidth(double width)
{
    qCDebug(lcEngine) << "Width changed to" << width;
    emit engine()->widthChanged(width);
}

void EnginePrivate::setHeight(double height)
{
    qCDebug(lcEngine) << "Height changed to" << height;
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
        if (!m_cardSlots[id].isEmpty()) {
            qCDebug(lcEngine) << "Clearing slot" << id;
            CardData none;
            emit engine()->action(Engine::ClearingAction, id, -1, none);
            m_cardSlots[id].clear();
        }
        return;
    }

    auto it = cards.constEnd();
    int i = m_cardSlots[id].count() - 1;
    while (it-- != cards.constBegin() && i >= 0) {
        if (m_cardSlots[id][i].equalValue(*it)) {
            if ((*it).show != m_cardSlots[id][i].show) {
                qCDebug(lcEngine) << "Flipping" << *it << "in slot" << id << "at index" << i;
                emit engine()->action(Engine::FlippingAction, id, i, *it);
                m_cardSlots[id][i].show = (*it).show;
            }
            i--;
        } else {
            qCDebug(lcEngine) << "Removing" << m_cardSlots[id][i] << "from slot" << id << "from index" << i;
            emit engine()->action(Engine::RemovalAction, id, i, m_cardSlots[id].takeAt(i));
            i--; ++it;
        }
    }
    for (; i >= 0; i--) {
        qCDebug(lcEngine) << "Remove" << m_cardSlots[id][i] << "from slot" << id << "from index" << i;
        emit engine()->action(Engine::RemovalAction, id, i, m_cardSlots[id].takeAt(i));
    }
    ++it;
    while (it-- != cards.constBegin()) {
        qCDebug(lcEngine) << "Appending" << *it << "to slot" << id << "to index 0";
        emit engine()->action(Engine::InsertionAction, id, 0, *it);
        m_cardSlots[id].insert(0, *it);
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
    emit engine()->showScore(!hasFeature(FeatureScoreHidden));
    emit engine()->showDeal(hasFeature(FeatureDealable));
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
