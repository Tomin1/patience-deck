#include <libguile.h>
#include <random>
#include <QObject>
#include <QTimer>
#include "aisleriot_scm.h"
#include "aisleriot.h"
#include "card.h"
#include "logging.h"
#include "slot.h"

namespace {

struct Call {
    SCM lambda;
    SCM *args;
    size_t n;
};

const char LambdaNames[] = {
  "new-game\0"
  "button-pressed\0"
  "button-released\0"
  "button-clicked\0"
  "button-double-clicked\0"
  "game-over\0"
  "winning-game\0"
  "hint\0"
  "get-options\0"
  "apply-options\0"
  "timeout\0"
  "droppable\0"
  "dealable\0"
};

const int DelayedCallDelay = 50;

void *init(void *data);
void interfaceInit(void *data);
SCM startNewGameSCM(void *data);
SCM loadGameSCM(void *data);
SCM setFeatureWord(SCM features);
SCM getFeatureWord(void);
SCM setStatusbarMessage(SCM message);
SCM resetSurface(void);
SCM addCardSlot(SCM slotData);
SCM getCardSlot(SCM slotId);
SCM setCards(SCM slotId, SCM newCards);
SCM setSlotYExpansion(SCM slotId, SCM newExpVal);
SCM setSlotXExpansion(SCM slotId, SCM newExpVal);
SCM setLambda(SCM startGameLambda, SCM pressedLambda, SCM releasedLambda,
              SCM clickedLambda, SCM doubleClickedLambda, SCM gameOverLambda,
              SCM winningGameLambda, SCM hintLambda, SCM rest);
SCM setLambdaX(SCM symbol, SCM lambda);
SCM myrandom(SCM range);
SCM clickToMoveP(void);
SCM updateScore(SCM newScore);
SCM getTimeout(void);
SCM setTimeout(SCM newTimeout);
SCM delayedCall(SCM callback);
SCM undoSetSensitive(SCM state);
SCM redoSetSensitive(SCM state);
SCM dealableSetSensitive(SCM state);
QSharedPointer<Card> createCard(SCM data);
QList<QSharedPointer<Card>> cardsFromSlot(SCM cards);
SCM cardToSCM(QSharedPointer<Card> card);
SCM slotToSCM(QSharedPointer<Slot> slot);
SCM callLambda(void *data);
SCM preUnwindHandler(void *data, SCM tag, SCM throwArgs);
SCM catchHandler(void *data, SCM tag, SCM throwArgs);

void *init(void *data)
{
    scm_c_define_module("aisleriot interface", interfaceInit, data);
    return NULL;
}

void interfaceInit(void *data)
{
    // See aisleriot/src/game.c:cscm_init for reference
    Q_UNUSED(data)
    /* List all C functions that Aisleriot can call */
    scm_c_define_gsubr("set-feature-word!", 1, 0, 0, (void *)setFeatureWord);
    scm_c_define_gsubr("get-feature-word", 0, 0, 0, (void *)getFeatureWord);
    scm_c_define_gsubr("set-statusbar-message-c", 1, 0, 0, (void *)setStatusbarMessage);
    scm_c_define_gsubr("reset-surface", 0, 0, 0, (void *)resetSurface);
    scm_c_define_gsubr("add-slot", 1, 0, 0, (void *)addCardSlot);
    scm_c_define_gsubr("get-slot", 1, 0, 0, (void *)getCardSlot);
    scm_c_define_gsubr("set-cards-c!", 2, 0, 0, (void *)setCards);
    scm_c_define_gsubr("set-slot-y-expansion!", 2, 0, 0, (void *)setSlotYExpansion);
    scm_c_define_gsubr("set-slot-x-expansion!", 2, 0, 0, (void *)setSlotXExpansion);
    scm_c_define_gsubr("set-lambda", 8, 0, 1, (void *)setLambda);
    scm_c_define_gsubr("set-lambda!", 2, 0, 0, (void *)setLambdaX);
    scm_c_define_gsubr("aisleriot-random", 1, 0, 0, (void *)myrandom);
    scm_c_define_gsubr("click-to-move?", 0, 0, 0, (void *)clickToMoveP);
    scm_c_define_gsubr("update-score", 1, 0, 0, (void *)updateScore);
    scm_c_define_gsubr("get-timeout", 0, 0, 0, (void *)getTimeout);
    scm_c_define_gsubr("set-timeout!", 1, 0, 0, (void *)setTimeout);
    scm_c_define_gsubr("delayed-call", 1, 0, 0, (void *)delayedCall);
    scm_c_define_gsubr("undo-set-sensitive", 1, 0, 0, (void *)undoSetSensitive);
    scm_c_define_gsubr("redo-set-sensitive", 1, 0, 0, (void *)redoSetSensitive);
    scm_c_define_gsubr("dealable-set-sensitive", 1, 0, 0, (void *)dealableSetSensitive);

    scm_c_export("set-feature-word!", "get-feature-word", "set-statusbar-message-c",
                 "reset-surface", "add-slot", "get-slot", "set-cards-c!",
                 "set-slot-y-expansion!", "set-slot-x-expansion!",
                 "set-lambda", "set-lambda!", "aisleriot-random",
                 "click-to-move?", "update-score", "get-timeout",
                 "set-timeout!", "delayed-call", "undo-set-sensitive",
                 "redo-set-sensitive", "dealable-set-sensitive", NULL);

    qCDebug(lcScheme) << "Initialized aisleriot interface";
}

SCM startNewGameSCM(void *data)
{
    Aisleriot *game = static_cast<Aisleriot *>(data);
    // TODO: Deal with game over situations
    SCM size = SCM_UNDEFINED;
    Q_ASSERT(game->makeSCMCall(Engine::NewGameLambda, NULL, 0, &size));
    game->setWidth(scm_to_double(SCM_CAR(size)));
    game->setHeight(scm_to_double(SCM_CADR(size)));
    scm_remember_upto_here_1(size);

    game->makeSCMCall(QStringLiteral("start-game"), NULL, 0, NULL);

    if (game->hasFeature(Engine::FeatureDealable)) {
        SCM rv;
        Q_ASSERT(game->makeSCMCall(Engine::DealableLambda, NULL, 0, &rv));
        game->setCanDeal(scm_is_true(rv));
    }

    return SCM_BOOL_T;
}

SCM loadGameSCM(void *data)
{
    QString *gameFile = static_cast<QString *>(data);
    scm_dynwind_begin((scm_t_dynwind_flags)0);
    scm_primitive_load_path(scm_from_utf8_string(gameFile->toUtf8().data()));
    // TODO: Test all lambdas
    scm_dynwind_end();
    return SCM_BOOL_T;
}

SCM setFeatureWord(SCM features)
{
    auto *game = Aisleriot::instance();
    game->m_features = scm_to_uint(features);
    return SCM_EOL;
}

SCM getFeatureWord()
{
    auto *game = Aisleriot::instance();
    return scm_from_uint(game->m_features);
}

SCM setStatusbarMessage(SCM newMessage)
{
    auto *game = Aisleriot::instance();
    if (!scm_is_string(newMessage))
        return SCM_EOL;

    scm_dynwind_begin((scm_t_dynwind_flags)0);

    char *message = scm_to_utf8_string(newMessage);
    scm_dynwind_free(message);

    game->setMessage(QString::fromUtf8(message));

    scm_dynwind_end();
    return SCM_EOL;
}

SCM resetSurface()
{
    auto *game = Aisleriot::instance();
    game->clearGame();
    return SCM_EOL;
}

SCM addCardSlot(SCM slotData)
{
    auto *game = Aisleriot::instance();
    if (game->state() > Aisleriot::BeginState) {
        return scm_throw(scm_from_locale_symbol("aisleriot-invalid-call"),
                         scm_list_1(scm_from_utf8_string("Cannot add a new slot after the game has started.")));
    }

    // Basically copy-paste from aisleriot/src/game.c:cscmi_add_slot
#define EQUALS_SYMBOL(string,object) (scm_is_true (scm_equal_p (scm_from_locale_symbol (string), object)))

    SCM slotPlacement = SCM_CADDR(slotData);
    bool expandedDown, expandedRight;
    double expansionDepth = 0.0;
    if (EQUALS_SYMBOL("expanded", SCM_CAR(slotPlacement))) {
        expandedDown = true;
    } else if (EQUALS_SYMBOL("expanded-right", SCM_CAR(slotPlacement))) {
        expandedRight = true;
    } else if (EQUALS_SYMBOL("partially-expanded", SCM_CAR(slotPlacement))) {
        expandedDown = true;
        expansionDepth = scm_to_int(SCM_CADDR(slotPlacement));
    } else if (EQUALS_SYMBOL("partially-expanded-right", SCM_CAR(slotPlacement))) {
        expandedRight = true;
        expansionDepth = scm_to_int(SCM_CADDR(slotPlacement));
    }

    /* 3rd argument is the slot type (optionally) */
    SCM slotType = SCM_CDDDR(slotData);
    Slot::SlotType type = Slot::UnknownSlot;
    if (slotType != SCM_EOL) {
        if (EQUALS_SYMBOL("chooser", SCM_CAR(slotType))) {
            type = Slot::ChooserSlot;
        } else if (EQUALS_SYMBOL("foundation", SCM_CAR(slotType))) {
            type = Slot::FoundationSlot;
        } else if (EQUALS_SYMBOL("reserve", SCM_CAR(slotType))) {
            type = Slot::ReserveSlot;
        } else if (EQUALS_SYMBOL("stock", SCM_CAR(slotType))) {
            type = Slot::StockSlot;
        } else if (EQUALS_SYMBOL("tableau", SCM_CAR(slotType))) {
            type = Slot::TableauSlot;
        } else if (EQUALS_SYMBOL("waste", SCM_CAR(slotType))) {
            type = Slot::WasteSlot;
        }
    }

#undef EQUALS_SYMBOL
    auto slot = QSharedPointer<Slot>::create(scm_to_int(SCM_CAR(slotData)), type,
                                             scm_to_double(SCM_CAR(SCM_CADR(SCM_CADDR(slotData)))),
                                             scm_to_double(SCM_CAR(SCM_CADR(SCM_CADDR(slotData)))),
                                             expansionDepth, expandedDown, expandedRight);

    slot->setCards(cardsFromSlot(SCM_CADR(slotData)));

    game->addSlot(slot);

    return SCM_EOL;
}

SCM getCardSlot(SCM slotId)
{
    auto *game = Aisleriot::instance();
    QSharedPointer<Slot> slot = game->getSlot(scm_to_int(slotId));
    if (!slot)
        return SCM_EOL;

    return scm_cons(slotId, scm_cons(slotToSCM(slot), SCM_EOL));
}

SCM setCards(SCM slotId, SCM newCards)
{
    auto *game = Aisleriot::instance();
    QSharedPointer<Slot> slot = game->getSlot(scm_to_int(slotId));
    Q_ASSERT_X(slot, __FUNCTION__, "no such slot");
    slot->setCards(cardsFromSlot(newCards));

    return SCM_BOOL_T;
}

SCM setSlotYExpansion(SCM slotId, SCM value)
{
    auto *game = Aisleriot::instance();
    QSharedPointer<Slot> slot = game->getSlot(scm_to_int(slotId));
    Q_ASSERT_X(slot, __FUNCTION__, "no such slot");
    if (!slot->expandsDown())
        return SCM_EOL;
    if (slot->expandedRight())
        return SCM_EOL;
    slot->setExpansionToDown(scm_to_double(value));
    return SCM_EOL;
}

SCM setSlotXExpansion(SCM slotId, SCM value)
{
    auto *game = Aisleriot::instance();
    QSharedPointer<Slot> slot = game->getSlot(scm_to_int(slotId));
    Q_ASSERT_X(slot, __FUNCTION__, "no such slot");
    if (!slot->expandsRight())
        return SCM_EOL;
    if (slot->expandedDown())
        return SCM_EOL;
    slot->setExpansionToRight(scm_to_double(value));
    return SCM_EOL;
}

SCM setLambda(SCM startGameLambda, SCM pressedLambda, SCM releasedLambda,
                                SCM clickedLambda, SCM doubleClickedLambda, SCM gameOverLambda,
                                SCM winningGameLambda, SCM hintLambda, SCM rest)
{
    auto *game = Aisleriot::instance();
    game->m_lambdas[Engine::NewGameLambda] = startGameLambda;
    game->m_lambdas[Engine::ButtonPressedLambda] = pressedLambda;
    game->m_lambdas[Engine::ButtonReleasedLambda] = releasedLambda;
    game->m_lambdas[Engine::ButtonClickedLambda] = clickedLambda;
    game->m_lambdas[Engine::ButtonDoubleClickedLambda] = doubleClickedLambda;
    game->m_lambdas[Engine::GameOverLambda] = gameOverLambda;
    game->m_lambdas[Engine::WinningGameLambda] = winningGameLambda;
    game->m_lambdas[Engine::HintLambda] = hintLambda;

    game->m_lambdas[Engine::GetOptionsLambda] = SCM_CAR(rest);
    rest = SCM_CDR(rest);
    game->m_lambdas[Engine::ApplyOptionsLambda] = SCM_CAR(rest);
    rest = SCM_CDR(rest);
    game->m_lambdas[Engine::TimeoutLambda] = SCM_CAR(rest);
    rest = SCM_CDR(rest);

    if (game->m_features & Engine::FeatureDroppable) {
        game->m_lambdas[Engine::DroppableLambda] = SCM_CAR(rest);
        rest = SCM_CDR(rest);
    } else {
        game->m_lambdas[Engine::DroppableLambda] = SCM_UNDEFINED;
    }

    if (game->m_features & Engine::FeatureDroppable) {
        game->m_lambdas[Engine::DealableLambda] = SCM_CAR(rest);
        rest = SCM_CDR(rest);
    } else {
        game->m_lambdas[Engine::DealableLambda] = SCM_UNDEFINED;
    }

    return SCM_EOL;
}

SCM setLambdaX(SCM symbol, SCM lambda)
{
    auto *game = Aisleriot::instance();
    // Basically copy-paste from aisleriot/src/game.c:scm_set_lambda_x
    // TODO: maybe rewrite to use hash table instead

    const char *lambdaName = LambdaNames;
    for (int i = 0; i < Engine::LambdaCount; ++i) {
        if (scm_is_true(scm_equal_p(symbol, scm_from_locale_symbol(lambdaName)))) {
            game->m_lambdas[i] = lambda;
            return SCM_EOL;
        }

        lambdaName += strlen(lambdaName) + 1;
    }

    return scm_throw(scm_from_locale_symbol("aisleriot-invalid-call"),
                     scm_list_1(scm_from_utf8_string("Unknown lambda name in set-lambda!")));
}

SCM myrandom(SCM range)
{
    // TODO: Needs to be reimplemented
    auto *game = Aisleriot::instance();
    std::uniform_int_distribution<quint32> distribution(0, scm_to_int(range));
    return scm_from_uint32(distribution(game->m_generator));
}

SCM clickToMoveP(void)
{
    // See aisleriot/src/game.c:scm_click_to_move_p for explanation
    return SCM_BOOL_F;
}

SCM updateScore(SCM newScore)
{
    auto *game = Aisleriot::instance();
    char *score = scm_to_utf8_string(newScore);
    bool ok;
    int value = QString::fromUtf8(score).toInt(&ok);
    Q_ASSERT_X(ok, __FUNCTION__, "expected an integer value");
    game->setScore(value);
    free(score);
    return newScore;
}

SCM getTimeout(void)
{
    auto *game = Aisleriot::instance();
    return scm_from_int(game->m_timeout);
}

SCM setTimeout(SCM newTimeout)
{
    auto *game = Aisleriot::instance();
    int timeout = scm_to_int(newTimeout);
    if (game->m_timeout != timeout) {
        game->m_timeout = timeout;
        // TODO: Emit timeout changed signal
    }
    return newTimeout;
}

SCM delayedCall(SCM callback)
{
    auto *game = Aisleriot::instance();
    if (game->m_delayedCallTimer) {
        return scm_throw(scm_from_locale_symbol("aisleriot-invalid-call"),
                         scm_list_1(scm_from_utf8_string("Already have a delayed callback pending.")));
    }

    scm_gc_protect_object(callback);
    game->m_delayedCallTimer = new QTimer();
    QObject::connect(game->m_delayedCallTimer, &QTimer::timeout, game, [game, callback] {
        game->m_delayedCallTimer->deleteLater();
        game->m_delayedCallTimer = nullptr;

        if (game->makeSCMCall(callback, NULL, 0, NULL))
            game->testGameOver();
    });
    game->m_delayedCallTimer->start(DelayedCallDelay);
    return SCM_EOL;
}

SCM undoSetSensitive(SCM state)
{
    auto *game = Aisleriot::instance();
    game->setCanUndo(scm_is_true(state));
    return SCM_EOL;
}

SCM redoSetSensitive(SCM state)
{
    auto *game = Aisleriot::instance();
    game->setCanRedo(scm_is_true(state));
    return SCM_EOL;
}

SCM dealableSetSensitive(SCM state)
{
    auto *game = Aisleriot::instance();
    game->setCanDeal(scm_is_true(state));
    return SCM_EOL;
}

QSharedPointer<Card> createCard(SCM data)
{
    return QSharedPointer<Card>::create(!(scm_is_true(SCM_CADDR(data))),
                                        static_cast<Card::Suit>(scm_to_int(SCM_CADR(data))),
                                        static_cast<Card::Rank>(scm_to_int(SCM_CAR(data))));
}

QList<QSharedPointer<Card>> cardsFromSlot(SCM cards)
{
    // mimics aisleriot/src/game.c:cscmi_slot_set_cards
    QList<QSharedPointer<Card>> newCards;
    if (scm_is_true(scm_list_p(cards))) {
        for (SCM it = cards; it != SCM_EOL; it = SCM_CDR(it)) {
            newCards.append(createCard(it));
        }
    }
    return newCards;
}


SCM cardToSCM(QSharedPointer<Card> card)
{
    return scm_cons(scm_from_uint(card->rank()),
                    scm_cons(scm_from_uint(card->suit()),
                             scm_cons(SCM_BOOL(!card->faceDown()),
                                      SCM_EOL)));
}

SCM slotToSCM(QSharedPointer<Slot> slot)
{
    SCM cards = SCM_EOL;
    for (const QSharedPointer<Card> card : slot->cards()) {
        cards = scm_cons(cardToSCM(card), cards);
    }
    return cards;
}

SCM callLambda(void *data)
{
    Call *call = static_cast<Call *>(data);
    return scm_call_n(call->lambda, call->args, call->n);
}

inline QString getMessage(SCM message)
{
    SCM port = scm_open_output_string();
    scm_display(message, port);
    char *string = scm_to_utf8_string(scm_get_output_string(port));
    scm_dynwind_free(string);
    scm_close_output_port(port);
    return QString::fromUtf8(string);
}

SCM preUnwindHandler(void *data, SCM tag, SCM throwArgs)
{
    Q_UNUSED(tag)
    Q_UNUSED(throwArgs)
    // TODO: Get more info
    bool *error = static_cast<bool *>(data);
    *error = true;

    scm_dynwind_begin((scm_t_dynwind_flags)0);

    QString errorMessage = getMessage(throwArgs);
    QString tagName = getMessage(tag);

    qCWarning(lcScheme) << "Scheme exception occured:" << errorMessage << "in" << tagName;

    SCM stack = scm_make_stack(SCM_BOOL_T, SCM_EOL);
    if (scm_is_false(stack)) {
        qCWarning(lcScheme) << "No scheme stack trace";
    } else {
        SCM port = scm_open_output_string();
        scm_display_backtrace(stack, port, SCM_UNDEFINED, SCM_UNDEFINED);
        char *string = scm_to_utf8_string(scm_get_output_string(port));
        scm_dynwind_free(string);
        scm_close_output_port(port);
        QString backtrace = QString::fromUtf8(string);
        qCDebug(lcScheme) << "Backtrace:" << backtrace;
    }

    scm_dynwind_end();

    return SCM_UNDEFINED;
}

SCM catchHandler(void *data, SCM tag, SCM throwArgs)
{
    Q_UNUSED(data)
    Q_UNUSED(tag)
    Q_UNUSED(throwArgs)
    return SCM_UNDEFINED;
}

} // namespace

