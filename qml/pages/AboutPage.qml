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
                title: qsTr("About")
                description: qsTr("Patience Deck")
            }

            Label {
                color: Theme.highlightColor
                text: qsTr("This is a collection of patience games for Sailfish OS written by %1.\nIt utilises GNOME Aisleriot's implementations of patience games and artwork.").arg("Tomi Leppänen")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            SectionHeader {
                text: qsTr("Thanks")
            }

            Label {
                color: Theme.highlightColor
                text: qsTr("Thank you to GNOME Aisleriot authors")
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
                text: qsTr("Also thank you to card graphics creator")
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
        }

        VerticalScrollDecorator {}
    }
}
