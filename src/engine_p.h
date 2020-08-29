#ifndef ENGINE_P_H
#define ENGINE_P_H

#include <libguile.h>
#include <QHash>
#include <QList>
#include <QObject>
#include <QTimer>
#include "enginedata.h"

class Engine;
class EnginePrivate : public QObject
{
    Q_OBJECT

public:
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
    Q_ENUM(GameFeature);
    Q_DECLARE_FLAGS(GameFeatures, GameFeature);

    enum GameState : int {
        UninitializedState,
        LoadedState,
        BeginState,
        RunningState,
        GameOverState,
    };
    Q_ENUM(GameState);

    struct Card {
        Suit suit;
        Rank rank;
        bool faceDown;
    };

    explicit EnginePrivate(QObject *parent = nullptr);
    ~EnginePrivate();
    static EnginePrivate *instance();

    void updateDealable();
    void endMove();
    bool isGameOver();
    bool isWinningGame();
    bool isInitialized();
    void clear();
    void testGameOver();

    void setCanUndo(bool canUndo);
    void setCanRedo(bool canRedo);
    void setCanDeal(bool canDeal);
    void setScore(int score);
    void setMessage(QString message);
    void setWidth(double width);
    void setHeight(double height);

    void addSlot(int id, QList<Card> cards, SlotType type, double x, double y,
                 int expansionDepth, bool expandedDown, bool expandedRight);
    const QList<Card> getSlot(int slot);
    void setCards(int id, QList<Card> cards);
    void setExpansionToDown(int id, double expansion);
    void setExpansionToRight(int id, double expansion);
    void setLambda(Lambda lambda, SCM func);
    uint getFeatures();
    void setFeatures(uint features);
    bool hasFeature(GameFeature feature);
    int getTimeout();
    void setTimeout(int timeout);
    void die(const char *message);

    bool makeSCMCall(Lambda lambda, SCM *args, size_t n, SCM *retval);
    bool makeSCMCall(SCM lambda, SCM *args, size_t n, SCM *retval);
    bool makeSCMCall(QString name, SCM *args, size_t n, SCM *retval);

    // TODO: Make private
    std::random_device m_rd;
    std::mt19937 m_generator;
    QTimer *m_delayedCallTimer;

private:
    friend Engine;

    QHash<int, QList<Card>> m_cardSlots;
    SCM m_lambdas[LambdaCount];
    GameFeatures m_features;
    GameState m_state;
    int m_timeout;
    bool m_canUndo;
    bool m_canRedo;
    bool m_canDeal;

    Engine *engine();
};

#endif // ENGINE_P_H