Engine::Engine()
    : m_features(NoFeatures)
    , m_generator(m_rd())
    , m_timeout(0)
{
    scm_with_guile(&init, this);
    qCDebug(lcAisleriot) << "Initialized Aisleriot Engine";
}

Engine::~Engine()
{
    m_delayedCallTimer->stop();
    delete m_delayedCallTimer;
}

bool Engine::startNewGameSCM()
{
    bool error = false;
    scm_c_catch(SCM_BOOL_T, ::startNewGameSCM, this,
                catchHandler, NULL, preUnwindHandler, &error);
    if (error) {
        qCWarning(lcAisleriot) << "A scheme error happened while starting new game";
        return false;
    }
    return true;
}

void Engine::loadGameSCM(QString gameFile)
{
    bool error = false;
    m_features = 0;
    scm_c_catch(SCM_BOOL_T, ::loadGameSCM, &gameFile,
                catchHandler, NULL, preUnwindHandler, &error);
    if (error)
        qCWarning(lcAisleriot) << "A scheme error happened while loading";
    else
        qCDebug(lcAisleriot) << "Loaded" << gameFile;
}

bool Engine::hasFeature(GameFeature feature)
{
    return (m_features & feature) != 0;
}

void Engine::updateDealable()
{
    SCM rv;
    if (hasFeature(FeatureDealable) && makeSCMCall(m_lambdas[DealableLambda], NULL, 0, &rv)) {
        setCanDeal(scm_is_true(rv));
    }
}

