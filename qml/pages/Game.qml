/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021  Tomi Leppänen
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

import QtFeedback 5.0
import QtQml 2.2
import QtQuick 2.6
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import Nemo.KeepAlive 1.2
import Patience 1.0
import "../toolbar"

Page {
    id: page

    property bool active: page.status === PageStatus.Active
    property bool needsGameStart

    allowedOrientations: Orientation.All

    function resetHint() {
        message.hint = ""
    }

    onOrientationChanged: message.x = 0

    onActiveChanged: {
        Patience.paused = !active
        resetHint()
        if (needsGameStart) {
            Patience.startNewGame()
            needsGameStart = false
        }
    }

    Connections {
        target: Qt.application
        onStateChanged: {
            if (Qt.application.state === Qt.ApplicationActive) {
                Patience.paused = !page.active
            } else {
                Patience.paused = true
            }
            resetHint()
        }
    }

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            id: pullDownMenu

            MenuItem {
                //% "About"
                text: qsTrId("patience-me-about")
                onClicked: pageStack.push(Qt.resolvedUrl("about/AboutPage.qml"))
            }

            MenuItem {
                //% "Select game"
                text: qsTrId("patience-me-select_game")
                enabled: !Patience.engineFailed
                onClicked: pageStack.push(Qt.resolvedUrl("SelectGame.qml"))
            }

            MenuItem {
                //% "Options & Rules"
                text: qsTrId("patience-me-game_options")
                enabled: !Patience.engineFailed
                onClicked: pageStack.push(Qt.resolvedUrl("OptionsAndRules.qml"))
            }
        }

        contentHeight: parent.height

        Toolbar {
            id: toolbar

            enabled: !Patience.engineFailed
            vertical: page.isLandscape
            pageActive: page.active
            z: 10
        }

        Item {
            id: tableContainer

            clip: toolbar.expanded || toolbar.animating
            x: page.isLandscape ? toolbar.width : 0
            y: page.isLandscape ? 0 : toolbar.height
            height: page.height - messageBar.height - (page.isLandscape ? 0 : toolbar.height)
            width: page.width - (page.isLandscape ? toolbar.width : 0)

            Table {
                id: table

                enabled: Patience.state < Patience.GameOverState && !Patience.engineFailed

                height: page.height - (page.isLandscape ? 0 : Theme.itemSizeLarge + toolbar.handleWidth) - messageBar.height
                width: page.width - (page.isLandscape ? Theme.itemSizeLarge + toolbar.handleWidth : 0)
                anchors.horizontalCenter: parent.horizontalCenter

                minimumSideMargin: Theme.horizontalPageMargin
                horizontalMargin: page.isLandscape ? Theme.paddingLarge : Theme.paddingSmall
                maximumHorizontalMargin: Theme.paddingLarge
                verticalMargin: page.isLandscape ? Theme.paddingSmall : Theme.paddingLarge
                maximumVerticalMargin: Theme.paddingLarge

                backgroundColor: settings.backgroundColor
                highlightColor: Theme.rgba(Theme.highlightColor, Theme.opacityLow)

                layer.enabled: pullDownMenu.active

                FeedbackEvent.onClicked: feedback.playEffect()
                FeedbackEvent.onDropSucceeded: feedback.playEffect()
                FeedbackEvent.onDropFailed: feedback.playEffect(true)

                Component.onCompleted: Patience.restoreSavedOrLoad("klondike.scm")
            }
        }

        MouseArea {
            id: messageBar

            anchors {
                left: page.isLandscape ? toolbar.right : parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: message.height
            clip: message.contentWidth > width

            drag {
                target: message
                axis: Drag.XAxis
                minimumX: Math.min(width - message.contentWidth, 0)
                maximumX: 0
                threshold: 0
            }

            Label {
                id: message
                property string hint
                text: hint !== "" && hintTimer.running ? hint : Patience.message
                color: Theme.highlightColor
                anchors.bottom: parent.bottom
                onTextChanged: x = 0

                Timer {
                    id: hintTimer
                    interval: 60*1000
                }
            }
        }

        Loader {
            id: overlayLoader

            active: Patience.state === Patience.WonState || Patience.state === Patience.GameOverState
            source: "GameOverOverlay.qml"
            x: tableContainer.x
            y: tableContainer.y
            height: tableContainer.height
            width: tableContainer.width
            z: 5
        }

        Loader {
            id: failureOverlayLoader
            active: Patience.engineFailed
            source: "EngineFailureOverlay.qml"
            x: tableContainer.x
            y: tableContainer.y
            height: tableContainer.height
            width: tableContainer.width
            z: 10
        }
    }

    Connections {
        target: Patience
        onStateChanged: {
            if (Patience.state === Patience.LoadedState) {
                if (active) {
                    Patience.startNewGame()
                } else {
                    needsGameStart = true
                }
            } else if (Patience.state === Patience.StartingState) {
                resetHint()
            }
        }
        onHint: {
            message.hint = hint
            hintTimer.restart()
        }
        onCardMoved: resetHint()
    }

    DisplayBlanking {
        preventBlanking: settings.preventBlanking && Patience.state === Patience.RunningState && !Patience.paused
    }

    ThemeEffect {
        id: feedback

        function playEffect(weak) {
            if (settings.feedbackEffects) {
                play(weak ? ThemeEffect.PressWeak : ThemeEffect.Press)
            }
        }
    }

    Settings { id: settings }
}
