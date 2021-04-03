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

Row {
    property alias text: nameLabel.text
    property alias value: valueLabel.text
    property alias nameVisible: nameLabel.visible
    readonly property int nameWidth: nameLabel.width + spacing
    property int maximumWidth

    spacing: Theme.paddingSmall
    Behavior on x {
        enabled: animating
        NumberAnimation { duration: 100 }
    }

    Label {
        id: nameLabel
        color: Theme.highlightColor
        opacity: expanded || animating ? 1.0 : 0.0
    }

    Label {
        id: valueLabel
        color: Theme.highlightColor
        truncationMode: TruncationMode.Fade
        width: Math.min(maximumWidth, Math.ceil(contentWidth))
    }
}
