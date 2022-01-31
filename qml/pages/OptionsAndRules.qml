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
        id: flickable

        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                //% "Common settings"
                text: qsTrId("patience-me-common-settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
        }

        Column {
            id: column

            spacing: Theme.paddingSmall
            width: page.width

            PageHeader {
                // Page title for options and rules page
                //% "Options & Rules"
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
                height: implicitHeight + Theme.paddingMedium
            }

            Button {
                //: Starts a new game
                //% "New game"
                text: qsTrId("patience-bt-new_game")
                onClicked: {
                    Patience.startNewGame()
                    pageStack.navigateBack()
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
                            pageStack.pop(pageStack.previousPage(page))
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
                //% "Rules"
                text: qsTrId("patience-se-game_rules")
            }

            HelpView {
                source: Patience.helpFile
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }
                bottomPadding: Theme.paddingLarge
                spacing: Theme.paddingMedium
            }
        }

        VerticalScrollDecorator {}
    }

    Component {
        id: radioOptionComponent

        ComboBox {
            property bool ready

            currentIndex: selectedIndex
            menu: ContextMenu {
                Repeater {
                    model: DelegateModel {
                        model: gameOptions
                        rootIndex: modelIndex(topIndex)

                        delegate: MenuItem {
                            text: display
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
