#ifndef AISLERIOT_P_H
#define AISLERIOT_P_H

#include <QObject>
#include <QString>
#include <QVector>
#include <random>
#include <libguile.h>
#include "slot.h"

#define GAME(q, d) \
    Aisleriot *q = Aisleriot::instance(); \
    if (!q) \
        return SCM_EOL; \
    AisleriotPrivate *d = q->d_ptr

class Aisleriot;
class AisleriotPrivate
{
public:
    AisleriotPrivate();

    enum Lambdas {
        NewGameLambda,
        ButtonPressedLambda,
        ButtonReleasedLambda,
        ButtonClickedLambda,
        ButtonDoubleClickedLambda,
        GameOverLambda,
        WinningGameLambda,
        HintLambda,
        GetOptionsLambda,
        ApplyOptionsLambda,
        TimeoutLambda,
        DroppableLambda,
        DealableLambda,
        LambdaCount,
        LastMandatoryLambda = TimeoutLambda,
    };

    enum GameFeatures {
        FeatureDroppable = 0x01,
        FeatureScoreHidden = 0x02,
        FeatureDealable = 0x04,
        AllFeatures = 0x07,
    };

    enum GameState {
        UninitializedState,
        LoadedState,
        BeginState,
        RunningState,
        GameOverState,
        WonState,
        LastGameState,
    };

    static SCM setFeatureWord(SCM features);
    static SCM getFeatureWord();
    static SCM setStatusbarMessage(SCM message);
    static SCM resetSurface();
    static SCM addSlot(SCM slotData);
    static SCM getSlot(SCM slotId);
    static SCM setCards(SCM slotId, SCM newCards);
    static SCM setSlotYExpansion(SCM slotId, SCM newExpVal);
    static SCM setSlotXExpansion(SCM slotId, SCM newExpVal);
    static SCM setLambda(SCM startGameLambda, SCM pressedLambda, SCM releasedLambda,
                         SCM clickedLambda, SCM doubleClickedLambda, SCM gameOverLambda,
                         SCM winningGameLambda, SCM hintLambda, SCM rest);
    static SCM setLambdaX(SCM symbol, SCM lambda);
    static SCM myrandom(SCM range);
    static SCM clickToMoveP(void);
    static SCM updateScore(SCM newScore);
    static SCM getTimeout(void);
    static SCM setTimeout(SCM newTimeout);
    static SCM delayedCall(SCM callback);
    static SCM undoSetSensitive(SCM inState);
    static SCM redoSetSensitive(SCM inState);
    static SCM dealableSetSensitive(SCM inState);

    uint features;
    std::random_device rd;
    std::mt19937 generator;
    QString message;
    GameState state;
    QVector<Slot*> cardSlots;
    SCM lambdas[LambdaCount];
    QString score;
    int timeout;
    bool canUndo;
    bool canRedo;
    bool canDeal;
};

#endif // AISLERIOT_P_H
