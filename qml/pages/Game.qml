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

import QtQml 2.2
import QtQuick 2.6
import Sailfish.Silica 1.0
import Patience 1.0

Page {
    id: page

    property bool isPortrait: orientation & Orientation.PortraitMask
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
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }

            MenuItem {
                //% "Select game"
                text: qsTrId("patience-me-select_game")
                enabled: !Patience.engineFailed
                onClicked: pageStack.push(Qt.resolvedUrl("SelectGame.qml"))
            }

            MenuItem {
                //% "Options & Help"
                text: qsTrId("patience-me-game_options")
                enabled: !Patience.engineFailed
                onClicked: pageStack.push(Qt.resolvedUrl("GameOptions.qml"))
            }
        }

        contentHeight: parent.height

        Toolbar {
            id: toolbar

            enabled: !Patience.engineFailed
            vertical: page.isLandscape
            z: 10

            Connections {
                /*
                 * This is a "looks good enough" workaround for another issue.
                 * I would prefer to set tableContainer.clip = true while
                 * transitioning pages but for some reason Table doesn't behave
                 * well that is set. I would like to fix that for other reasons
                 * as well.
                 */
                target: pageStack
                onBusyChanged: if (pageStack.busy) toolbar.expanded = false
            }
        }

        Item {
            id: tableContainer

            clip: page.isPortrait && (toolbar.expanded || toolbar.animating)
            x: page.isLandscape ? toolbar.width : 0
            y: page.isLandscape ? 0 : toolbar.height
            height: page.height - messageBar.height - (page.isLandcape ? 0 : toolbar.height)
            width: page.width - (page.isLandscape ? toolbar.width : 0)

            Table {
                id: table

                enabled: Patience.state < Patience.GameOverState && !Patience.engineFailed
                height: page.height - (page.isLandscape ? 0 : Theme.itemSizeLarge) - messageBar.height
                width: page.width - (page.isLandscape ? Theme.itemSizeLarge : 0)
                minimumSideMargin: Theme.horizontalPageMargin
                horizontalMargin: isPortrait ? Theme.paddingSmall : Theme.paddingLarge
                maximumHorizontalMargin: Theme.paddingLarge
                verticalMargin: isPortrait ? Theme.paddingLarge : Theme.paddingSmall
                maximumVerticalMargin: Theme.paddingLarge
                highlightColor: Theme.rgba(Theme.highlightColor, Theme.opacityLow)
                layer.enabled: pullDownMenu.active
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
            height: table.height
            width: table.width
            z: 5

            onActiveChanged: if (active) toolbar.expanded = false
        }

        Loader {
            id: failureOverlayLoader
            active: Patience.engineFailed
            source: "EngineFailureOverlay.qml"
            x: tableContainer.x
            y: tableContainer.y
            height: table.height
            width: table.width
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
}
