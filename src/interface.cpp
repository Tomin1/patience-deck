#include "enginedata.h"
#include "engine_p.h"
#include "interface.h"
#include "logging.h"

void Interface::init_module(void* data)
{
    Q_UNUSED(data)
    scm_c_define_gsubr("set-feature-word!", 1, 0, 0, (void *)&setFeatureWord);
    scm_c_define_gsubr("get-feature-word", 0, 0, 0, (void *)&getFeatureWord);
    scm_c_define_gsubr("set-statusbar-message-c", 1, 0, 0, (void *)&setStatusbarMessage);
    scm_c_define_gsubr("reset-surface", 0, 0, 0, (void *)&resetSurface);
    scm_c_define_gsubr("add-slot", 1, 0, 0, (void *)&addCardSlot);
    scm_c_define_gsubr("get-slot", 1, 0, 0, (void *)&getCardSlot);
    scm_c_define_gsubr("set-cards-c!", 2, 0, 0, (void *)&setCards);
    scm_c_define_gsubr("set-slot-y-expansion!", 2, 0, 0, (void *)&setSlotYExpansion);
    scm_c_define_gsubr("set-slot-x-expansion!", 2, 0, 0, (void *)&setSlotXExpansion);
    scm_c_define_gsubr("set-lambda", 8, 0, 1, (void *)&setLambda);
    scm_c_define_gsubr("set-lambda!", 2, 0, 0, (void *)&setLambdaX);
    scm_c_define_gsubr("aisleriot-random", 1, 0, 0, (void *)&myrandom);
    scm_c_define_gsubr("click-to-move?", 0, 0, 0, (void *)&clickToMoveP);
    scm_c_define_gsubr("update-score", 1, 0, 0, (void *)&updateScore);
    scm_c_define_gsubr("get-timeout", 0, 0, 0, (void *)&getTimeout);
    scm_c_define_gsubr("set-timeout!", 1, 0, 0, (void *)&setTimeout);
    scm_c_define_gsubr("delayed-call", 1, 0, 0, (void *)&delayedCall);
    scm_c_define_gsubr("undo-set-sensitive", 1, 0, 0, (void *)&undoSetSensitive);
    scm_c_define_gsubr("redo-set-sensitive", 1, 0, 0, (void *)&redoSetSensitive);
    scm_c_define_gsubr("dealable-set-sensitive", 1, 0, 0, (void *)&dealableSetSensitive);

    scm_c_export("set-feature-word!", "get-feature-word", "set-statusbar-message-c",
                 "reset-surface", "add-slot", "get-slot", "set-cards-c!",
                 "set-slot-y-expansion!", "set-slot-x-expansion!",
                 "set-lambda", "set-lambda!", "aisleriot-random",
                 "click-to-move?", "update-score", "get-timeout",
                 "set-timeout!", "delayed-call", "undo-set-sensitive",
                 "redo-set-sensitive", "dealable-set-sensitive", nullptr);

    qCInfo(lcScheme) << "Initialized aisleriot interface";
}

SCM Scheme::preUnwindHandler(void *data, SCM tag, SCM throwArgs)
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
        qCWarning(lcScheme) << "No scheme stack";
    } else {
        SCM port = scm_open_output_string();
        scm_display_backtrace(stack, port, SCM_UNDEFINED, SCM_UNDEFINED);
        char *string = scm_to_utf8_string(scm_get_output_string(port));
        scm_dynwind_free(string);
        scm_close_output_port(port);
        qCDebug(lcScheme) << "Backtrace:";
        for (const QString &line : QString::fromUtf8(string).split("\n")) {
            qCDebug(lcScheme) << line;
        }
    }

    scm_dynwind_end();

    return SCM_UNDEFINED;
}

SCM Scheme::catchHandler(void *data, SCM tag, SCM throwArgs)
{
    Q_UNUSED(tag)
    Q_UNUSED(throwArgs)
    bool *error = static_cast<bool *>(data);
    *error = true;
    return SCM_UNDEFINED;
}

inline QString Scheme::getMessage(SCM message)
{
    SCM port = scm_open_output_string();
    scm_display(message, port);
    char *string = scm_to_utf8_string(scm_get_output_string(port));
    scm_dynwind_free(string);
    scm_close_output_port(port);
    return QString::fromUtf8(string);
}

const CardData Scheme::createCard(SCM data)
{
    return {
        Suit(scm_to_int(SCM_CADR(data))),
        Rank(scm_to_int(SCM_CAR(data))),
        scm_is_true(SCM_CADDR(data))
    };
}

