/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021 Tomi Leppänen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENGINE_H
#define ENGINE_H

#ifndef ENGINE_EXERCISER
#include <MGConfItem>
#endif

#include <QObject>
#include <QString>
#include "enginedata.h"

class EngineHelper;
class EnginePrivate;
class Engine : public QObject
{
    Q_OBJECT
public:
    ~Engine();

    static Engine *instance();

    enum ActionType {
        InsertionAction,
        RemovalAction,
        FlippingAction,
        ClearingAction,
    };
    Q_ENUM(ActionType)

public slots:
    void init();
    void initWithDirectory(const QString &gameDirectory);
    void load(const QString &gameFile);
    void start();
    void restart();
    void undoMove();
    void redoMove();
    void dealCard();
    void getHint();
    bool drag(quint32 id, int slotId, const CardList &cards);
    void cancelDrag(quint32 id, int slotId, const CardList &cards);
    bool checkDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    bool drop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    bool click(quint32 id, int slotId);
    bool doubleClick(quint32 id, int slotId);
    void requestGameOptions();
    bool setGameOption(const GameOption &option);
    bool setGameOptions(const GameOptionList &options);
#ifndef ENGINE_EXERCISER
    void saveState();
    void resetSavedState();
    void restoreSavedState();
#endif // ENGINE_EXERCISER

signals:
    void canUndo(bool canUndo);
    void canRedo(bool canRedo);
    void canDeal(bool canDeal);
    void score(int score);
    void message(const QString &message);
    void hint(const QString &hint);

    void engineFailure(QString message);
    void gameLoaded(const QString &gameFile);
    void gameStarted();
    void restoreCompleted(bool success);
    void gameOver(bool won);
    void gameOptions(GameOptionList options);

    void showScore(bool show);
    void showDeal(bool show);

    void newSlot(int id, const CardList &cards, int type, double x, double y,
                 int expansionDepth, bool expandedDown, bool expandedRight);
    void setExpansionToDown(int id, double expansion);
    void setExpansionToRight(int id, double expansion);
    void action(Engine::ActionType action, int slotId, int index, const CardData &card);
    void clearData();
    void widthChanged(double width);
    void heightChanged(double height);

    void couldDrag(quint32 id, int slotId, bool could);
    void couldDrop(quint32 id, int slotId, bool could);
    void dropped(quint32 id, int slotId, bool could);
    void clicked(quint32 id, int slotId, bool could);
    void doubleClicked(quint32 id, int slotId, bool could);

    void moveEnded();

private:
    friend EnginePrivate;
#ifdef ENGINE_EXERCISER
    friend EngineHelper;
#endif

    void loadGame(const QString &gameFile, bool restored);
    void startEngine(bool newSeed);

    explicit Engine(QObject *parent = nullptr);
    static Engine *s_engine;
    EnginePrivate *d_ptr;
#ifndef ENGINE_EXERCISER
    MGConfItem m_stateConf;
#endif // ENGINE_EXERCISER
};

#endif // ENGINE_H
