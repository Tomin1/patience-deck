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
import Patience 1.0

Item {
    id: toolbar

    property bool vertical
    property bool expanded
    readonly property bool animating: heightAnimation.running || widthAnimation.running
    readonly property int _labeledButtonWidth: Theme.itemSizeLarge * 2

    height: {
        if (vertical) {
            return parent.height
        } else if (expanded) {
            return extraButtons.y + extraButtons.height + Theme.paddingSmall
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

    Flow {
        id: mainButtons

        x: vertical ? 0 : Theme.horizontalPageMargin
        y: vertical ? title.height : 0
        height: vertical ? Theme.itemSizeLarge * 3 + 2 * Theme.paddingSmall : Theme.itemSizeLarge
        width: {
            if (vertical) {
                return expanded ? _labeledButtonWidth : Theme.itemSizeLarge
            } else {
                return Theme.itemSizeLarge * 3
            }
        }

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

        IconButton {
            id: expandButton

            enabled: !overlayLoader.active
            icon.source: "../images/icon-m-expand.svg"
            icon.sourceSize.height: Theme.iconSizeLarge
            icon.sourceSize.width: Theme.iconSizeLarge
            parent: vertical ? toolbar : mainButtons
            x: vertical ? 0 : 0
            y: vertical ? toolbar.height - height - Theme.paddingSmall : 0
            height: Theme.itemSizeLarge
            width: Theme.itemSizeLarge
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

        ToolbarButton {
            id: hintButton
            //% "Hint"
            text: qsTrId("patience-bt-hint")
            imageSource: "../images/icon-m-hint.svg"
            parent: vertical ? mainButtons : extraButtons
            showText: !vertical || expanded || animating
            onClicked: Patience.getHint()
        }
    }

    Flow {
        id: extraButtons

        x: vertical ? _labeledButtonWidth : Theme.horizontalPageMargin
        y: mainButtons.y + (vertical ? 0 : mainButtons.height)
        width: vertical ? _labeledButtonWidth : (parent.width - 2 * Theme.horizontalPageMargin)
        spacing: vertical? 0 : (width - hintButton.width - dealButton.width - restartButton.width) / 2
        visible: expanded || animating

        ToolbarButton {
            id: restartButton
            //% "Restart"
            text: qsTrId("patience-bt-restart")
            imageSource: "../images/icon-m-restart.svg"
            onClicked: {
                expanded = false
                Patience.restartGame()
            }
        }

        ToolbarButton {
            id: dealButton
            //% "Deal"
            text: qsTrId("patience-bt-deal")
            imageSource: "../images/icon-m-deal.svg"
            enabled: Patience.canDeal
            onClicked: Patience.dealCard()
        }
    }

    Label {
        id: title

        // These apply to !vertical
        readonly property int maximumWidth: parent.width - minimumX - Theme.horizontalPageMargin
        readonly property int minimumX: mainButtons.x + mainButtons.width + Theme.paddingSmall

        text: Patience.gameName
        color: Theme.highlightColor
        font.pixelSize: Theme.fontSizeLarge
        truncationMode: TruncationMode.Fade
        verticalAlignment: Text.AlignVCenter
        height: vertical ? Theme.itemSizeMedium : Theme.itemSizeLarge
        width: Math.min(contentWidth, vertical ? parent.width : maximumWidth)
        x: vertical ? Theme.paddingSmall : Math.max(minimumX, parent.width - width - Theme.horizontalPageMargin)
    }
}
