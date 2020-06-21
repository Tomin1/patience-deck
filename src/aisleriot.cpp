#include <QObject>
#include <QJSEngine>
#include <QQmlEngine>
#include "aisleriot.h"
#include "card.h"
#include "slot.h"

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
    , m_state(Aisleriot::UninitializedState)
{
    scm_c_define_module("aisleriot interface", interfaceInit, this);
}

Aisleriot::~Aisleriot()
{
}

void Aisleriot::startNewGame()
{
    // TODO
}

void Aisleriot::restartGame()
{
    // TODO
}

bool Aisleriot::loadGame(QString gameFile)
{
    Q_UNUSED(gameFile) // TODO
    return false; // TODO
}

void Aisleriot::undoMove()
{
    // TODO
}

void Aisleriot::redoMove()
{
    // TODO
}

bool Aisleriot::canUndo() const
{
    return m_canUndo;
}

void Aisleriot::setCanUndo(bool canUndo)
{
    if (m_canUndo != canUndo) {
        m_canUndo = canUndo;
        emit canUndoChanged();
    }
}

bool Aisleriot::canRedo() const
{
    return m_canRedo;
}

void Aisleriot::setCanRedo(bool canUndo)
{
    if (m_canRedo != canUndo) {
        m_canRedo = canUndo;
        emit canRedoChanged();
    }
}

bool Aisleriot::canDeal() const
{
    return m_canDeal;
}

void Aisleriot::setCanDeal(bool canUndo)
{
    if (m_canDeal != canUndo) {
        m_canDeal = canUndo;
        emit canDealChanged();
    }
}

QString Aisleriot::gameFile() const
{
    return m_gameFile;
}

void Aisleriot::setGameFile(QString file)
{
    if (m_gameFile != file) {
        m_gameFile = file;
        emit gameFileChanged();
    }
}

int Aisleriot::score() const
{
    return m_score;
}

void Aisleriot::setScore(int score)
{
    if (m_score != score) {
        m_score = score;
        emit scoreChanged();
    }
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

void Aisleriot::setMessage(QString message)
{
    if (m_message != message) {
        m_message = message;
        emit messageChanged();
    }
}

void Aisleriot::interfaceInit(void *data)
{
    // See aisleriot/src/game.c:cscm_init for reference
    Q_UNUSED(data)
    /* List all C functions that Aisleriot can call */
    scm_c_define_gsubr("set-feature-word!", 1, 0, 0, (void *)AisleriotSCM::setFeatureWord);
    scm_c_define_gsubr("get-feature-word", 0, 0, 0, (void *)AisleriotSCM::getFeatureWord);
    scm_c_define_gsubr("set-statusbar-message-c", 1, 0, 0, (void *)AisleriotSCM::setStatusbarMessage);
    scm_c_define_gsubr("reset-surface", 0, 0, 0, (void *)AisleriotSCM::resetSurface);
    scm_c_define_gsubr("add-slot", 1, 0, 0, (void *)AisleriotSCM::addCardSlot);
    scm_c_define_gsubr("get-slot", 1, 0, 0, (void *)AisleriotSCM::getCardSlot);
    scm_c_define_gsubr("set-cards-c!", 2, 0, 0, (void *)AisleriotSCM::setCards);
    scm_c_define_gsubr("set-slot-y-expansion!", 2, 0, 0, (void *)AisleriotSCM::setSlotYExpansion);
    scm_c_define_gsubr("set-slot-x-expansion!", 2, 0, 0, (void *)AisleriotSCM::setSlotXExpansion);
    scm_c_define_gsubr("set-lambda", 8, 0, 1, (void *)AisleriotSCM::setLambda);
    scm_c_define_gsubr("set-lambda!", 2, 0, 0, (void *)AisleriotSCM::setLambdaX);
    scm_c_define_gsubr("aisleriot-random", 1, 0, 0, (void *)AisleriotSCM::myrandom);
    scm_c_define_gsubr("click-to-move?", 0, 0, 0, (void *)AisleriotSCM::clickToMoveP);
    scm_c_define_gsubr("update-score", 1, 0, 0, (void *)AisleriotSCM::updateScore);
    scm_c_define_gsubr("get-timeout", 0, 0, 0, (void *)AisleriotSCM::getTimeout);
    scm_c_define_gsubr("set-timeout!", 1, 0, 0, (void *)AisleriotSCM::setTimeout);
    scm_c_define_gsubr("delayed-call", 1, 0, 0, (void *)AisleriotSCM::delayedCall);
    scm_c_define_gsubr("undo-set-sensitive", 1, 0, 0, (void *)AisleriotSCM::undoSetSensitive);
    scm_c_define_gsubr("redo-set-sensitive", 1, 0, 0, (void *)AisleriotSCM::redoSetSensitive);
    scm_c_define_gsubr("dealable-set-sensitive", 1, 0, 0, (void *)AisleriotSCM::dealableSetSensitive);

    scm_c_export("set-feature-word!", "get-feature-word", "set-statusbar-message-c",
                 "reset-surface", "add-slot", "get-slot", "set-cards-c!",
                 "set-slot-y-expansion!", "set-slot-x-expansion!",
                 "set-lambda", "set-lambda!", "aisleriot-random",
                 "click-to-move?", "update-score", "get-timeout",
                 "set-timeout!", "delayed-call", "undo-set-sensitive",
                 "redo-set-sensitive", "dealable-set-sensitive", NULL);
}

void Aisleriot::endMove()
{
    makeSCMCall(QStringLiteral("end-move"), NULL, 0, NULL);
}

void Aisleriot::updateDealable()
{
    SCM rv;
    if ((m_features & FeatureDealable) != 0
            && makeSCMCall(m_lambdas[DealableLambda], NULL, 0, &rv)) {
        setCanDeal(scm_is_true(rv));
    }
}

bool Aisleriot::winningGame()
{
    return makeTestLambdaCall(WinningGameLambda);
}

void Aisleriot::addSlot(QSharedPointer<Slot> slot)
{
    m_cardSlots.append(slot);
}

QSharedPointer<Slot> Aisleriot::getSlot(int slot)
{
    Q_ASSERT_X(slot > 0 && slot < m_cardSlots.count(), "getSlot", "invalid slot index");
    return m_cardSlots[slot];
}

void Aisleriot::testGameOver()
{
    endMove();
    updateDealable();
    if (m_state < GameOverState) {
        if (makeTestLambdaCall(GameOverLambda)) {
            GameState newState;
            if (winningGame())
                newState = WonState;
            else
                newState = GameOverState;
            setState(newState);
        }
    }
}

void Aisleriot::clearGame()
{
    m_cardSlots.clear();
    // TODO: Emit signal for clearing game
}
