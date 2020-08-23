#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QVector>

class Slot;
class EnginePrivate;
class Engine : public QObject
{
    Q_OBJECT
public:
    ~Engine();

    static QSharedPointer<Engine> instance();
    static QSharedPointer<Engine> instance(const QString &loadPath);

    // The same as EnginePrivate::GameState
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

    bool load(const QString &gameFile);
    bool start();
    void undoMove();
    void redoMove();

    bool canUndo() const;
    bool canRedo() const;
    bool canDeal() const;
    GameState state() const;
    int score() const;
    QString gameFile() const;
    QString message() const;

signals:
    void canUndoChanged();
    void canRedoChanged();
    void canDealChanged();
    void stateChanged();
    void scoreChanged();
    void gameFileChanged();
    void messageChanged();

private:
    friend EnginePrivate;

    explicit Engine(const QString &loadPath, QObject *parent = nullptr);
    static QSharedPointer<Engine> s_engine;
    EnginePrivate *d_ptr;
};

#endif // ENGINE_H
