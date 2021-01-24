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
import QtQuick.Layouts 1.0
import Sailfish.Silica 1.0
import Patience 1.0

Item {
    id: toolbar

    property bool vertical
    property bool expanded
    readonly property bool animating: heightAnimation.running || widthAnimation.running
    readonly property int _labeledButtonWidth: Theme.itemSizeLarge + Theme.itemSizeSmall + _buttonSpacing
    readonly property int _buttonSpacing: vertical
        ? (Theme.itemSizeLarge - Theme.itemSizeSmall) / 2
        : Theme.paddingSmall

    height: {
        if (vertical) {
            return parent.height
        } else if (expanded) {
            return _labeledButtonWidth
        } else {
            return Theme.itemSizeLarge
        }
    }
    Behavior on height {
        enabled: !vertical && !orientationTransitionRunning

        NumberAnimation {
            id: heightAnimation
            duration: 100
        }
    }
    width: {
        if (!vertical) {
            return parent.width
        } else if (expanded) {
            return Theme.horizontalPageMargin + _labeledButtonWidth * 2
        } else {
            return Theme.itemSizeLarge
        }
    }
    Behavior on width {
        enabled: vertical && !orientationTransitionRunning

        NumberAnimation {
            id: widthAnimation
            duration: 100
        }
    }
    clip: animating

    GridLayout {
        id: mainButtons

        flow: vertical ? GridLayout.TopToBottom : GridLayout.LeftToRight
        x: vertical ? (Theme.itemSizeLarge - Theme.itemSizeSmall) / 2 : Theme.horizontalPageMargin
        y: vertical ? title.height : (Theme.itemSizeLarge - Theme.itemSizeSmall) / 2
        height: vertical ? Theme.itemSizeLarge * 2 + Theme.paddingSmall : Theme.itemSizeSmall
        width: vertical ? _labeledButtonWidth : expandButton.x - x - Theme.paddingSmall
        columnSpacing: Theme.paddingSmall

        ToolbarButton {
            //% "Undo"
            text: qsTrId("patience-bt-undo")
            imageSource: "../images/icon-m-undo.svg"
            showText: vertical && (expanded || animating)
            enabled: Patience.canUndo
            onClicked: Patience.undoMove()
        }

        ToolbarButton {
            //% "Redo"
            text: qsTrId("patience-bt-redo")
            imageSource: "../images/icon-m-redo.svg"
            showText: vertical && (expanded || animating)
            enabled: Patience.canRedo
            onClicked: Patience.redoMove()
        }
    }

    GridLayout {
        id: extraButtons

        flow: vertical ? GridLayout.TopToBottom : GridLayout.LeftToRight
        x: vertical ? mainButtons.x + mainButtons.width : Theme.horizontalPageMargin
        y: vertical ? mainButtons.y : Theme.itemSizeLarge
        width: vertical ? _labeledButtonWidth : (parent.width - 2 * Theme.horizontalPageMargin)
        rowSpacing: Theme.paddingLarge
        visible: (expanded || animating)

        ToolbarButton {
            //% "Hint"
            text: qsTrId("patience-bt-hint")
            imageSource: "../images/icon-m-hint.svg"
            onClicked: Patience.hint()
        }

        ToolbarButton {
            //% "Deal"
            text: qsTrId("patience-bt-deal")
            imageSource: "../images/icon-m-deal.svg"
            enabled: Patience.canDeal
            onClicked: Patience.dealCard()
        }

        ToolbarButton {
            //% "Restart"
            text: qsTrId("patience-bt-restart")
            imageSource: "../images/icon-m-restart.svg"
            onClicked: Patience.restartGame()
        }
    }

    Label {
        id: title

        // These apply to !vertical
        readonly property int maximumWidth: minimumX - Theme.paddingSmall - Theme.horizontalPageMargin
        readonly property int minimumX: parent.width / 2

        text: Patience.gameName
        color: Theme.highlightColor
        font.pixelSize: Theme.fontSizeLarge
        truncationMode: TruncationMode.Fade
        verticalAlignment: Text.AlignVCenter
        height: vertical ? Theme.itemSizeMedium : Theme.itemSizeLarge
        width: Math.min(contentWidth, vertical ? parent.width : maximumWidth)
        x: vertical ? Theme.paddingSmall : parent.width - width - Theme.horizontalPageMargin
        y: 0
    }

    IconButton {
        id: expandButton

        enabled: !overlayLoader.active
        icon.source: "../images/icon-m-expand.svg"
        icon.height: Theme.itemSizeSmall
        icon.width: Theme.itemSizeSmall
        x: vertical ? mainButtons.x : title.minimumX - width
        y: vertical ? parent.height - height - Theme.paddingSmall : (Theme.itemSizeLarge - height) / 2
        transformOrigin: Item.Center
        rotation: {
            if (vertical) {
                return expanded ? 90 : 270
            } else {
                return expanded ? 180 : 0
            }
        }
        Behavior on rotation {
            enabled: !orientationTransitionRunning
            NumberAnimation { duration: 100 }
        }
        onClicked: expanded = !expanded
    }
}
