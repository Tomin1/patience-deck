#include <libguile.h>
#include <random>
#include <QObject>
#include <QTimer>
#include "aisleriot_scm.h"
#include "aisleriot.h"
#include "card.h"
#include "slot.h"

static const char LambdaNames[] = {
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

static const int DelayedCallDelay = 50;

AisleriotSCM::AisleriotSCM()
    : m_features(NoFeatures)
    , m_generator(m_rd())
    , m_timeout(0)
{
}

AisleriotSCM::~AisleriotSCM()
{
    m_delayedCallTimer->stop();
    delete m_delayedCallTimer;
}

SCM AisleriotSCM::setFeatureWord(SCM features)
{
    auto *game = Aisleriot::instance();
    game->m_features = scm_to_uint(features);
    return SCM_EOL;
}

SCM AisleriotSCM::getFeatureWord()
{
    auto *game = Aisleriot::instance();
    return scm_from_uint(game->m_features);
}

SCM AisleriotSCM::setStatusbarMessage(SCM newMessage)
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

SCM AisleriotSCM::resetSurface()
{
    auto *game = Aisleriot::instance();
    game->clearGame();
    return SCM_EOL;
}

SCM AisleriotSCM::addCardSlot(SCM slotData)
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

SCM AisleriotSCM::getCardSlot(SCM slotId)
{
    auto *game = Aisleriot::instance();
    QSharedPointer<Slot> slot = game->getSlot(scm_to_int(slotId));
    if (!slot)
        return SCM_EOL;

    return scm_cons(slotId, scm_cons(slotToSCM(slot), SCM_EOL));
}

SCM AisleriotSCM::setCards(SCM slotId, SCM newCards)
{
    auto *game = Aisleriot::instance();
    QSharedPointer<Slot> slot = game->getSlot(scm_to_int(slotId));
    if (!slot)
        return SCM_BOOL_F;  // TODO: Is this correct behaviour?
    slot->setCards(cardsFromSlot(newCards));

    return SCM_BOOL_T;
}

SCM AisleriotSCM::setSlotYExpansion(SCM slotId, SCM value)
{
    auto *game = Aisleriot::instance();
    QSharedPointer<Slot> slot = game->getSlot(scm_to_int(slotId));
    if (!slot)
        return SCM_EOL; // TODO: Is this correct behaviour?
    if (!slot->expandsDown())
        return SCM_EOL;
    if (slot->expandedRight())
        return SCM_EOL;
    slot->setExpansionToDown(scm_to_double(value));
    return SCM_EOL;
}

SCM AisleriotSCM::setSlotXExpansion(SCM slotId, SCM value)
{
    auto *game = Aisleriot::instance();
    QSharedPointer<Slot> slot = game->getSlot(scm_to_int(slotId));
    if (!slot)
        return SCM_EOL; // TODO: Is this correct behaviour?
    if (!slot->expandsRight())
        return SCM_EOL;
    if (slot->expandedDown())
        return SCM_EOL;
    slot->setExpansionToRight(scm_to_double(value));
    return SCM_EOL;
}

SCM AisleriotSCM::setLambda(SCM startGameLambda, SCM pressedLambda, SCM releasedLambda,
                                SCM clickedLambda, SCM doubleClickedLambda, SCM gameOverLambda,
                                SCM winningGameLambda, SCM hintLambda, SCM rest)
{
    auto *game = Aisleriot::instance();
    game->m_lambdas[NewGameLambda] = startGameLambda;
    game->m_lambdas[ButtonPressedLambda] = pressedLambda;
    game->m_lambdas[ButtonReleasedLambda] = releasedLambda;
    game->m_lambdas[ButtonClickedLambda] = clickedLambda;
    game->m_lambdas[ButtonDoubleClickedLambda] = doubleClickedLambda;
    game->m_lambdas[GameOverLambda] = gameOverLambda;
    game->m_lambdas[WinningGameLambda] = winningGameLambda;
    game->m_lambdas[HintLambda] = hintLambda;

    game->m_lambdas[GetOptionsLambda] = SCM_CAR(rest);
    rest = SCM_CDR(rest);
    game->m_lambdas[ApplyOptionsLambda] = SCM_CAR(rest);
    rest = SCM_CDR(rest);
    game->m_lambdas[TimeoutLambda] = SCM_CAR(rest);
    rest = SCM_CDR(rest);

    if (game->m_features & FeatureDroppable) {
        game->m_lambdas[DroppableLambda] = SCM_CAR(rest);
        rest = SCM_CDR(rest);
    } else {
        game->m_lambdas[DroppableLambda] = SCM_UNDEFINED;
    }

    if (game->m_features & FeatureDroppable) {
        game->m_lambdas[DealableLambda] = SCM_CAR(rest);
        rest = SCM_CDR(rest);
    } else {
        game->m_lambdas[DealableLambda] = SCM_UNDEFINED;
    }

    return SCM_EOL;
}

SCM AisleriotSCM::setLambdaX(SCM symbol, SCM lambda)
{
    auto *game = Aisleriot::instance();
    // Basically copy-paste from aisleriot/src/game.c:scm_set_lambda_x
    // TODO: maybe rewrite to use hash table instead

    const char *lambdaName = LambdaNames;
    for (int i = 0; i < LambdaCount; ++i) {
        if (scm_is_true(scm_equal_p(symbol, scm_from_locale_symbol(lambdaName)))) {
            game->m_lambdas[i] = lambda;
            return SCM_EOL;
        }

        lambdaName += strlen(lambdaName) + 1;
    }

    return scm_throw(scm_from_locale_symbol("aisleriot-invalid-call"),
                     scm_list_1(scm_from_utf8_string("Unknown lambda name in set-lambda!")));
}

SCM AisleriotSCM::myrandom(SCM range)
{
    auto *game = Aisleriot::instance();
    std::uniform_int_distribution<quint32> distribution(0, scm_to_int(range));
    return scm_from_uint32(distribution(game->m_generator));
}

SCM AisleriotSCM::clickToMoveP(void)
{
    // See aisleriot/src/game.c:scm_click_to_move_p for explanation
    return SCM_BOOL_F;
}

SCM AisleriotSCM::updateScore(SCM newScore)
{
    auto *game = Aisleriot::instance();
    char *score = scm_to_utf8_string(newScore);
    bool ok;
    int value = QString::fromUtf8(score).toInt(&ok);
    Q_ASSERT_X(ok, "updateScore", "expected an integer value");
    game->setScore(value);
    free(score);
    return newScore;
}

SCM AisleriotSCM::getTimeout(void)
{
    auto *game = Aisleriot::instance();
    return scm_from_int(game->m_timeout);
}

SCM AisleriotSCM::setTimeout(SCM newTimeout)
{
    auto *game = Aisleriot::instance();
    int timeout = scm_to_int(newTimeout);
    if (game->m_timeout != timeout) {
        game->m_timeout = timeout;
        // TODO: Emit timeout changed signal
    }
    return newTimeout;
}

SCM AisleriotSCM::delayedCall(SCM callback)
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

SCM AisleriotSCM::undoSetSensitive(SCM state)
{
    auto *game = Aisleriot::instance();
    game->setCanUndo(scm_is_true(state));
    return SCM_EOL;
}

SCM AisleriotSCM::redoSetSensitive(SCM state)
{
    auto *game = Aisleriot::instance();
    game->setCanRedo(scm_is_true(state));
    return SCM_EOL;
}

SCM AisleriotSCM::dealableSetSensitive(SCM state)
{
    auto *game = Aisleriot::instance();
    game->setCanDeal(scm_is_true(state));
    return SCM_EOL;
}

bool AisleriotSCM::makeSCMCall(SCM lambda, SCM *args, int n, SCM *retval)
{
    // TODO: Add error handling
    SCM r = scm_call_n(lambda, args, n);
    if (retval)
        *retval = r;
    return true;
}

bool AisleriotSCM::makeSCMCall(QString name, SCM *args, int n, SCM *retval)
{
    SCM lambda = scm_c_eval_string(name.toUtf8().data());
    if (!makeSCMCall(lambda, args, n, retval))
        return false;
    scm_remember_upto_here_1(lambda);
    return true;
}

bool AisleriotSCM::makeTestLambdaCall(Lambda lambda)
{
    auto *game = Aisleriot::instance();
    SCM rv = nullptr;
    return game->makeSCMCall(game->m_lambdas[lambda], NULL, 0, &rv) && scm_is_true(rv);
}

QSharedPointer<Card> AisleriotSCM::createCard(SCM data)
{
    return QSharedPointer<Card>::create(!(scm_is_true(SCM_CADDR(data))),
                                        static_cast<Card::Suit>(scm_to_int(SCM_CADR(data))),
                                        static_cast<Card::Rank>(scm_to_int(SCM_CAR(data))));
}

QList<QSharedPointer<Card>> AisleriotSCM::cardsFromSlot(SCM cards)
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


SCM AisleriotSCM::cardToSCM(QSharedPointer<Card> card)
{
    return scm_cons(scm_from_uint(card->m_rank),
                    scm_cons(scm_from_uint(card->m_suit),
                             scm_cons(SCM_BOOL(!card->m_faceDown),
                                      SCM_EOL)));
}

SCM AisleriotSCM::slotToSCM(QSharedPointer<Slot> slot)
{
    SCM cards = SCM_EOL;
    for (const QSharedPointer<Card> card : slot->m_cards) {
        cards = scm_cons(cardToSCM(card), cards);
    }
    return cards;
}
