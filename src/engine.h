#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QString>
#include "enginedata.h"

class EnginePrivate;
class Engine : public QObject
{
    Q_OBJECT
public:
    ~Engine();

    static Engine *instance();

public slots:
    void init();
    void load(const QString &gameFile);
    void start();
    void restart();
    void undoMove();
    void redoMove();

signals:
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void canDealChanged(bool canDeal);
    void scoreChanged(int score);
    void messageChanged(const QString &message);

    void engineFailure(QString message);
    void gameLoaded(const QString &gameFile);
    void gameStarted();
    void gameOver(bool won);

    void newSlot(int id, int type, double x, double y,
                 int expansionDepth, bool expandedDown, bool expandedRight);
    void setExpansionToDown(int id, double expansion);
    void setExpansionToRight(int id, double expansion);
    void clearSlot(int id);
    void newCard(int slotId, int suit, int rank, bool show);
    void clearData();
    void widthChanged(double width);
    void heightChanged(double height);

private:
    friend EnginePrivate;

    explicit Engine(QObject *parent = nullptr);
    static Engine *s_engine;
    EnginePrivate *d_ptr;
};

#endif // ENGINE_H
