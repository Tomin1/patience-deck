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
                enabled: Patience.state < Patience.GameOverState
                height: page.height - header.height - message.height
                width: parent.width
                minimumSideMargin: Theme.horizontalPageMargin
                horizontalMargin: isPortrait ? Theme.paddingSmall : Theme.paddingLarge
                maximumHorizontalMargin: Theme.paddingLarge
                verticalMargin: isPortrait ? Theme.paddingLarge : Theme.paddingSmall
                maximumVerticalMargin: Theme.paddingLarge
                Component.onCompleted: Patience.restoreSavedOrLoad("klondike.scm")

                Loader {
                    active: Patience.state === Patience.WonState || Patience.state === Patience.GameOverState
                    sourceComponent: Component {
                        Rectangle {
                            gradient: Gradient {
                                GradientStop {
                                    position: 1.0
                                    color: Qt.rgba(1.0, 1.0, 1.0, 0.0)
                                }
                                GradientStop {
                                    position: 0.0
                                    color: Qt.rgba(1.0, 1.0, 1.0, 0.5)
                                }
                            }

                            Rectangle {
                                readonly property real gray: Theme.colorScheme === Theme.LightOnDark ? 0.3 : 0.8
                                anchors {
                                    horizontalCenter: parent.horizontalCenter
                                    verticalCenter: parent.verticalCenter
                                }
                                height: gameOverText.implicitHeight + 2*Theme.paddingSmall
                                width: Math.min(gameOverText.implicitWidth + 2*Theme.paddingMedium, parent.width - 2*Theme.horizontalPageMargin)
                                color: Qt.rgba(gray, gray, gray, 0.8)
                                radius: 15
                                border {
                                    color: Theme.secondaryColor
                                    width: 1
                                }

                                Label {
                                    id: gameOverText

                                    text: Patience.state === Patience.WonState
                                        //: Shown on top of the game when player has won the game
                                        //% "You win"
                                        ? qsTrId("patience-la-game_won")
                                        //: Shown on top of the game when player has lost the game
                                        //% "Game over"
                                        : qsTrId("patience-la-game_over")
                                    color: Theme.highlightColor
                                    font.pixelSize: Theme.fontSizeHuge
                                    font.bold: true
                                    wrapMode: Text.Wrap

                                    anchors.verticalCenter: parent.verticalCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    x: Theme.paddingMedium
                                    width: parent.width - 2*x
                                }
                            }
                        }
                    }
                    anchors.fill: parent
                    z: 5
                }
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
