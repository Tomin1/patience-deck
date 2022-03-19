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

#ifndef PATIENCE_H
#define PATIENCE_H

#include <MGConfItem>
#include <QObject>
#include <QThread>
#include "engine.h"
#include "timer.h"

class QQmlEngine;
class QJSEngine;
class QThread;
class Patience : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(bool canDeal READ canDeal NOTIFY canDealChanged)
    Q_PROPERTY(int score READ score NOTIFY scoreChanged)
    Q_PROPERTY(QString elapsedTime READ elapsedTime NOTIFY elapsedTimeChanged)
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged)
    Q_PROPERTY(QString gameName READ gameName NOTIFY gameNameChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)
    Q_PROPERTY(bool showScore READ showScore NOTIFY showScoreChanged);
    Q_PROPERTY(bool showDeal READ showDeal NOTIFY showDealChanged);
    Q_PROPERTY(QString aisleriotAuthors READ aisleriotAuthors CONSTANT)
    Q_PROPERTY(QString aisleriotTranslatorInfo READ aisleriotTranslatorInfo CONSTANT)
    Q_PROPERTY(bool showAllGames READ showAllGames WRITE setShowAllGames NOTIFY showAllGamesChanged)
    Q_PROPERTY(QStringList history READ history NOTIFY historyChanged)
    Q_PROPERTY(bool engineFailed READ engineFailed NOTIFY engineFailedChanged)
    Q_PROPERTY(QString helpFile READ helpFile NOTIFY gameNameChanged)
    Q_PROPERTY(int gamesCount READ gamesCount CONSTANT)
    Q_PROPERTY(bool showLibraryLicenses READ showLibraryLicenses CONSTANT)

public:
    static Patience* instance();
    static QObject* instance(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~Patience();

    enum GameState {
        UninitializedState,
        LoadedState,
        StartingState,
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
    Q_INVOKABLE void dealCard();
    Q_INVOKABLE void getHint();
    Q_INVOKABLE void restoreSavedOrLoad(const QString &fallback);
    Q_INVOKABLE QString getIconPath(int size) const;

    // Properties
    bool canUndo() const;
    bool canRedo() const;
    bool canDeal() const;
    bool showDeal() const;
    QString gameName() const;
    QString gameFile() const;
    QString helpFile() const;
    int score() const;
    bool showScore() const;
    QString elapsedTime() const;
    GameState state() const;
    bool paused() const;
    void setPaused(bool paused);
    QString message() const;
    QString aisleriotAuthors() const;
    QString aisleriotTranslatorInfo() const;
    bool showAllGames() const;
    void setShowAllGames(bool show);
    QStringList history() const;
    bool engineFailed() const;
    int gamesCount() const;
    bool showLibraryLicenses() const;

signals:
    void canUndoChanged();
    void canRedoChanged();
    void canDealChanged();
    void scoreChanged();
    void elapsedTimeChanged();
    void stateChanged();
    void pausedChanged();
    void gameNameChanged();
    void messageChanged();
    void showScoreChanged();
    void showDealChanged();
    void hint(const QString &hint);
    void cardMoved();
    void showAllGamesChanged();
    void historyChanged();
    void engineFailedChanged();

    void doStart();
    void doRestart();
    void doLoad(const QString &gameFile);
    void doUndoMove();
    void doRedoMove();
    void doDealCard();
    void doGetHint();
    void doSaveEngineState();
    void doResetSavedEngineState();
    void doRestoreSavedEngineState();

private slots:
    void catchFailure(QString message);
    void handleGameLoaded(const QString &gameFile);
    void handleGameStarted();
    void handleGameContinued();
    void handleCardMoved();
    void handleGameOver(bool won);
    void handleCanUndoChanged(bool canUndo);
    void handleCanRedoChanged(bool canRedo);
    void handleCanDealChanged(bool canDeal);
    void handleScoreChanged(int score);
    void handleMessageChanged(const QString &message);
    void handleShowScore(bool show);
    void handleShowDeal(bool show);
    void handleRestoreCompleted(bool success);

private:
    explicit Patience(QObject *parent = nullptr);
    void setState(GameState state);

    QThread m_engineThread;
    bool m_engineFailed;
    bool m_canUndo;
    bool m_canRedo;
    bool m_canDeal;
    bool m_showDeal;
    int m_score;
    bool m_showScore;
    GameState m_state;
    QString m_gameFile;
    QString m_message;
    MGConfItem m_historyConf;
    Timer m_timer;

    static Patience *s_game;
};

#endif // PATIENCE_H
