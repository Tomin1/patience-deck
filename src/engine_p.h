#ifndef ENGINE_P_H
#define ENGINE_P_H

#include <libguile.h>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>
#include <QVector>

class Slot;
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

    // TODO: Deduplicate
    enum GameState : int {
        UninitializedState,
        LoadedState,
        BeginState,
        RunningState,
        GameOverState,
        WonState,
        LastGameState,
    };
    Q_ENUM(GameState)

    explicit EnginePrivate(QObject *parent = nullptr);
    ~EnginePrivate();
    static EnginePrivate *instance();

    void updateDealable();
    void endMove();
    bool isGameOver();
    bool isWinningGame();
    void clear();
    void testGameOver();

    void setCanUndo(bool canUndo);
    void setCanRedo(bool canRedo);
    void setCanDeal(bool canDeal);
    void setScore(int score);
    GameState getState() const;
    void setState(GameState state);
    void setGameFile(QString gameFile);
    void setMessage(QString message);
    void setWidth(double width);
    void setHeight(double height);

    void addSlot(QSharedPointer<Slot> slot);
    QSharedPointer<Slot> getSlot(int slot);
    void setLambda(Lambda lambda, SCM func);
    uint getFeatures();
    void setFeatures(uint features);
    bool hasFeature(GameFeature feature);
    int getTimeout();
    void setTimeout(int timeout);

    bool makeSCMCall(Lambda lambda, SCM *args, size_t n, SCM *retval);
    bool makeSCMCall(SCM lambda, SCM *args, size_t n, SCM *retval);
    bool makeSCMCall(QString name, SCM *args, size_t n, SCM *retval);

    // TODO: Make private
    std::random_device m_rd;
    std::mt19937 m_generator;
    QTimer *m_delayedCallTimer;

private:
    friend Engine;

    QVector<QSharedPointer<Slot>> m_cardSlots;
    GameFeatures m_features;
    SCM m_lambdas[LambdaCount];
    int m_timeout;
    bool m_canUndo;
    bool m_canRedo;
    bool m_canDeal;
    GameState m_state;
    int m_score;
    QString m_gameFile;
    QString m_message;

    Engine *engine();
};

#endif // ENGINE_P_H
