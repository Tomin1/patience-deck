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
import Patience 1.0

Loader {
    active: true
    source: {
        if (model.type === HelpModel.TitleElement) {
            return "components/Title.qml"
        } else if (model.type === HelpModel.ParaElement) {
            return "components/Para.qml"
        } else if (model.type === HelpModel.InformalTableElement) {
            return "components/InformalTable.qml"
        } else if (model.type === HelpModel.VariableListElement) {
            return "components/VariableList.qml"
        } else if (model.type === HelpModel.ItemizedListElement) {
            return "components/ItemizedList.qml"
        } else if (model.type === HelpModel.ScreenshotElement) {
            return "components/Screenshot.qml"
        } else {
            console.log("Unknown type", model.type, "in", helpView.source)
            return ""
        }
    }
}
