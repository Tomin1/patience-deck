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

import QtQuick 2.0
import Sailfish.Silica 1.0
import Patience 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    GameOptions {
        id: gameOptions
    }

    SilicaFlickable {
        anchors.fill: parent

        contentHeight: column.height

        Column {
            id: column

            spacing: Theme.paddingSmall
            width: page.width

            PageHeader {
                // Page title for options page
                //% "Options"
                title: qsTrId("patience-he-options")
            }

            Label {
                text: gameOptions.count > 0
                    //% "Starting a new game or changing game options will resuffle the cards"
                    ? qsTrId("patience-la-starting_new_or_changing_resuffle")
                    //% "Starting a new game will resuffle the cards"
                    : qsTrId("patience-la-starting_new_resuffle")
                color: Theme.highlightColor
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }
                wrapMode: Text.Wrap
            }

            Button {
                //: Starts a new game
                //% "New game"
                text: qsTrId("patience-bt-new_game")
                onClicked: {
                    Patience.startNewGame()
                    pageStack.pop()
                }
                anchors.horizontalCenter: parent.horizontalCenter
            }

            SectionHeader {
                //: Options section title for game named %1
                //% "%1 options"
                text: qsTrId("patience-se-game_options").arg(Patience.gameName)
                visible: gameOptions.count > 0
            }

            Repeater {
                model: gameOptions
                delegate: BackgroundItem {
                    id: delegate

                    Label {
                        x: Theme.horizontalPageMargin
                        text: display
                        anchors.verticalCenter: parent.verticalCenter
                        color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
                    }

                    Icon {
                        x: page.width - Theme.horizontalPageMargin - width
                        anchors.verticalCenter: parent.verticalCenter
                        source: {
                            if (type == GameOptions.CheckType) {
                                return "image://theme/icon-s-" + (set ? "accept" : "decline")
                            } else {
                                return "image://theme/icon-s-" + (set ? "installed" : "checkmark")
                            }
                        }
                    }

                    onClicked: {
                        if (type != GameOptions.RadioType || !set) {
                            gameOptions.select(index)
                            Patience.startNewGame()
                            pageStack.pop()
                        }
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
