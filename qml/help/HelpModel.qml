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
import QtQuick.XmlListModel 2.0

XmlListModel {
    id: xmlModel

    signal ready()

    onStatusChanged: if (status == XmlListModel.Ready) xmlModel.ready()

    XmlRole {
        name: "type"
        query: "name()"
    }

    XmlRole {
        name: "text"
        query: "string()"
    }

    XmlRole {
        name: "parent"
        query: "\"%1\"".arg(xmlModel.query.slice(0, -1))
    }

    XmlRole {
        name: "position"
        query: "position()"
    }

    XmlRole {
        name: "columns"
        query: "tgroup/@cols/number()"
    }

    XmlRole {
        name: "picture"
        query: "mediaobject/imageobject/imagedata/@fileref/string()"
    }

    XmlRole {
        name: "phrase"
        query: "mediaobject/textobject/phrase/string()"
    }
}
