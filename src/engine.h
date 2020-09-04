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
    void checkDrag(int slotId, const CardList &cards);
    void checkDrop(int startSlotId, int endSlotId, const CardList &cards);
    void drop(int startSlotId, int endSlotId, const CardList &cards);

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

    void newSlot(int id, const CardList &cards, int type, double x, double y,
                 int expansionDepth, bool expandedDown, bool expandedRight);
    void setExpansionToDown(int id, double expansion);
    void setExpansionToRight(int id, double expansion);
    void insertCard(int slotId, int index, const CardData &card);
    void appendCard(int slotId, const CardData &card);
    void removeCard(int slotId, int index);
    void clearData();
    void widthChanged(double width);
    void heightChanged(double height);

    void couldDrag(bool could, int slotId, const CardList &cards);
    void couldDrop(bool could, int startSlotId, int endSlotId, const CardList &cards);
    void dropped(bool could, int startSlotId, int endSlotId, const CardList &cards);

private:
    friend EnginePrivate;

    explicit Engine(QObject *parent = nullptr);
    static Engine *s_engine;
    EnginePrivate *d_ptr;
};

#endif // ENGINE_H
