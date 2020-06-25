#ifndef AISLERIOT_SCM_H
#define AISLERIOT_SCM_H

#include <QSharedPointer>
#include <QVector>
#include <random>
#include <libguile.h>

class QTimer;
class Card;
class Slot;
class AisleriotSCM
{
protected:
    AisleriotSCM();
    ~AisleriotSCM();

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

    enum GameFeature : uint {
        NoFeatures = 0x00,
        FeatureDroppable = 0x01,
        FeatureScoreHidden = 0x02,
        FeatureDealable = 0x04,
        AllFeatures = 0x07,
    };

    bool startNewGameSCM();
    bool hasFeature(GameFeature feature);
    void updateDealable();
    void endMove();
    bool isWinningGame();
    bool isGameOver();

public:
    virtual void setMessage(QString message) = 0;
    virtual void clearGame() = 0;
    virtual void addSlot(QSharedPointer<Slot> slot) = 0;
    virtual QSharedPointer<Slot> getSlot(int slot) = 0;
    virtual void setScore(int score) = 0;
    virtual void setCanUndo(bool canUndo) = 0;
    virtual void setCanRedo(bool canRedo) = 0;
    virtual void setCanDeal(bool canDeal) = 0;
    virtual void setWidth(double width) = 0;
    virtual void setHeight(double height) = 0;
    virtual void testGameOver() = 0;

private:
    static SCM startNewGameSCM(void *data);

    static SCM setFeatureWord(SCM features);
    static SCM getFeatureWord();
    static SCM setStatusbarMessage(SCM message);
    static SCM resetSurface();
    static SCM addCardSlot(SCM slotData);
    static SCM getCardSlot(SCM slotId);
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

    bool makeSCMCall(Lambda lambda, SCM *args, int n, SCM *retval);
    bool makeSCMCall(SCM lambda, SCM *args, int n, SCM *retval);
    bool makeSCMCall(QString name, SCM *args, int n, SCM *retval);

    static void interfaceInit(void *data);
    static QSharedPointer<Card> createCard(SCM data);
    static QList<QSharedPointer<Card>> cardsFromSlot(SCM cards);
    static SCM cardToSCM(QSharedPointer<Card> card);
    static SCM slotToSCM(QSharedPointer<Slot> slot);

    uint m_features;
    std::random_device m_rd;
    std::mt19937 m_generator;
    SCM m_lambdas[LambdaCount];
    QTimer *m_delayedCallTimer;
    int m_timeout;
};

#endif // AISLERIOT_SCM_H
