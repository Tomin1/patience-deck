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

Grid {
    id: informalTable

    readonly property var contents: model.contents

    columns: model.columns
    columnSpacing: Theme.paddingLarge
    rowSpacing: Theme.paddingSmall

    Repeater {
        id: cells

        property int space: informalTable.width - informalTable.columnSpacing * (informalTable.columns - 1)
        property int maxColumn: space / informalTable.columns
        property int spaceLeft: {
            if (!helpView.ready) {
                return 0
            }

            var sum = 0
            for (var i = 0; i < implicitMaxWidths.length; i++) {
                if (i != 1) {
                    sum += Math.min(implicitMaxWidths[i], maxColumn)
                }
            }
            return space - sum
        }
        property var implicitMaxWidths: new Array(informalTable.columns)

        model: informalTable.contents

        onItemAdded: {
            var position = index % informalTable.columns
            implicitMaxWidths[position] = Math.max(implicitMaxWidths[position] || 0,
                                                   item.implicitWidth)
        }

        Para {
            text: modelData
            width: {
                if (cells.spaceLeft != 0 && index % informalTable.columns == 1) {
                    return Math.min(implicitWidth, cells.spaceLeft)
                } else {
                    return Math.min(implicitWidth, cells.maxColumn)
                }
            }
        }
    }
}
