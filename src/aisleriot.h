#ifndef AISLERIOT_H
#define AISLERIOT_H

#include <QObject>
#include <QThread>
#include "engine.h"

class QQmlEngine;
class QJSEngine;
class QThread;
class Aisleriot : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(bool canDeal READ canDeal NOTIFY canDealChanged)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)
    Q_PROPERTY(int state READ state NOTIFY stateChanged);
    Q_PROPERTY(QString gameName READ gameName NOTIFY gameNameChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged);

public:
    static Aisleriot* instance();
    static QObject* instance(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~Aisleriot();

    enum GameState {
        UninitializedState,
        LoadedState,
        RunningState,
        GameOverState,
        WonState,
    };
    Q_ENUM(GameState);

    // QML API
    Q_INVOKABLE void startNewGame();
    Q_INVOKABLE void restartGame();
    Q_INVOKABLE void loadGame(const QString &gameFile);
    Q_INVOKABLE void undoMove();
    Q_INVOKABLE void redoMove();

    // Properties
    bool canUndo() const;
    bool canRedo() const;
    bool canDeal() const;
    QString gameName() const;
    int score() const;
    GameState state() const;
    QString message() const;

signals:
    void canUndoChanged();
    void canRedoChanged();
    void canDealChanged();
    void scoreChanged();
    void stateChanged();
    void gameNameChanged();
    void messageChanged();

    void doStart();
    void doRestart();
    void doLoad(const QString &gameFile);
    void doUndoMove();
    void doRedoMove();

private slots:
    void catchFailure(QString message);
    void handleGameLoaded(const QString &gameFile);
    void handleGameStarted();
    void handleGameOver(bool won);
    void handleCanUndoChanged(bool canUndo);
    void handleCanRedoChanged(bool canRedo);
    void handleCanDealChanged(bool canDeal);
    void handleScoreChanged(int score);
    void handleMessageChanged(const QString &message);

private:
    explicit Aisleriot(QObject *parent = nullptr);
    void setState(GameState state);

    QThread m_engineThread;
    bool m_canUndo;
    bool m_canRedo;
    bool m_canDeal;
    int m_score;
    GameState m_state;
    QString m_gameFile;
    QString m_message;

    static Aisleriot *s_game;
};

#endif // AISLERIOT_H
