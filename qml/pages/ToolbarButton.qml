/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021  Tomi Leppänen
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

MouseArea {
    property alias imageSource: image.source
    property alias text: label.text
    property alias showText: label.visible
    property bool down: pressed && containsPress

    height: image.height
    width: image.width + (showText ? _buttonSpacing + label.width : 0)

    SilicaControl {
        anchors.fill: parent
        highlighted: parent.down
        opacity: parent.enabled ? 1.0 : Theme.opacityLow

        HighlightImage {
            id: image
            highlighted: parent.highlighted
            height: Theme.itemSizeSmall
            width: Theme.itemSizeSmall
        }

        Label {
            id: label
            anchors {
                left: image.right
                leftMargin: _buttonSpacing
            }
            color: parent.highlighted ? Theme.highlightColor : Theme.primaryColor
            verticalAlignment: Text.AlignVCenter
            height: image.height
        }
    }
}
