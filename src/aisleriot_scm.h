#ifndef AISLERIOT_SCM_H
#define AISLERIOT_SCM_H

#include <QSharedPointer>
#include <QVector>
#include <random>
#include <libguile.h>

class QTimer;
class Card;
class Slot;
class Engine
{
protected:
    Engine();
    ~Engine();

public:
    // TODO: Make internal to engine
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

    // TODO: Make internal to engine
    enum GameFeature : uint {
        NoFeatures = 0x00,
        FeatureDroppable = 0x01,
        FeatureScoreHidden = 0x02,
        FeatureDealable = 0x04,
        AllFeatures = 0x07,
    };

    bool startNewGameSCM();
    void loadGameSCM(QString gameFile);
    bool hasFeature(GameFeature feature);
    void updateDealable();
    void undoMoveSCM();
    void redoMoveSCM();
    void endMove();
    bool isWinningGame();
    bool isGameOver();

    // TODO: Make these signals
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

    // TODO: Make these internal to engine
    bool makeSCMCall(Lambda lambda, SCM *args, size_t n, SCM *retval);
    bool makeSCMCall(SCM lambda, SCM *args, size_t n, SCM *retval);
    bool makeSCMCall(QString name, SCM *args, size_t n, SCM *retval);

    // TODO: Protect these better
    uint m_features;
    std::random_device m_rd;
    std::mt19937 m_generator;
    SCM m_lambdas[LambdaCount];
    QTimer *m_delayedCallTimer;
    int m_timeout;
};

#endif // AISLERIOT_SCM_H
