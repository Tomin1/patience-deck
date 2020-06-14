#include <libguile.h>
#include <random>
#include <QJSEngine>
#include <QQmlEngine>
#include "aisleriot.h"
#include "aisleriot_p.h"
#include "card.h"
#include "slot.h"

static const char lambdaNames[] = {
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

AisleriotPrivate::AisleriotPrivate()
    : features(0)
    , generator(rd())
    , state(UninitializedState)
    , timeout(0)
    , canUndo(false)
    , canRedo(false)
    , canDeal(false)
{
}

SCM AisleriotPrivate::setFeatureWord(SCM features)
{
    GAME(game, data);
    data->features = scm_to_uint(features);
    return SCM_EOL;
}

SCM AisleriotPrivate::getFeatureWord()
{
    GAME(game, data);
    return scm_from_uint(data->features);
}

SCM AisleriotPrivate::setStatusbarMessage(SCM newMessage)
{
    GAME(game, data);
    if (!scm_is_string(newMessage))
        return SCM_EOL;

    scm_dynwind_begin((scm_t_dynwind_flags)0);

    char *message = scm_to_utf8_string(newMessage);
    scm_dynwind_free(message);

    if (message && data->message != message) {
        data->message = message;
        // TODO: Emit message changed signal
    }

    scm_dynwind_end();
    return SCM_EOL;
}

SCM AisleriotPrivate::resetSurface()
{
    GAME(game, data); Q_UNUSED(data);
    return SCM_EOL; // TODO
}

SCM AisleriotPrivate::addSlot(SCM slotData)
{
    GAME(game, data);
    if (data->state > BeginState) {
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

    Slot *slot = new Slot(scm_to_int(SCM_CAR(slotData)), type,
                          scm_to_double(SCM_CAR(SCM_CADR(SCM_CADDR(slotData)))),
                          scm_to_double(SCM_CAR(SCM_CADR(SCM_CADDR(slotData)))),
                          expansionDepth, expandedDown, expandedRight,
                          game);

    slot->setCards(SCM_CADR(slotData));

    data->cardSlots.append(slot);

    return SCM_EOL;
}

SCM AisleriotPrivate::getSlot(SCM slotId)
{
    GAME(game, data); Q_UNUSED(data);
    return SCM_EOL; // TODO
}

SCM AisleriotPrivate::setCards(SCM slotId, SCM newCards)
{
    GAME(game, data); Q_UNUSED(data);
    return SCM_EOL; // TODO
}

SCM AisleriotPrivate::setSlotYExpansion(SCM slotId, SCM newExpVal)
{
    GAME(game, data); Q_UNUSED(data);
    return SCM_EOL; // TODO
}

SCM AisleriotPrivate::setSlotXExpansion(SCM slotId, SCM newExpVal)
{
    GAME(game, data); Q_UNUSED(data);
    return SCM_EOL; // TODO
}

SCM AisleriotPrivate::setLambda(SCM startGameLambda, SCM pressedLambda, SCM releasedLambda,
                                SCM clickedLambda, SCM doubleClickedLambda, SCM gameOverLambda,
                                SCM winningGameLambda, SCM hintLambda, SCM rest)
{
    GAME(game, data);
    data->lambdas[NewGameLambda] = startGameLambda;
    data->lambdas[ButtonPressedLambda] = pressedLambda;
    data->lambdas[ButtonReleasedLambda] = releasedLambda;
    data->lambdas[ButtonClickedLambda] = clickedLambda;
    data->lambdas[ButtonDoubleClickedLambda] = doubleClickedLambda;
    data->lambdas[GameOverLambda] = gameOverLambda;
    data->lambdas[WinningGameLambda] = winningGameLambda;
    data->lambdas[HintLambda] = hintLambda;

    data->lambdas[GetOptionsLambda] = SCM_CAR(rest);
    rest = SCM_CDR(rest);
    data->lambdas[ApplyOptionsLambda] = SCM_CAR(rest);
    rest = SCM_CDR(rest);
    data->lambdas[TimeoutLambda] = SCM_CAR(rest);
    rest = SCM_CDR(rest);

    if (data->features & FeatureDroppable) {
        data->lambdas[DroppableLambda] = SCM_CAR(rest);
        rest = SCM_CDR(rest);
    } else {
        data->lambdas[DroppableLambda] = SCM_UNDEFINED;
    }

    if (data->features & FeatureDroppable) {
        data->lambdas[DealableLambda] = SCM_CAR(rest);
        rest = SCM_CDR(rest);
    } else {
        data->lambdas[DealableLambda] = SCM_UNDEFINED;
    }

    return SCM_EOL;
}

SCM AisleriotPrivate::setLambdaX(SCM symbol, SCM lambda)
{
    GAME(game, data);
    // Basically copy-paste from aisleriot/src/game.c:scm_set_lambda_x
    // TODO: maybe rewrite to use hash table instead

    const char *lambdaName = lambdaNames;
    for (int i = 0; i < LambdaCount; ++i) {
      if (scm_is_true(scm_equal_p(symbol, scm_from_locale_symbol(lambdaName)))) {
        data->lambdas[i] = lambda;
        return SCM_EOL;
      }

      lambdaName += strlen(lambdaName) + 1;
    }

    return scm_throw(scm_from_locale_symbol("aisleriot-invalid-call"),
                     scm_list_1(scm_from_utf8_string("Unknown lambda name in set-lambda!")));
}

SCM AisleriotPrivate::myrandom(SCM range)
{
    GAME(game, data);
    std::uniform_int_distribution<quint32> distribution(0, scm_to_int(range));
    return scm_from_uint32(distribution(data->generator));
}

SCM AisleriotPrivate::clickToMoveP(void)
{
    GAME(game, data); Q_UNUSED(data);
    // See aisleriot/src/game.c:scm_click_to_move_p for explanation
    return SCM_BOOL_F;
}

SCM AisleriotPrivate::updateScore(SCM newScore)
{
    GAME(game, data);
    char *score = scm_to_utf8_string(newScore);
    if (data->score != score) {
        data->score = score;
        // TODO: Emit score changed signal
    }
    free(score);
    return newScore;
}

SCM AisleriotPrivate::getTimeout(void)
{
    GAME(game, data);
    return scm_from_int(data->timeout);
}

SCM AisleriotPrivate::setTimeout(SCM newTimeout)
{
    GAME(game, data);
    int timeout = scm_to_int(newTimeout);
    if (data->timeout != timeout) {
        data->timeout = timeout;
        // TODO: Emit timeout changed signal
    }
    return newTimeout;
}

SCM AisleriotPrivate::delayedCall(SCM callback)
{
    GAME(game, data); Q_UNUSED(data);
    return SCM_EOL; // TODO
}

SCM AisleriotPrivate::undoSetSensitive(SCM inState)
{
    GAME(game, data);
    bool state = scm_is_true(inState);
    if (state != data->canUndo) {
        data->canUndo = state;
        // TODO: Emit canUndo changed
    }
    return SCM_EOL;
}

SCM AisleriotPrivate::redoSetSensitive(SCM inState)
{
    GAME(game, data);
    bool state = scm_is_true(inState);
    if (state != data->canRedo) {
        data->canRedo = state;
        // TODO: Emit canRedo changed
    }
    return SCM_EOL;
}

SCM AisleriotPrivate::dealableSetSensitive(SCM inState)
{
    GAME(game, data);
    bool state = scm_is_true(inState);
    if (state != data->canDeal) {
        data->canDeal = state;
        // TODO: Emit canDeal changed
    }
    return SCM_EOL;
}

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
    , d_ptr(new AisleriotPrivate)
{
    scm_c_define_module("aisleriot interface", interfaceInit, this);
}

Aisleriot::~Aisleriot()
{
    delete d_ptr;
}

void Aisleriot::interfaceInit(void *data)
{
    // See aisleriot/src/game.c:cscm_init for reference
    Q_UNUSED(data)
    /* List all C functions that Aisleriot can call */
    scm_c_define_gsubr("set-feature-word!", 1, 0, 0, (void *)AisleriotPrivate::setFeatureWord);
    scm_c_define_gsubr("get-feature-word", 0, 0, 0, (void *)AisleriotPrivate::getFeatureWord);
    scm_c_define_gsubr("set-statusbar-message-c", 1, 0, 0, (void *)AisleriotPrivate::setStatusbarMessage);
    scm_c_define_gsubr("reset-surface", 0, 0, 0, (void *)AisleriotPrivate::resetSurface);
    scm_c_define_gsubr("add-slot", 1, 0, 0, (void *)AisleriotPrivate::addSlot);
    scm_c_define_gsubr("get-slot", 1, 0, 0, (void *)AisleriotPrivate::getSlot);
    scm_c_define_gsubr("set-cards-c!", 2, 0, 0, (void *)AisleriotPrivate::setCards);
    scm_c_define_gsubr("set-slot-y-expansion!", 2, 0, 0, (void *)AisleriotPrivate::setSlotYExpansion);
    scm_c_define_gsubr("set-slot-x-expansion!", 2, 0, 0, (void *)AisleriotPrivate::setSlotXExpansion);
    scm_c_define_gsubr("set-lambda", 8, 0, 1, (void *)AisleriotPrivate::setLambda);
    scm_c_define_gsubr("set-lambda!", 2, 0, 0, (void *)AisleriotPrivate::setLambdaX);
    scm_c_define_gsubr("aisleriot-random", 1, 0, 0, (void *)AisleriotPrivate::myrandom);
    scm_c_define_gsubr("click-to-move?", 0, 0, 0, (void *)AisleriotPrivate::clickToMoveP);
    scm_c_define_gsubr("update-score", 1, 0, 0, (void *)AisleriotPrivate::updateScore);
    scm_c_define_gsubr("get-timeout", 0, 0, 0, (void *)AisleriotPrivate::getTimeout);
    scm_c_define_gsubr("set-timeout!", 1, 0, 0, (void *)AisleriotPrivate::setTimeout);
    scm_c_define_gsubr("delayed-call", 1, 0, 0, (void *)AisleriotPrivate::delayedCall);
    scm_c_define_gsubr("undo-set-sensitive", 1, 0, 0, (void *)AisleriotPrivate::undoSetSensitive);
    scm_c_define_gsubr("redo-set-sensitive", 1, 0, 0, (void *)AisleriotPrivate::redoSetSensitive);
    scm_c_define_gsubr("dealable-set-sensitive", 1, 0, 0, (void *)AisleriotPrivate::dealableSetSensitive);

    scm_c_export("set-feature-word!", "get-feature-word", "set-statusbar-message-c",
                 "reset-surface", "add-slot", "get-slot", "set-cards-c!",
                 "set-slot-y-expansion!", "set-slot-x-expansion!",
                 "set-lambda", "set-lambda!", "aisleriot-random",
                 "click-to-move?", "update-score", "get-timeout",
                 "set-timeout!", "delayed-call", "undo-set-sensitive",
                 "redo-set-sensitive", "dealable-set-sensitive", NULL);
}
