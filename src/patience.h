/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020  Tomi Leppänen
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

#include <QObject>
#include <QThread>
#include "engine.h"

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
    Q_PROPERTY(int state READ state NOTIFY stateChanged);
    Q_PROPERTY(QString gameName READ gameName NOTIFY gameNameChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged);
    Q_PROPERTY(QString aisleriotAuthors READ aisleriotAuthors CONSTANT);
    Q_PROPERTY(bool showAllGames READ showAllGames WRITE setShowAllGames NOTIFY showAllGamesChanged)
    Q_PROPERTY(QStringList history READ history NOTIFY historyChanged);

public:
    static Patience* instance();
    static QObject* instance(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~Patience();

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
    QString aisleriotAuthors() const;
    bool showAllGames() const;
    void setShowAllGames(bool show);
    QStringList history() const;

signals:
    void canUndoChanged();
    void canRedoChanged();
    void canDealChanged();
    void scoreChanged();
    void stateChanged();
    void gameNameChanged();
    void messageChanged();
    void showAllGamesChanged();
    void historyChanged();

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
    explicit Patience(QObject *parent = nullptr);
    void setState(GameState state);

    QThread m_engineThread;
    bool m_canUndo;
    bool m_canRedo;
    bool m_canDeal;
    int m_score;
    GameState m_state;
    QString m_gameFile;
    QString m_message;

    static Patience *s_game;
};

#endif // PATIENCE_H
