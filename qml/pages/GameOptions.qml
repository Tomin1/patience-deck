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
                title: qsTr("Options")
            }

            Label {
                text: gameOptions.count > 0
                    ? qsTr("Starting a new game or changing game options will resuffle the cards.")
                    : qsTr("Starting a new game will resuffle the cards.")
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
                text: qsTr("New game")
                onClicked: {
                    Patience.startNewGame()
                    pageStack.pop()
                }
                anchors.horizontalCenter: parent.horizontalCenter
            }

            SectionHeader {
                text: qsTr("%1 options").arg(Patience.gameName)
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
