#ifndef AISLERIOT_H
#define AISLERIOT_H

#include <QObject>

class AisleriotPrivate;
class QQmlEngine;
class QJSEngine;

class Aisleriot : public QObject
{
    Q_OBJECT

public:
    static Aisleriot* instance();
    static QObject* instance(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~Aisleriot();

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


private:
    friend AisleriotPrivate;

    explicit Aisleriot(QObject *parent = nullptr);
    static void interfaceInit(void *data);

    void setDealable(bool dealable);
    void setState(GameState state);

    void endMove();
    void updateDealable();
    bool winningGame();
    void testGameOver();

    static Aisleriot *s_game;
    AisleriotPrivate *d_ptr;
};

#endif // AISLERIOT_H
