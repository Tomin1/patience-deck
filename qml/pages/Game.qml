/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2022 Tomi Leppänen
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
import "components"

Page {
    id: page

    property int longSide: Math.max(page.height, page.width)
    property int shortSide: Math.min(page.height, page.width)
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

    Component.onCompleted: {
        // Break bindings
        page.shortSide = Math.min(page.height, page.width)
        page.longSide = Math.max(page.height, page.width)
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

        states: State {
            name: "landscape"
            when: page.isLandscape
            PropertyChanges { target: toolbar; vertical: true }
            PropertyChanges {
                target: tableContainer

                x: toolbar.width
                y: 0
                height: shortSide - messageBar.height
                width: longSide - toolbar.width
            }
            PropertyChanges {
                target: table
                height: shortSide - messageBar.height
                width: longSide - toolbar.totalSpaceX
                horizontalMargin: Theme.paddingLarge
                verticalMargin: Theme.paddingSmall
            }
            AnchorChanges { target: messageBar; anchors.left: toolbar.right }
        }

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
            vertical: false
            pageActive: page.active
            z: 10
        }

        Item {
            id: tableContainer

            clip: toolbar.expanded || toolbar.animating
            x: 0
            y: toolbar.height
            height: longSide - messageBar.height - toolbar.height
            width: shortSide

            Table {
                id: table

                enabled: Patience.state < Patience.GameOverState && !Patience.engineFailed

                height: longSide - toolbar.totalSpaceY - messageBar.height
                width: shortSide
                anchors.horizontalCenter: parent.horizontalCenter

                minimumSideMargin: Theme.horizontalPageMargin
                horizontalMargin: Theme.paddingSmall
                maximumHorizontalMargin: Theme.paddingLarge
                verticalMargin: Theme.paddingLarge
                maximumVerticalMargin: Theme.paddingLarge

                backgroundColor: settings.backgroundColor
                highlightColor: Theme.rgba(Theme.highlightColor, Theme.opacityLow)

                layer.enabled: pullDownMenu.active

                FeedbackEvent.onClicked: feedback.playEffect()
                FeedbackEvent.onDropSucceeded: feedback.playEffect()
                FeedbackEvent.onDropFailed: feedback.playEffect(true)
                FeedbackEvent.onSelectionChanged: feedback.playEffect(true)

                Component.onCompleted: Patience.restoreSavedOrLoad("klondike.scm")
            }
        }

        MouseArea {
            id: messageBar

            anchors {
                left: parent.left
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
            source: "components/GameOverOverlay.qml"
            x: tableContainer.x
            y: tableContainer.y
            height: tableContainer.height
            width: tableContainer.width
            z: 5
        }

        Loader {
            id: failureOverlayLoader
            active: Patience.engineFailed
            source: "components/EngineFailureOverlay.qml"
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
