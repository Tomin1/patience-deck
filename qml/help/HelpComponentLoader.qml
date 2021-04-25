/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021 Tomi Leppänen
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

Loader {
    active: true
    source: {
        if (model.type.length == 5 && model.type.substring(0, 4) == "sect") {
            return "components/Section.qml"
        } else if (model.type == "title") {
            return "components/Title.qml"
        } else if (model.type == "para") {
            return "components/Para.qml"
        } else if (model.type == "informaltable") {
            return "components/InformalTable.qml"
        } else if (model.type == "variablelist") {
            return "components/VariableList.qml"
        } else if (model.type == "itemizedlist") {
            return "components/ItemizedList.qml"
        } else if (model.type == "screenshot") {
            return "components/Screenshot.qml"
        } else {
            console.log("Unknown type", model.type, "in", __HelpView_sourceFile)
            return ""
        }
    }
}
