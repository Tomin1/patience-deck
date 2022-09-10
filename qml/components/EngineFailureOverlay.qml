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

import QtQuick 2.6
import Sailfish.Silica 1.0

Rectangle {
    color: Theme.rgba(Theme.overlayBackgroundColor, Theme.opacityOverlay)

    Column {
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        Label {
            //% "Game engine failed"
            text: qsTrId("patience-la-engine_failed")
            color: Theme.highlightColor
            font {
                bold: true
                pixelSize: Theme.fontSizeLarge
            }
            horizontalAlignment: Text.AlignHCenter
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
        }

        Label {
            //% "Please report to Github:"
            text: qsTrId("patience-la-report_to_github")
            color: Theme.highlightColor
            font.bold: true
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
        }

        Label {
            text: "<a href=%2>%1</a>"
                .arg("github.com/Tomin1/patience-deck/issues")
                .arg("\"https://github.com/Tomin1/patience-deck/issues/\"")
            color: Theme.highlightColor
            linkColor: Theme.primaryColor
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * x
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }
}
