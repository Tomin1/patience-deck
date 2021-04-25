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

import QtQuick 2.0
import Sailfish.Silica 1.0
import ".."

Column {
    id: itemizedList

    property string query: "%1child::*[%2]/listitem/para".arg(model.parent).arg(model.position)

    Repeater {
        model: ItemizedListModel {
            query: itemizedList.query
        }

        Item {
            anchors {
                left: parent.left
                right: parent.right
            }
            height: text.height

            Rectangle {
                id: bullet

                color: Theme.secondaryHighlightColor
                height: Theme.fontSizeSmall / 3
                width: Theme.fontSizeSmall / 3
                radius: 180
                anchors {
                    left: parent.left
                    leftMargin: Theme.paddingSmall
                    top: parent.top
                    topMargin: Theme.fontSizeSmall / 2
                }
            }

            Para {
                id: text

                text: model.text
                anchors {
                    left: bullet.right
                    leftMargin: Theme.paddingSmall
                    right: parent.right
                }
            }
        }
    }
}
