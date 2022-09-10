/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2022 Tomi Leppänen
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
import Nemo.Configuration 1.0

Item {
    readonly property color backgroundColor: group.backgroundColor === ""
                                             ? Theme.rgba(Theme.highlightColor, Theme.opacityLow)
                                             : group.backgroundColor
    readonly property color solidBackgroundColor: group.backgroundColor === "" ? "green" : group.backgroundColor
    property alias backgroundColorValue: group.backgroundColor
    property alias cardStyle: group.cardStyle
    property alias feedbackEffects: group.feedbackEffects
    property alias preventBlanking: group.preventBlanking

    ConfigurationGroup {
        id: group

        property string backgroundColor
        property string cardStyle: "regular"
        property bool feedbackEffects
        property bool preventBlanking

        path: "/site/tomin/apps/PatienceDeck"
    }
}
