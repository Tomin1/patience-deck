/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021  Tomi Leppänen
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
        id: overlay

        readonly property real gray: Theme.colorScheme === Theme.LightOnDark ? 0.3 : 0.8
        readonly property int maximumWidth: parent.width - 2*Theme.horizontalPageMargin
        readonly property int contentWidth: Math.max(gameOverText.width, newGameButton.width)

        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        height: column.height
        width: Math.min(maximumWidth, contentWidth + 2*column.padding)
        color: Qt.rgba(gray, gray, gray, 0.8)
        radius: Theme.paddingMedium
        border {
            color: Theme.secondaryColor
            width: 1
        }

        Column {
            id: column

            width: parent.width
            spacing: Theme.paddingMedium
            padding: Theme.paddingLarge

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
                horizontalAlignment: Text.AlignHCenter
                width: parent.width - 2*parent.padding
            }

            Button {
                id: newGameButton

                text: Patience.state === Patience.WonState
                    //: Shown on top of the game when player has won the game
                    //% "New game"
                    ? qsTrId("patience-bt-new_game")
                    //: Shown on top of the game when player has lost the game
                    //% "Restart"
                    : qsTrId("patience-bt-restart")
                anchors.horizontalCenter: parent.horizontalCenter

                onClicked: {
                    if (Patience.state === Patience.WonState) {
                        Patience.startNewGame()
                    } else {
                        Patience.restartGame()
                    }
                }
            }
        }
    }
}
