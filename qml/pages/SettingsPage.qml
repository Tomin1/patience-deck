/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021 Tomi Leppänen
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

Page {
    id: page

    readonly property var backgroundColorOptions: [
        "maroon", "sienna", "peru", "goldenrod", "olive", "darkolivegreen", "green", "seagreen", "darkslategray", "steelblue", "navy", "darkslateblue", "indigo", "palevioletred", "dimgray"
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
                        if (backgroundColor.value === "") {
                            return 1
                        } else if (backgroundColor.color.a === 0.0) {
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
                                backgroundColor.value = Theme.rgba(backgroundColor.color, 0.0)
                            } else if (currentIndex === 1) {
                                backgroundColor.value = ""
                            } else {
                                backgroundColor.value = Theme.rgba(backgroundColor.color, 1.0)
                            }
                        }
                    }
                    Component.onCompleted: ready = true
                }

                Rectangle {
                    anchors {
                        right: parent.right
                        rightMargin: Theme.paddingLarge
                        top: parent.top
                        topMargin: (Theme.itemSizeSmall - height) / 2
                    }
                    height: Theme.itemSizeExtraSmall
                    width: height
                    radius: height / 4
                    color: backgroundSelector.currentIndex !== 1 ? backgroundColor.color : "transparent"
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
                                backgroundColor.value = dialog.color
                            })
                        }
                    }
                }
            }

            ComboBox {
                property bool ready
                readonly property var choices: ["regular", "optimized", "simplified"]

                //: Combo box for selecting card style
                //% "Card style"
                label: qsTrId("patience-la-card_style")
                currentIndex: {
                    var index = choices.indexOf(cardStyle.value)
                    return index !== -1 ? index : 0
                }

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

                onCurrentIndexChanged: if (ready) cardStyle.value = choices[currentIndex]
                Component.onCompleted: ready = true
            }

            SectionHeader {
                //% "Gameplay"
                text: qsTrId("patience-se-gameplay")
            }

            TextSwitch {
                //% "Prevent display from blanking"
                text: qsTrId("patience-la-prevent_display_blanking")
                //% "Display does not dim or turn black while a game is running"
                description: qsTrId("patience-de-display_will_not_blank")
                checked: preventBlanking.value
                onClicked: preventBlanking.value = !preventBlanking.value
            }

            TextSwitch {
                //% "Play feedback effects"
                text: qsTrId("patience-la-play_feedback_effects")
                //% "Vibrates when dropping and clicking"
                description: qsTrId("patience-de-play_feedback_effects")
                checked: feedbackEffects.value
                onClicked: feedbackEffects.value = !feedbackEffects.value
            }
        }

        VerticalScrollDecorator {}
    }

    ConfigurationValue {
        id: backgroundColor
        readonly property color color: value === "" ? "green" : value
        defaultValue: ""
        key: "/site/tomin/apps/PatienceDeck/backgroundColor"
    }

    ConfigurationValue {
        id: cardStyle
        defaultValue: "regular"
        key: "/site/tomin/apps/PatienceDeck/cardStyle"
    }

    ConfigurationValue {
        id: feedbackEffects
        defaultValue: false
        key: "/site/tomin/apps/PatienceDeck/feedbackEffects"
    }

    ConfigurationValue {
        id: preventBlanking
        defaultValue: false
        key: "/site/tomin/apps/PatienceDeck/preventBlanking"
    }
}
