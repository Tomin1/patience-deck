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

ParallelAnimation {
    SmoothedAnimation {
        target: titleLandscape
        properties: "width"
        from: spaceX - Theme.paddingSmall * 2
        to: maximumSpaceX - Theme.paddingSmall * 2
        velocity: toolbarVelocity
    }
    SmoothedAnimation {
        target: scoreTextLandscape
        properties: "x"
        from: Math.min(-scoreTextLandscape.nameWidth + spaceX - minimumSpaceX, 0)
        to: Math.min(-scoreTextLandscape.nameWidth + maximumSpaceX - minimumSpaceX, 0)
        velocity: toolbarVelocity
    }
    SmoothedAnimation {
        target: elapsedTextLandscape
        properties: "x"
        from: Math.min(-elapsedTextLandscape.nameWidth + spaceX - minimumSpaceX, 0)
        to: Math.min(-elapsedTextLandscape.nameWidth + maximumSpaceX - minimumSpaceX, 0)
        velocity: toolbarVelocity
    }
}
