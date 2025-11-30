/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2025 Tomi Leppänen
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
import PatienceDeck 1.0

CoverBackground {
    Column {
        anchors.centerIn: parent
        width: parent.width

        Icon {
            anchors.horizontalCenter: parent.horizontalCenter
            source: {
                if (Patience.state === Patience.WonState) {
                    return "../../buttons/icon-m-fireworks.svg"
                } else if (Patience.state === Patience.GameOverState) {
                    return "../../buttons/icon-m-skull.svg"
                } else {
                    return "../../buttons/icon-m-cards.svg"
                }
            }
            sourceSize.height: parent.width - 2 * Theme.paddingMedium
            sourceSize.width: parent.width - 2 * Theme.paddingMedium
        }

        Item {
            height: Theme.paddingMedium
            width: parent.width
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Patience.gameName
            horizontalAlignment: Text.AlignHCenter
            width: parent.width - 2 * Theme.paddingMedium
            wrapMode: Text.Wrap
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Patience.elapsedTime
            color: Theme.secondaryColor
        }
    }
}