CardList Scheme::cardsFromSlot(SCM cards)
{
    // mimics aisleriot/src/game.c:cscmi_slot_set_cards
    CardList newCards;
    if (scm_is_true(scm_list_p(cards))) {
        for (SCM it = cards; it != SCM_EOL; it = SCM_CDR(it)) {
            newCards.insert(0, createCard(SCM_CAR(it)));
        }
    }
    return newCards;
}

SCM Scheme::cardToSCM(const CardData &card)
{
    return scm_cons(
        scm_from_uint(card.rank),
        scm_cons(
            scm_from_uint(card.suit),
            scm_cons(
                SCM_BOOL(card.show),
                SCM_EOL
            )
        )
    );
}

SCM Scheme::slotToSCM(const CardList &slot)
{
    SCM cards = SCM_EOL;
    for (const CardData &card : slot) {
        cards = scm_cons(cardToSCM(card), cards);
    }
    return cards;
}

SCM Scheme::startNewGame(void *data)
{
    EnginePrivate *engine = static_cast<EnginePrivate *>(data);
    // TODO: Deal with game over situations
    SCM size = SCM_UNDEFINED;
    engine->makeSCMCall(EnginePrivate::NewGameLambda, nullptr, 0, &size);
    engine->setWidth(scm_to_double(SCM_CAR(size)));
    engine->setHeight(scm_to_double(SCM_CADR(size)));
    scm_remember_upto_here_1(size);

    engine->makeSCMCall(QStringLiteral("start-game"), nullptr, 0, nullptr);

    if (engine->hasFeature(EnginePrivate::FeatureDealable)) {
        SCM rv;
        engine->makeSCMCall(EnginePrivate::DealableLambda, nullptr, 0, &rv);
        engine->setCanDeal(scm_is_true(rv));
    }

    return SCM_BOOL_T;
}

SCM Scheme::loadGameFromFile(void *data)
{
    scm_dynwind_begin((scm_t_dynwind_flags)0);
    QByteArray file(static_cast<const QString *>(data)->toUtf8());
    scm_primitive_load_path(scm_from_utf8_string(file.constData()));
    // TODO: Test all lambdas
    scm_dynwind_end();
    return SCM_BOOL_T;
}

SCM Scheme::callLambda(void *data)
{
    Interface::Call *call = static_cast<Interface::Call *>(data);
    return scm_call_n(call->lambda, call->args, call->n);
}

void *Interface::init(void *data)
{
    const QString *loadPath = static_cast<const QString *>(data);
    SCM var;
    var = scm_c_module_lookup(scm_the_root_module(), "%load-path");
    scm_variable_set_x(var,
        scm_append_x(scm_list_2(
                scm_variable_ref(var),
                scm_list_1(scm_from_utf8_string(loadPath->toUtf8().constData()))
        ))
    );
    scm_c_define_module("aisleriot interface", init_module, nullptr);
    return SCM_UNDEFINED;
}

SCM Interface::setFeatureWord(SCM features)
{
    auto *engine = EnginePrivate::instance();
    engine->setFeatures(scm_to_uint(features));
    qCDebug(lcScheme) << "Set features to" << EnginePrivate::GameFeatures(engine->getFeatures());
    return SCM_EOL;
}

SCM Interface::getFeatureWord()
{
    auto *engine = EnginePrivate::instance();
    return scm_from_uint(engine->getFeatures());
}

SCM Interface::setStatusbarMessage(SCM newMessage)
{
    auto *engine = EnginePrivate::instance();
    if (!scm_is_string(newMessage))
        return SCM_EOL;

    scm_dynwind_begin((scm_t_dynwind_flags)0);

    char *message = scm_to_utf8_string(newMessage);
    scm_dynwind_free(message);

    engine->setMessage(QString::fromUtf8(message));

    scm_dynwind_end();
    return SCM_EOL;
}

SCM Interface::resetSurface()
{
    auto *engine = EnginePrivate::instance();
    qCDebug(lcScheme) << "Reseting surface";
    engine->clear();
    return SCM_EOL;
}

