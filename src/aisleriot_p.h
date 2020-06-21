#ifndef AISLERIOT_P_H
#define AISLERIOT_P_H

#include <QObject>
#include <QString>
#include <QVector>
#include <random>
#include <libguile.h>
#include "aisleriot.h"

#define GAME(q, d, err) \
    Aisleriot *q = Aisleriot::instance(); \
    if (!q) \
        return err; \
    AisleriotPrivate *d = q->d_ptr

class QTimer;
class Slot;

class AisleriotPrivate : QObject
{
    Q_OBJECT

public:
    explicit AisleriotPrivate(QObject *parent = nullptr);

    enum Lambda {
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
    Q_ENUM(Lambda)

    enum GameFeature : uint {
        NoFeatures = 0x00,
        FeatureDroppable = 0x01,
        FeatureScoreHidden = 0x02,
        FeatureDealable = 0x04,
        AllFeatures = 0x07,
    };
    Q_ENUM(GameFeature)
    Q_DECLARE_FLAGS(GameFeatures, GameFeature)

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
    static SCM undoSetSensitive(SCM state);
    static SCM redoSetSensitive(SCM state);
    static SCM dealableSetSensitive(SCM state);

    bool makeSCMCall(SCM lambda, SCM *args, int n, SCM *retval);
    bool makeSCMCall(QString name, SCM *args, int n, SCM *retval);
    bool makeTestLambdaCall(Lambda lambda);

public slots:
    static void runDelayedCallback(SCM callback);

public:
    bool canUndo;
    bool canRedo;
    bool canDeal;
    QString gameFile;
    Aisleriot::GameState state;
    int score;

    GameFeatures features;
    std::random_device rd;
    std::mt19937 generator;
    QString message;
    QVector<Slot*> cardSlots;
    SCM lambdas[LambdaCount];
    QTimer *delayedCallTimer;
    int timeout;
};

#endif // AISLERIOT_P_H
