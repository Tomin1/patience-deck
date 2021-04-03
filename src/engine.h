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

#include <MGConfItem>
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

    enum ActionType {
        InsertionAction,
        RemovalAction,
        FlippingAction,
        ClearingAction,
    };
    Q_ENUM(ActionType)

public slots:
    void init();
    void load(const QString &gameFile);
    void start();
    void restart();
    void undoMove();
    void redoMove();
    void dealCard();
    void getHint();
    void drag(quint32 id, int slotId, const CardList &cards);
    void cancelDrag(quint32 id, int slotId, const CardList &cards);
    void checkDrop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void drop(quint32 id, int startSlotId, int endSlotId, const CardList &cards);
    void click(quint32 id, int slotId);
    void doubleClick(quint32 id, int slotId);
    void requestGameOptions();
    void setGameOption(const GameOption &option);
    void setGameOptions(const GameOptionList &options);
    void saveState();
    void resetSavedState();
    void restoreSavedState();

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

    void startEngine(bool newSeed);

    explicit Engine(QObject *parent = nullptr);
    static Engine *s_engine;
    EnginePrivate *d_ptr;
    MGConfItem m_stateConf;
};

#endif // ENGINE_H
