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
import QtQml.Models 2.2
import Sailfish.Silica 1.0
import Patience 1.0
import "../help"

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
                //% "Options & Help"
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
                height: implicitHeight + Theme.paddingSmall
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
                //% "Options"
                text: qsTrId("patience-se-game_options")
                visible: gameOptions.count > 0
            }

            Repeater {
                model: gameOptions
                delegate: Component {
                    Loader {
                        property string displayName: display
                        property bool selected: set
                        property int selectedIndex: current
                        property int topIndex: index

                        signal select(bool selected)
                        signal selectIndex(int index)

                        function finish() {
                            Patience.startNewGame()
                            pageStack.pop()
                        }

                        sourceComponent: type === GameOptions.RadioType
                            ? radioOptionComponent
                            : checkOptionComponent
                        width: parent.width

                        onSelect: {
                            set = selected
                            finish()
                        }
                        onSelectIndex: {
                            current = index
                            finish()
                        }
                    }
                }
                width: parent.width
            }

            SectionHeader {
                //% "Help"
                text: qsTrId("patience-se-game_help")
            }

            BusyIndicator {
                id: loadingIndicator
                anchors.horizontalCenter: parent.horizontalCenter
                height: running ? implicitHeight : 0
                running: true
                size: BusyIndicatorSize.Large
            }

            Loader {
                active: page.status == PageStatus.Active
                sourceComponent: Component {
                    HelpView {
                        source: Patience.helpFile
                        opacity: ready ? 1.0 : 0.0
                        Behavior on opacity {
                            FadeAnimator {
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }
                        }
                        bottomPadding: Theme.paddingLarge
                        onReadyChanged: if (ready) loadingIndicator.running = false
                    }
                }
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }
                onActiveChanged: if (active) active = true // Break binding
            }
        }

        VerticalScrollDecorator {}
    }

    Component {
        id: radioOptionComponent

        ComboBox {
            id: comboBox

            property bool ready

            currentIndex: selectedIndex
            menu: ContextMenu {
                Repeater {
                    model: DelegateModel {
                        model: gameOptions
                        rootIndex: modelIndex(topIndex)

                        delegate: MenuItem {
                            text: display
                            onClicked: comboBox.currentIndex = index
                        }
                    }
                }
            }

            onCurrentIndexChanged: if (ready) parent.selectIndex(currentIndex)
            Component.onCompleted: ready = true
        }
    }

    Component {
        id: checkOptionComponent

        TextSwitch {
            property bool ready

            text: displayName
            checked: selected

            onCheckedChanged: if (ready) parent.select(checked)
            Component.onCompleted: ready = true
        }
    }
}
