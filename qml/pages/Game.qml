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

import QtQuick 2.6
import Sailfish.Silica 1.0
import Patience 1.0

Page {
    id: page

    allowedOrientations: Orientation.All
    property bool isPortrait: orientation & Orientation.PortraitMask

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                //% "About"
                text: qsTrId("patience-me-about")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }

            MenuItem {
                //% "Select game"
                text: qsTrId("patience-me-select_game")
                onClicked: pageStack.push(Qt.resolvedUrl("SelectGame.qml"))
            }

            MenuItem {
                //% "Game options"
                text: qsTrId("patience-me-game_options")
                onClicked: pageStack.push(Qt.resolvedUrl("GameOptions.qml"))
            }
        }

        contentHeight: column.height

        Column {
            id: column

            width: page.width

            PageHeader {
                id: header
                title: Patience.gameName

                Row {
                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    spacing: Theme.paddingSmall

                    IconButton {
                        icon.source: "../images/icon-m-undo.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        enabled: Patience.canUndo
                        onClicked: Patience.undoMove()
                    }

                    IconButton {
                        icon.source: "../images/icon-m-redo.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        enabled: Patience.canRedo
                        onClicked: Patience.redoMove()
                    }

                    IconButton {
                        icon.source: "../images/icon-m-deal.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        enabled: Patience.canDeal
                        onClicked: Patience.dealCard()
                        visible: false // TODO
                    }

                    IconButton {
                        icon.source: "../images/icon-m-hint.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        onClicked: Patience.hint()
                        visible: false // TODO
                    }

                    IconButton {
                        icon.source: "../images/icon-m-restart.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        onClicked: Patience.restartGame()
                    }
                }
            }

            Table {
                id: table

                enabled: Patience.state < Patience.GameOverState
                height: page.height - header.height - message.height
                width: parent.width
                minimumSideMargin: Theme.horizontalPageMargin
                horizontalMargin: isPortrait ? Theme.paddingSmall : Theme.paddingLarge
                maximumHorizontalMargin: Theme.paddingLarge
                verticalMargin: isPortrait ? Theme.paddingLarge : Theme.paddingSmall
                maximumVerticalMargin: Theme.paddingLarge
                Component.onCompleted: Patience.restoreSavedOrLoad("klondike.scm")
            }

            Label {
                id: message
                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: Patience.message
            }
        }

        Loader {
            active: Patience.state === Patience.WonState || Patience.state === Patience.GameOverState
            source: "GameOverOverlay.qml"
            x: table.x
            y: table.y
            height: table.height
            width: table.width
            z: 5
        }
    }

    Connections {
        target: Patience
        onStateChanged: {
            if (Patience.state === Patience.LoadedState) {
                Patience.startNewGame()
            }
        }
    }
}
