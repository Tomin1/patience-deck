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

#ifndef ENGINE_P_H
#define ENGINE_P_H

#include <libguile.h>
#include <QHash>
#include <QList>
#include <QObject>
#include <QTimer>
#include <random>
#include "enginedata.h"

class Engine;
class EngineHelper;
class EnginePrivate : public QObject
{
    Q_OBJECT

public:
    enum Lambda {
        NewGameLambda,
        ButtonPressedLambda,
        ButtonReleasedLambda,
        ButtonClickedLambda,
        ButtonDoubleClickedLambda,
        MovesLeftLambda,
        WinningGameLambda,
        HintLambda,
        GetOptionsLambda,
        ApplyOptionsLambda,
        TimeoutLambda,
        DroppableLambda,
        DealableLambda,
        LambdaCount,
        LastMandatoryLambda = TimeoutLambda,
    };

    enum GameFeature : uint {
        NoFeatures = 0x00,
        FeatureDroppable = 0x01,
        FeatureScoreHidden = 0x02,
        FeatureDealable = 0x04,
        AllFeatures = 0x07,
    };
    Q_ENUM(GameFeature)
    Q_DECLARE_FLAGS(GameFeatures, GameFeature)

    enum GameState : int {
        UninitializedState,
        RestoredState,
        LoadedState,
        BeginState,
        RunningState,
        GameOverState,
    };
    Q_ENUM(GameState)

    explicit EnginePrivate(QObject *parent = nullptr);
    ~EnginePrivate();
    static EnginePrivate *instance();

    GameOptionList getGameOptions();
    void updateDealable();
    void recordMove(int slotId);
    void endMove(bool fromDelayedCall = false);
    void discardMove();
    bool isGameOver();
    bool isWinningGame();
    bool isInitialized();
    void clear(bool resetData = false);
    void testGameOver();

    void setCanUndo(bool canUndo);
    void setCanRedo(bool canRedo);
    void setCanDeal(bool canDeal);
    void setScore(int score);
    void setMessage(QString message);
    void setWidth(double width);
    void setHeight(double height);

    void addSlot(int id, const CardList &cards, SlotType type,
                 double x, double y, int expansionDepth,
                 bool expandedDown, bool expandedRight);
    const CardList &getSlot(int slot);
    void setCards(int id, const CardList &cards);
    void setExpansionToDown(int id, double expansion);
    void setExpansionToRight(int id, double expansion);
    void setLambda(Lambda lambda, SCM func);
    uint getFeatures();
    void setFeatures(uint features);
    bool hasFeature(GameFeature feature);
    int getTimeout();
    void setTimeout(int timeout);
    quint32 getRandomValue(quint32 first, quint32 last);
    void resetGenerator(bool generateNewSeed);
    void die(const char *message);

    bool makeSCMCall(Lambda lambda, SCM *args, size_t n, SCM *retval);
    bool makeSCMCall(SCM lambda, SCM *args, size_t n, SCM *retval);
    bool makeSCMCall(QString name, SCM *args, size_t n, SCM *retval);

    // TODO: Make private
    QTimer *m_delayedCallTimer;

private:
    friend Engine;
#ifdef ENGINE_EXERCISER
    friend EngineHelper;
#endif

    QHash<int, CardList> m_cardSlots;
    SCM m_lambdas[LambdaCount];
    GameFeatures m_features;
    GameState m_state;
    int m_timeout;
    QString m_gameFile;
    uint_fast32_t m_seed;
    std::mt19937 m_generator;
    bool m_recordingMove;
    quint32 m_action;

    Engine *engine();
};

#endif // ENGINE_P_H
