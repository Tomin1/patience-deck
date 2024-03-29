/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021 - 2022 Tomi Leppänen
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
import Nemo.Configuration 1.0
import Patience 1.0
import "../components"

Page {
    id: page

    readonly property var backgroundColorOptions: [
        "maroon", "sienna", "peru", "goldenrod", "olive", "darkolivegreen", "green", "seagreen", "darkslategray", "steelblue", "navy", "darkslateblue", "indigo", "palevioletred", "dimgray"
    ]
    readonly property var backColorOptions: [
        "#000055", "#4682b4", "#4b0082", "#b22222", "#ff7f50", "#006400", "#9acd32", "#deb887", "#000000"
    ]

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        Column {
            id: column

            bottomPadding: Theme.paddingLarge
            spacing: Theme.paddingMedium
            width: parent.width

            PageHeader {
                //% "Settings"
                title: qsTrId("patience-he-settings")
            }

            Label {
                //% "These settings affect all games"
                text: qsTrId("patience-la-affect-all-games")
                color: Theme.highlightColor
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }
                wrapMode: Text.Wrap
            }

            SectionHeader {
                //% "Appearance"
                text: qsTrId("patience-se-appearance")
            }

            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                height: backgroundSelector.height

                ComboBox {
                    id: backgroundSelector

                    property bool ready

                    //% "Background"
                    label: qsTrId("patience-la-background")
                    currentIndex: {
                        if (settings.backgroundColorValue === "") {
                            return 1
                        } else if (settings.solidBackgroundColor.a === 0.0) {
                            return 2
                        } else {
                            return 0
                        }
                    }
                    anchors {
                        left: parent.left
                        top: parent.top
                    }

                    menu: ContextMenu {
                        MenuItem {
                            //: Solid (opaque) background color
                            //% "Solid"
                            text: qsTrId("patience-me-solid")
                        }

                        MenuItem {
                            //: Background color is adapted to current ambience
                            //% "Adaptive"
                            text: qsTrId("patience-me-adaptive")
                        }

                        MenuItem {
                            //: Transparent background, no color
                            //% "Transparent"
                            text: qsTrId("patience-me-transparent")
                        }
                    }

                    onCurrentIndexChanged: {
                        if (ready) {
                            if (currentIndex === 2) {
                                settings.backgroundColorValue = Theme.rgba(settings.solidBackgroundColor, 0.0)
                            } else if (currentIndex === 1) {
                                settings.backgroundColorValue = ""
                            } else {
                                settings.backgroundColorValue = Theme.rgba(settings.solidBackgroundColor, 1.0)
                            }
                        }
                    }
                    Component.onCompleted: ready = true
                }

                Rectangle {
                    anchors {
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        top: parent.top
                        topMargin: (Theme.itemSizeSmall - height) / 2
                    }
                    height: Theme.itemSizeExtraSmall
                    width: height
                    radius: height / 4
                    color: backgroundSelector.currentIndex !== 1 ? settings.solidBackgroundColor : "transparent"
                    border {
                        color: backgroundSelector.currentIndex === 0 ? Theme.primaryColor : "transparent"
                        width: 2
                    }

                    BackgroundItem {
                        anchors.fill: parent
                        enabled: backgroundSelector.currentIndex === 0
                        contentItem.radius: height / 4

                        onClicked: {
                            var dialog = pageStack.push("Sailfish.Silica.ColorPickerDialog", {
                                "colors": page.backgroundColorOptions
                            })
                            dialog.accepted.connect(function() {
                                settings.backgroundColorValue = dialog.color
                            })
                        }
                    }
                }
            }

            ChoiceBox {
                initialValue: settings.cardStyle
                defaultIndex: 0
                choices: ["regular", "optimized", "simplified"]

                //: Combo box for selecting card style
                //% "Card style"
                label: qsTrId("patience-la-card_style")

                menu: ContextMenu {
                    MenuItem {
                        // Regular variant, looks like playing cards
                        //% "Regular"
                        text: qsTrId("patience-la-card_style_regular")
                    }

                    MenuItem {
                        //: Optimised for mobile use
                        //% "Optimised"
                        text: qsTrId("patience-la-card_style_optimised")
                    }

                    MenuItem {
                        //: Simpler style, more suitable for tiny cards
                        //% "Simplified"
                        text: qsTrId("patience-la-card_style_simplified")
                    }
                }

                onChoiceSelected: settings.cardStyle = choice
            }

            BackgroundItem {
                onClicked: {
                    var dialog = pageStack.push("Sailfish.Silica.ColorPickerDialog", {
                        "colors": page.backColorOptions
                    })
                    dialog.accepted.connect(function() {
                        var colors = settings.cardColors.split(";")
                        colors[0] = dialog.color
                        settings.cardColors = colors.join(";")
                    })
                }

                Label {
                    //% "Card back colour"
                    text: qsTrId("patience-la-card_back_color")
                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                }

                Rectangle {
                    anchors {
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    height: Theme.itemSizeExtraSmall
                    width: height
                    radius: height / 4
                    color: {
                        var backColor = settings.cardColors.split(";")[0]
                        return backColorOptions.indexOf(backColor) !== -1 ? backColor : backColorOptions[0]
                    }
                    border {
                        color: parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                        width: 2
                    }
                }
            }

            ChoiceBox {
                id: suitColorsPicker
                initialValue: {
                    var sep = settings.cardColors.indexOf(";")
                    if (sep === -1) {
                        return ""
                    }
                    return settings.cardColors.substring(sep + 1)
                }
                defaultIndex: 0
                choices: ["", "green;blue", ";orange;;green", "blue;orange"]

                //% "Card suit colours"
                label: qsTrId("patience-la-card_suit_colors")

                menu: ContextMenu {
                    Repeater {
                        model: suitColorsPicker.choices

                        MenuItem {
                            readonly property var colors: modelData.split(";")

                            function getColor(index, defaultValue) {
                                return colors[index] || defaultValue
                            }

                            text: "<font color=\"%1\">♣</font>".arg(getColor(0, "black")) + 
                                  "<font color=\"%1\">♦</font>".arg(getColor(1, "red")) +
                                  "<font color=\"%1\">♥</font>".arg(getColor(2, "red")) +
                                  "<font color=\"%1\">♠</font>".arg(getColor(3, "black"))
                        }
                    }
                }

                onChoiceSelected: {
                    var sep = settings.cardColors.indexOf(";")
                    if (sep === -1) {
                        settings.cardColors = settings.cardColors + (choice === "" ? "" : ";" + choice)
                    } else {
                        settings.cardColors = "%1;%2".arg(settings.cardColors.substring(0, sep)).arg(choice)
                    }
                }
            }

            SectionHeader {
                //% "Gameplay"
                text: qsTrId("patience-se-gameplay")
            }

            ChoiceBox {
                initialValue: settings.landscapeToolbarSide
                defaultIndex: 0
                choices: ["auto", "left", "right"]

                //: Combo box to select toolbar placement on landscape orientation
                //% "Toolbar position"
                label: qsTrId("patience-la-toolbar_position")
                //: Description text for combo box
                //% "Set toolbar placement on landscape orientation"
                description: qsTrId("patience-de-set_toolbar_placement_on_landscape_orientation")
                menu: ContextMenu {
                    MenuItem {
                        //: Put the toolbar on the "top" side of the device,
                        //: meaning that it can be on left or right depending on orientation
                        //% "Orientation dependent"
                        text: qsTrId("patience-la-orientation_dependent")
                    }
                    MenuItem {
                        //: Toolbar is always on left side when device is on landscape orientation
                        //% "Always on left"
                        text: qsTrId("patience-la-always_on_left")
                    }
                    MenuItem {
                        //: Toolbar is always on right side when device is on landscape orientation
                        //% "Always on right"
                        text: qsTrId("patience-la-always_on_right")
                    }
                }

                onChoiceSelected: settings.landscapeToolbarSide = choice
            }

            ChoiceBox {
                id: delayedCallDelayChoice
                readonly property bool custom: choices.indexOf(settings.delayedCallDelay) === -1
                initialValue: settings.delayedCallDelay
                defaultIndex: custom ? 3 : 1
                choices: [0, 50, 200]

                //: Combo box to select delay for game engine itself moving cards between moves
                //% "Auto move delay"
                label: qsTrId("patience-la-automated_move_delay")
                //% "Set how quickly automatic moves repeat"
                description: qsTrId("patience-de-set_how_quickly_automatic_moves_repeat")
                menu: ContextMenu {
                    MenuItem {
                        //: No delay for moves from game engine, moves happen instantly
                        //% "Instant"
                        text: qsTrId("patience-la-instant_delay")
                    }

                    MenuItem {
                        //: Very short delay for moves from game engine, moves happen quickly
                        //% "Quick"
                        text: qsTrId("patience-la-quick_delay")
                    }

                    MenuItem {
                        //: Very long delay for moves from game engine, moves happen slowly
                        //% "Slow"
                        text: qsTrId("patience-la-slow_delay")
                    }

                    MenuItem {
                        // Custom option if user has set the value with dconf
                        text: "%1 ms".arg(settings.delayedCallDelay)
                        visible: delayedCallDelayChoice.custom
                    }
                }

                onChoiceSelected: settings.delayedCallDelay = choice
            }

            TextSwitch {
                //% "Prevent display from blanking"
                text: qsTrId("patience-la-prevent_display_blanking")
                //% "Display does not dim or turn black while a game is running"
                description: qsTrId("patience-de-display_will_not_blank")
                checked: settings.preventBlanking
                onClicked: settings.preventBlanking = !settings.preventBlanking
            }

            TextSwitch {
                //% "Play feedback effects"
                text: qsTrId("patience-la-play_feedback_effects")
                //% "Vibrates when dropping and clicking"
                description: qsTrId("patience-de-play_feedback_effects")
                checked: settings.feedbackEffects
                onClicked: settings.feedbackEffects = !settings.feedbackEffects
            }
        }

        VerticalScrollDecorator {}
    }

    Settings { id: settings }
}
