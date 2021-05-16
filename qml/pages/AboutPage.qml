/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021 Tomi Leppänen
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
import Nemo.Configuration 1.0
import Patience 1.0

Page {
    id: aboutPage

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height
        contentWidth: content.width

        Column {
            id: content

            width: aboutPage.width

            PageHeader {
                //% "About"
                title: qsTrId("patience-he-about")
                //: Name of this application, %1 is version number
                //% "Patience Deck %1"
                description: qsTrId("patience-de-patience_deck").arg(Qt.application.version)
            }

            Label {
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                linkColor: Theme.primaryColor
                //: About this application, %1 is the name of the author. Keep the line break.
                //% "This is a collection of patience games for Sailfish OS written by %1.<br />It utilises <a href=%2>GNOME Aisleriot</a>'s implementations of patience games and artwork."
                text: qsTrId("patience-la-about_text")
                    .arg("Tomi Leppänen")
                    .arg("\"https://wiki.gnome.org/Apps/Aisleriot\"")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
                onLinkActivated: Qt.openUrlExternally(link)
            }

            Label {
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                linkColor: Theme.primaryColor
                text: "Github: <a href=%2>%1</a>"
                    .arg("github.com/Tomin1/patience-deck")
                    .arg("\"https://github.com/Tomin1/patience-deck/\"")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
                onLinkActivated: Qt.openUrlExternally(link)
            }

            SectionHeader {
                //: Thank you section for developers of GNOME Aisleriot
                //% "Thanks"
                text: qsTrId("patience-se-thanks")
            }

            Label {
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                //: Listing all developers of GNOME Aisleriot developers below this
                //% "Thank you to GNOME Aisleriot authors"
                text: qsTrId("patience-la-thank_you_gnome_aisleriot_authors")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            Label {
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: Patience.aisleriotAuthors
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            Label {
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                //: Listing the card graphics creator and game manual author below this
                //% "Also thank you to card graphics creator and game manual author"
                text: qsTrId("patience-la-thank_you_card_graphics_author")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            Label {
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: "Aike Reyer"
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            Label {
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: "Rosanna Yuen"
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            Label {
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                //% "See also individual game manuals for their authors"
                text: qsTrId("patience-la-see_also_manuals")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            SectionHeader {
                //% "Experimental"
                text: qsTrId("patience-se-experimental")
            }

            TextSwitch {
                //% "Show all games"
                text: qsTrId("patience-la-show_all_games")
                description: Patience.showAllGames
                    //% "Select to hide unsupported games"
                    ? qsTrId("patience-de-unsupported_games_shown")
                    //% "Select to show unsupported games"
                    : qsTrId("patience-de-unsupported_games_hidden")
                checked: Patience.showAllGames
                onClicked: Patience.showAllGames = !Patience.showAllGames
                height: implicitHeight
            }

            TextSwitch {
                //% "Prevent display blanking"
                text: qsTrId("patience-la-prevent_display_blanking")
                description: preventBlanking.value
                    //% "Select to allow display blanking while playing"
                    ? qsTrId("patience-de-select_to_allow_display_blanking")
                    //% "Select to prevent display blanking while playing"
                    : qsTrId("patience-de-select_to_prevent_display_blanking")
                checked: preventBlanking.value
                onClicked: preventBlanking.value = !preventBlanking.value
                height: implicitHeight + Theme.paddingLarge
            }
        }

        VerticalScrollDecorator {}
    }

    ConfigurationValue {
        id: preventBlanking
        defaultValue: false
        key: "/site/tomin/apps/PatienceDeck/preventBlanking"
    }
}