SCM Interface::addCardSlot(SCM slotData)
{
    auto *engine = EnginePrivate::instance();
    if (engine->isInitialized()) {
        return scm_throw(scm_from_locale_symbol("aisleriot-invalid-call"),
                         scm_list_1(scm_from_utf8_string("Cannot add a new slot after the game has started.")));
    }

    // Basically copy-paste from aisleriot/src/game.c:cscmi_add_slot
#define EQUALS_SYMBOL(string, object) (scm_is_true(scm_equal_p(scm_from_locale_symbol(string), object)))

    SCM slotPlacement = SCM_CADDR(slotData);
    bool expandedDown = false, expandedRight = false;
    int expansionDepth = Expansion::None;
    if (EQUALS_SYMBOL("expanded", SCM_CAR(slotPlacement))) {
        expandedDown = true;
        expansionDepth = Expansion::Full;
    } else if (EQUALS_SYMBOL("expanded-right", SCM_CAR(slotPlacement))) {
        expandedRight = true;
        expansionDepth = Expansion::Full;
    } else if (EQUALS_SYMBOL("partially-expanded", SCM_CAR(slotPlacement))) {
        expandedDown = true;
        expansionDepth = scm_to_int(SCM_CADDR(slotPlacement));
    } else if (EQUALS_SYMBOL("partially-expanded-right", SCM_CAR(slotPlacement))) {
        expandedRight = true;
        expansionDepth = scm_to_int(SCM_CADDR(slotPlacement));
    }

    /* 3rd argument is the slot type (optionally) */
    SCM slotType = SCM_CDDDR(slotData);
    SlotType type = UnknownSlot;
    if (slotType == SCM_EOL) {
        // Optional
    } else if (EQUALS_SYMBOL("chooser", SCM_CAR(slotType))) {
        type = ChooserSlot;
    } else if (EQUALS_SYMBOL("foundation", SCM_CAR(slotType))) {
        type = FoundationSlot;
    } else if (EQUALS_SYMBOL("reserve", SCM_CAR(slotType))) {
        type = ReserveSlot;
    } else if (EQUALS_SYMBOL("stock", SCM_CAR(slotType))) {
        type = StockSlot;
    } else if (EQUALS_SYMBOL("tableau", SCM_CAR(slotType))) {
        type = TableauSlot;
    } else if (EQUALS_SYMBOL("waste", SCM_CAR(slotType))) {
        type = WasteSlot;
    }
#undef EQUALS_SYMBOL

    int id = scm_to_int(SCM_CAR(slotData));
    double x = scm_to_double(SCM_CAR(SCM_CADR(SCM_CADDR(slotData))));
    double y = scm_to_double(SCM_CADR(SCM_CADR(SCM_CADDR(slotData))));
    auto cards = Scheme::cardsFromSlot(SCM_CADR(slotData));
    engine->addSlot(id, cards, type, x, y, expansionDepth, expandedDown, expandedRight);

    return SCM_EOL;
}

SCM Interface::getCardSlot(SCM slotId)
{
    auto *engine = EnginePrivate::instance();
    const CardList slot = engine->getSlot(scm_to_int(slotId));
    return scm_cons(slotId, scm_cons(Scheme::slotToSCM(slot), SCM_EOL));
}

SCM Interface::setCards(SCM slotId, SCM newCards)
{
    auto *engine = EnginePrivate::instance();
    engine->setCards(scm_to_int(slotId), Scheme::cardsFromSlot(newCards));
    return SCM_BOOL_T;
}

SCM Interface::setSlotYExpansion(SCM slotId, SCM value)
{
    auto *engine = EnginePrivate::instance();
    engine->setExpansionToDown(scm_to_int(slotId), scm_to_double(value));
    return SCM_EOL;
}

SCM Interface::setSlotXExpansion(SCM slotId, SCM value)
{
    auto *engine = EnginePrivate::instance();
    engine->setExpansionToRight(scm_to_int(slotId), scm_to_double(value));
    return SCM_EOL;
}

SCM Interface::setLambda(SCM startGameLambda, SCM pressedLambda, SCM releasedLambda,
                         SCM clickedLambda, SCM doubleClickedLambda, SCM gameOverLambda,
                         SCM winningGameLambda, SCM hintLambda, SCM rest)
{
    auto *engine = EnginePrivate::instance();
    qCDebug(lcScheme) << "Setting all lambdas";
    engine->setLambda(EnginePrivate::NewGameLambda, startGameLambda);
    engine->setLambda(EnginePrivate::ButtonPressedLambda, pressedLambda);
    engine->setLambda(EnginePrivate::ButtonReleasedLambda, releasedLambda);
    engine->setLambda(EnginePrivate::ButtonClickedLambda, clickedLambda);
    engine->setLambda(EnginePrivate::ButtonDoubleClickedLambda, doubleClickedLambda);
    engine->setLambda(EnginePrivate::GameOverLambda, gameOverLambda);
    engine->setLambda(EnginePrivate::WinningGameLambda, winningGameLambda);
    engine->setLambda(EnginePrivate::HintLambda, hintLambda);

    engine->setLambda(EnginePrivate::GetOptionsLambda, SCM_CAR(rest));
    rest = SCM_CDR(rest);
    engine->setLambda(EnginePrivate::ApplyOptionsLambda, SCM_CAR(rest));
    rest = SCM_CDR(rest);
    engine->setLambda(EnginePrivate::TimeoutLambda, SCM_CAR(rest));
    rest = SCM_CDR(rest);

    if (engine->hasFeature(EnginePrivate::FeatureDroppable)) {
        engine->setLambda(EnginePrivate::DroppableLambda, SCM_CAR(rest));
        rest = SCM_CDR(rest);
    } else {
        engine->setLambda(EnginePrivate::DroppableLambda, SCM_UNDEFINED);
    }

    if (engine->hasFeature(EnginePrivate::FeatureDealable)) {
        engine->setLambda(EnginePrivate::DealableLambda, SCM_CAR(rest));
        rest = SCM_CDR(rest);
    } else {
        engine->setLambda(EnginePrivate::DealableLambda, SCM_UNDEFINED);
    }

    return SCM_EOL;
}

