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
    property bool pageActive
    property bool expanded
    readonly property bool animating: heightAnimation.running || widthAnimation.running
    readonly property int _labeledButtonWidth: Theme.itemSizeLarge * 2
    readonly property int buttonCount: {
        if (vertical) {
            var buttonSpace = parent.height - title.verticalHeight
            return buttonSpace / Theme.itemSizeLarge
        } else {
            return 3
        }
    }

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
        enabled: !vertical && !orientationTransitionRunning && pageActive

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
        enabled: vertical && !orientationTransitionRunning && pageActive

        NumberAnimation {
            id: widthAnimation
            duration: 100
        }
    }
    clip: animating

    Flow {
        id: mainButtons

        x: vertical ? 0 : Theme.horizontalPageMargin
        y: vertical ? title.y + title.height : 0
        height: vertical ? Theme.itemSizeLarge * (buttonCount - 1) : Theme.itemSizeLarge
        width: {
            if (vertical) {
                return expanded ? _labeledButtonWidth : Theme.itemSizeLarge
            } else {
                return Theme.itemSizeLarge * buttonCount
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
            y: vertical ? toolbar.height - height : 0
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
                enabled: !orientationTransitionRunning && pageActive
                NumberAnimation { duration: 100 }
            }
            onClicked: expanded = !expanded
        }

        ToolbarButton {
            id: hintButton
            //% "Hint"
            text: qsTrId("patience-bt-hint")
            imageSource: "../images/icon-m-hint.svg"
            parent: vertical && buttonCount >= 4 ? mainButtons : extraButtons
            showText: !vertical || expanded || animating
            enabled: Patience.state === Patience.StartingState || Patience.state === Patience.RunningState
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
            visible: Patience.showDeal
            onClicked: Patience.dealCard()
        }
    }

    Item {
        id: title

        // These apply to !vertical
        readonly property int maximumWidth: parent.width - minimumX - Theme.horizontalPageMargin
        readonly property int minimumX: mainButtons.x + mainButtons.width + Theme.paddingSmall
        readonly property int verticalHeight: gameTitle.height
                                            + (Patience.showScore ? scoreText.height : 0)
                                            + elapsedText.height

        height: vertical ? verticalHeight : Theme.itemSizeLarge
        width: vertical ? parent.width - Theme.paddingSmall * 2 : maximumWidth
        x: vertical ? Theme.paddingSmall : minimumX
        anchors {
            top: parent.top
            topMargin: {
                if (vertical) {
                    var remainingSpace = parent.height - (height + buttonCount * Theme.itemSizeLarge)
                    if (remainingSpace > Theme.paddingMedium) {
                        return Theme.paddingMedium
                    } else {
                        return remainingSpace
                    }
                } else {
                    return 0
                }
            }
        }

        Label {
            id: gameTitle

            text: Patience.gameName
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeLarge
            truncationMode: TruncationMode.Fade
            verticalAlignment: Text.AlignBottom
            width: Math.min(parent.width, contentWidth)
            x: vertical ? 0 : parent.width - width
            y: vertical ? 0 : scoreText.y - height
        }


        ScoreText {
            id: scoreText

            //% "Score:"
            text: qsTrId("patience-la-score")
            value: Patience.score
            nameVisible: vertical
            anchors.bottom: vertical ? elapsedText.top : parent.bottom
            x: {
                if (vertical) {
                    return expanded ? 0 : -nameWidth
                } else {
                    return parent.width - width - elapsedText.width - spacer.width - 2 * Theme.paddingSmall
                }
            }
            maximumWidth: {
                if (vertical) {
                    return parent.width
                } else {
                    return title.maximumWidth - elapsedText.width - spacer.width - 3 * Theme.paddingSmall
                }
            }
            visible: Patience.showScore
        }

        Label {
            id: spacer
            text: "\u2022"
            color: Theme.highlightColor
            anchors {
                bottom: parent.bottom
                right: elapsedText.left
                rightMargin: Theme.paddingSmall
            }
            visible: !vertical && Patience.showScore
        }

        ScoreText {
            id: elapsedText

            //% "Time:"
            text: qsTrId("patience-la-time")
            value: Patience.elapsedTime
            nameVisible: vertical
            anchors.bottom: parent.bottom
            x: {
                if (vertical) {
                    return expanded ? 0 : -nameWidth
                } else {
                    return parent.width - width
                }
            }
            maximumWidth: vertical ? parent.width : title.maximumWidth
        }
    }
}
