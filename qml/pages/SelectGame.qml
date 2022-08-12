/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2022 Tomi Leppänen
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
    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView
        model: GameList { id: gameList }
        anchors.fill: parent
        bottomMargin: Theme.paddingLarge
        currentIndex: -1
        header: Column {
            width: parent.width

            PageHeader {
                //% "Games"
                title: qsTrId("patience-he-games")
            }

            SearchField {
                id: gameSearchField
                //% "Type to search games"
                placeholderText: qsTrId("patience-ph-search_games")
                width: parent.width
                Component.onCompleted: gameList.searchedText = Qt.binding(function() { return text })
            }
        }
        delegate: BackgroundItem {
            id: delegate

            Label {
                id: nameLabel
                anchors {
                    verticalCenter: parent.verticalCenter
                    verticalCenterOffset: originalName.visible ? -Theme.fontSizeMedium/2 : 0
                }
                x: Theme.horizontalPageMargin
                text: display
                color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
            }

            Label {
                id: originalName
                anchors {
                    left: nameLabel.left
                    top: nameLabel.bottom
                }
                text: capitalized
                font.pixelSize: Theme.fontSizeExtraSmall
                color: delegate.highlighted ? Theme.highlightColor : Theme.secondaryColor
                visible: matchedBy == GameList.CapitalizedName
            }

            IconButton {
                icon.source: "image://theme/icon-m-favorite" + (favorite ? "-selected" : "")
                anchors {
                    verticalCenter: parent.verticalCenter
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                }
                onClicked: gameList.setFavorite(index, !favorite)
            }

            onClicked: {
                Patience.loadGame(filename)
                pageStack.pop()
            }
        }
        section {
            property: "section"
            delegate: SectionHeader {
                text: {
                    if (section == GameList.LastPlayed) {
                        //% "Last played"
                        return qsTrId("patience-se-last_played")
                    } else if (section == GameList.Favorites) {
                        //% "Favourites"
                        return qsTrId("patience-se-favourites")
                    } else if (section == GameList.SearchResults) {
                        //% "Search results"
                        return qsTrId("patience-se-search_results")
                    } else {
                        //% "All games"
                        return qsTrId("patience-se-all_games")
                    }
                }
            }
        }

        VerticalScrollDecorator {}
    }
}