SCM Interface::setLambdaX(SCM symbol, SCM lambda)
{
    qCDebug(lcScheme) << "Setting a lambda";
    auto *engine = EnginePrivate::instance();
    // Basically copy-paste from aisleriot/src/game.c:scm_set_lambda_x
    // TODO: maybe rewrite to use hash table instead

    const char *lambdaName = LambdaNames;
    for (int i = 0; i < EnginePrivate::LambdaCount; ++i) {
        if (scm_is_true(scm_equal_p(symbol, scm_from_locale_symbol(lambdaName)))) {
            engine->setLambda(static_cast<EnginePrivate::Lambda>(i), lambda);
            return SCM_EOL;
        }

        lambdaName += strlen(lambdaName) + 1;
    }

    return scm_throw(scm_from_locale_symbol("aisleriot-invalid-call"),
                     scm_list_1(scm_from_utf8_string("Unknown lambda name in set-lambda!")));
}

SCM Interface::myrandom(SCM range)
{
    // TODO: Needs to be reimplemented
    auto *engine = EnginePrivate::instance();
    std::uniform_int_distribution<quint32> distribution(0, scm_to_int(range)-1);
    return scm_from_uint32(distribution(engine->m_generator));
}

SCM Interface::clickToMoveP(void)
{
    // See aisleriot/src/game.c:scm_click_to_move_p for explanation
    return SCM_BOOL_F;
}

SCM Interface::updateScore(SCM newScore)
{
    auto *engine = EnginePrivate::instance();
    char *score = scm_to_utf8_string(newScore);
    bool ok;
    int value = QString::fromUtf8(score).toInt(&ok);
    if (!ok) {
        engine->die("expected an integer value");
        return 0;
    }
    engine->setScore(value);
    qCDebug(lcScheme) << "Set score to" << value;
    free(score);
    return newScore;
}

SCM Interface::getTimeout(void)
{
    auto *engine = EnginePrivate::instance();
    return scm_from_int(engine->getTimeout());
}

SCM Interface::setTimeout(SCM newTimeout)
{
    auto *engine = EnginePrivate::instance();
    engine->setTimeout(scm_to_int(newTimeout));
    qCDebug(lcScheme) << "Set timeout to" << engine->getTimeout();
    return newTimeout;
}

SCM Interface::delayedCall(SCM callback)
{
    auto *engine = EnginePrivate::instance();
    qCDebug(lcScheme) << "Creating delayed call";
    if (engine->m_delayedCallTimer) {
        return scm_throw(scm_from_locale_symbol("aisleriot-invalid-call"),
                         scm_list_1(scm_from_utf8_string("Already have a delayed callback pending.")));
    }

    scm_gc_protect_object(callback);
    engine->m_delayedCallTimer = new QTimer();
    QObject::connect(engine->m_delayedCallTimer, &QTimer::timeout, engine, [engine, callback] {
        engine->m_delayedCallTimer->deleteLater();
        engine->m_delayedCallTimer = nullptr;

        if (engine->makeSCMCall(callback, nullptr, 0, nullptr))
            engine->testGameOver();
    });
    engine->m_delayedCallTimer->start(DelayedCallDelay);
    return SCM_EOL;
}

SCM Interface::undoSetSensitive(SCM state)
{
    auto *engine = EnginePrivate::instance();
    engine->setCanUndo(scm_is_true(state));
    return SCM_EOL;
}

SCM Interface::redoSetSensitive(SCM state)
{
    auto *engine = EnginePrivate::instance();
    engine->setCanRedo(scm_is_true(state));
    return SCM_EOL;
}

SCM Interface::dealableSetSensitive(SCM state)
{
    auto *engine = EnginePrivate::instance();
    engine->setCanDeal(scm_is_true(state));
    return SCM_EOL;
}
