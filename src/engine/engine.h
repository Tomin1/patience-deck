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

class QCommandLineParser;

class EngineHelper;
class EngineInternals;
class Engine : public QObject
{
    Q_OBJECT
public:
    ~Engine();

    static Engine *instance();

#ifndef ENGINE_EXERCISER
    static void addArguments(QCommandLineParser *parser);
    static void setArguments(QCommandLineParser *parser);
#endif // ENGINE_EXERCISER

    enum ActionType {
        InsertionAction,
        RemovalAction,
        FlippingAction,
        ClearingAction,
        MoveEndedAction,
        ActionTypeMask = InsertionAction | RemovalAction | FlippingAction | ClearingAction | MoveEndedAction,
        EngineActionFlag = ActionTypeMask + 1,
    };
    Q_ENUM(ActionType)
    Q_DECLARE_FLAGS(ActionTypeFlags, ActionType)
    Q_FLAG(ActionTypeFlags)

    static ActionType actionType(ActionTypeFlags action);

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
    void gameContinued();
    void restoreCompleted(bool success);
    void gameOver(bool won);
    void gameOptions(GameOptionList options);

    void showScore(bool show);
    void showDeal(bool show);

    void newSlot(int id, const CardList &cards, int type, double x, double y,
                 int expansionDepth, bool expandedDown, bool expandedRight);
    void setExpansionToDown(int id, double expansion);
    void setExpansionToRight(int id, double expansion);
    void action(Engine::ActionTypeFlags action, int slotId, int index, const CardData &card);
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
    friend EngineInternals;

#ifdef ENGINE_EXERCISER
    friend EngineHelper;

#else
    struct ReadSavedState {
        bool valid;
        QString gameFile;
        quint32 seed;
        bool hasSeed;
        bool seedOk;

        ReadSavedState()
            : valid(false)
            , seed(0)
            , hasSeed(false)
            , seedOk(true) {}
    };

    static ReadSavedState readSavedState(const MGConfItem &stateConf);
#endif // ENGINE_EXERCISER

    void loadGame(const QString &gameFile, bool restored);
    void startEngine(bool newSeed);

    explicit Engine(QObject *parent = nullptr);
    static Engine *s_engine;
    EngineInternals *d_ptr;
    quint32 m_action;
#ifndef ENGINE_EXERCISER
    MGConfItem m_stateConf;
#endif // ENGINE_EXERCISER
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Engine::ActionTypeFlags);

#endif // ENGINE_H
