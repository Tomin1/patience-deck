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

import QtQuick 2.6
import Patience 1.0

Column {
    id: helpView

    property alias source: helpModel.helpFile
    property alias ready: helpModel.ready

    HelpModel { id: helpModel }

    Repeater {
        model: helpModel

        HelpComponentLoader {
            anchors {
                left: parent.left
                right: parent.right
            }
        }
    }
}