void Engine::undoMoveSCM()
{
    makeSCMCall(QStringLiteral("undo"), NULL, 0, NULL);
}

void Engine::redoMoveSCM()
{
    makeSCMCall(QStringLiteral("redo"), NULL, 0, NULL);
}

void Engine::endMove()
{
    makeSCMCall(QStringLiteral("end-move"), NULL, 0, NULL);
}

bool Engine::isWinningGame()
{
    SCM rv;
    return makeSCMCall(WinningGameLambda, NULL, 0, &rv) && scm_is_true(rv);
}

bool Engine::isGameOver()
{
    SCM rv;
    return makeSCMCall(GameOverLambda, NULL, 0, &rv) && scm_is_true(rv);
}


bool Engine::makeSCMCall(Lambda lambda, SCM *args, size_t n, SCM *retval)
{
    return makeSCMCall(m_lambdas[lambda], args, n, retval);
}

bool Engine::makeSCMCall(SCM lambda, SCM *args, size_t n, SCM *retval)
{
    Call call = { lambda, args, n };
    bool error = false;

    // TODO: Add error handling
    SCM r = scm_c_catch(SCM_BOOL_T, callLambda, &call,
                        catchHandler, NULL, preUnwindHandler, &error);
    if (error) {
        qCWarning(lcAisleriot) << "Scheme reported an error";
        return false;
    }

    if (retval)
        *retval = r;
    return true;
}

bool Engine::makeSCMCall(QString name, SCM *args, size_t n, SCM *retval)
{
    SCM lambda = scm_c_eval_string(name.toUtf8().data());
    if (!makeSCMCall(lambda, args, n, retval))
        return false;
    scm_remember_upto_here_1(lambda);
    return true;
}
