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

ComboBox {
    property var choices: []
    property var initialValue
    property int defaultIndex
    property bool _ready

    signal choiceSelected(string choice)

    currentIndex: {
        var index = choices.indexOf(initialValue)
        return index !== -1 ? index : defaultIndex
    }

    onCurrentIndexChanged: if (_ready) choiceSelected(choices[currentIndex])
    Component.onCompleted: _ready = true
}
