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
    id: aboutPage

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
                //: Name of this application
                //% "Patience Deck"
                description: qsTrId("patience-de-patience_deck")
            }

            Label {
                color: Theme.highlightColor
                //: About this application, %1 is the name of the author. Keep the line break.
                //% "This is a collection of patience games for Sailfish OS written by %1.<br />It utilises GNOME Aisleriot's implementations of patience games and artwork."
                text: qsTrId("patience-la-about_text").arg("Tomi Leppänen")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            Label {
                color: Theme.highlightColor
                linkColor: Theme.primaryColor
                text: "Github: <a href=\"%1\">%1</a>".arg("https://github.com/Tomin1/patience-deck/")
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
                //: Listing the card graphics creator below this
                //% "Also thank you to card graphics creator"
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
                height: implicitHeight + Theme.paddingLarge
            }
        }

        VerticalScrollDecorator {}
    }
}
