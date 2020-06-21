#ifndef AISLERIOT_H
#define AISLERIOT_H

#include <QObject>

class AisleriotPrivate;
class QQmlEngine;
class QJSEngine;

class Aisleriot : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(bool canDeal READ canDeal NOTIFY canDealChanged)
    Q_PROPERTY(QString gameFile READ gameFile NOTIFY gameFileChanged)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)
    Q_PROPERTY(int state READ state NOTIFY stateChanged);

public:
    static Aisleriot* instance();
    static QObject* instance(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~Aisleriot();

    // Values
    enum GameState {
        UninitializedState,
        LoadedState,
        BeginState,
        RunningState,
        GameOverState,
        WonState,
        LastGameState,
    };
    Q_ENUM(GameState)

    // QML API
    Q_INVOKABLE void startNewGame();
    Q_INVOKABLE void restartGame();
    Q_INVOKABLE bool loadGame(QString gameFile);
    Q_INVOKABLE void undoMove();
    Q_INVOKABLE void redoMove();

    // Properties
    bool canUndo() const;
    bool canRedo() const;
    bool canDeal() const;
    QString gameFile() const;
    int score() const;
    GameState state() const;

signals:
    void canUndoChanged();
    void canRedoChanged();
    void canDealChanged();
    void gameFileChanged();
    void scoreChanged();
    void stateChanged();

private:
    friend AisleriotPrivate;

    explicit Aisleriot(QObject *parent = nullptr);
    static void interfaceInit(void *data);

    void setCanUndo(bool canUndo);
    void setCanRedo(bool canRedo);
    void setCanDeal(bool canDeal);
    void setGameFile(QString file);
    void setScore(int score);
    void setState(GameState state);

    void endMove();
    void updateDealable();
    bool winningGame();
    void testGameOver();

    static Aisleriot *s_game;
    AisleriotPrivate *d_ptr;
};

#endif // AISLERIOT_H
