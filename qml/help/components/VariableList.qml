/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021-2022 Tomi Leppänen
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
    id: variableList

    readonly property var contents: model.contents

    spacing: Theme.paddingMedium

    Repeater {
        model: variableList.contents

        Row {
            height: Math.max(labelText.height, valueText.height)
            spacing: Theme.paddingMedium

            Para {
                id: labelText
                horizontalAlignment: Text.AlignRight
                text: modelData.label
                width: (variableList.width - Theme.paddingMedium) / 2
            }

            Para {
                id: valueText
                color: Theme.highlightColor
                text: modelData.value
                width: (variableList.width - Theme.paddingMedium) / 2
            }
        }
    }
}
