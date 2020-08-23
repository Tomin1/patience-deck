#ifndef AISLERIOT_H
#define AISLERIOT_H

#include <QObject>
#include <QSharedPointer>
#include "engine.h"

class QQmlEngine;
class QJSEngine;
class Slot;
class Aisleriot : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(bool canDeal READ canDeal NOTIFY canDealChanged)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)
    Q_PROPERTY(int state READ state NOTIFY stateChanged);
    Q_PROPERTY(QString gameFile READ gameFile NOTIFY gameFileChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged);

public:
    static Aisleriot* instance();
    static QObject* instance(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~Aisleriot();

    // The same as Engine::GameState
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
    QString message() const;

signals:
    void canUndoChanged();
    void canRedoChanged();
    void canDealChanged();
    void scoreChanged();
    void stateChanged();
    void gameFileChanged();
    void messageChanged();
    void gameLoaded();

private slots:
    void catchFailure(QString message);

private:
    explicit Aisleriot(QObject *parent = nullptr);

    QSharedPointer<Engine> m_engine;

    static Aisleriot *s_game;
};

#endif // AISLERIOT_H
