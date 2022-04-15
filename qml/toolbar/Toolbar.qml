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

import QtQuick 2.6
import Sailfish.Silica 1.0
import Patience 1.0

Item {
    id: toolbar

    property bool vertical
    property bool pageActive
    property bool expanded
    readonly property bool dragged: dragArea.drag.active
    readonly property bool animating: dragged || horizontalTransition.running
                                              || horizontalExpandedTransition.running
                                              || verticalTransition.running
                                              || verticalExpandedTransition.running
    readonly property int buttonCountHorizontal: Math.ceil(Screen.width / 2 / Theme.itemSizeLarge)
    readonly property int spaceY: handle.y
    readonly property int minimumSpaceY: Theme.itemSizeLarge
    readonly property int maximumSpaceY: extraButtons.y + extraButtons.height + Theme.paddingSmall
    readonly property bool showHandleY: buttonCountHorizontal < 5
    readonly property int totalSpaceY: minimumSpaceY + (showHandleY ? handleWidth : 0)
    readonly property int buttonCountVertical: Math.max(
                            Math.floor((Screen.width - titleVertical.height) / Theme.itemSizeLarge),
                            Patience.showDeal ? 5 : 4)
    readonly property int spaceX: handle.x
    readonly property int minimumSpaceX: Theme.itemSizeLarge
    readonly property int maximumSpaceX: Math.max(gameTitleVertical.contentWidth,
                                                  scoreTextVertical.contentWidth,
                                                  elapsedTextVertical.contentWidth,
                                                  undoButton.contentWidth,
                                                  redoButton.contentWidth,
                                                  hintButton.contentWidth,
                                                  dealButton.contentWidth,
                                                  restartButton.contentWidth) + Theme.paddingLarge
    readonly property int totalSpaceX: minimumSpaceX + handleWidth
    readonly property int handleWidth: Theme.itemSizeExtraSmall / 4
    readonly property int toolbarVelocity: Theme.dp(3000)

    height: totalSpaceY
    width: Screen.width
    state: "non-vertical"
    states: [
        State {
            name: "dragged"
            when: !vertical && dragged
            extend: "expanded"
            PropertyChanges { target: toolbar; height: spaceY + handleWidth }
            AnchorChanges { target: handle; anchors.bottom: undefined }
        },
        State {
            name: "expanded"
            when: !vertical && expanded
            extend: "non-vertical"
            PropertyChanges { target: toolbar; height: maximumSpaceY + handleWidth }
        },
        State {
            // To ensure that title is visible also on portrait when the app is started in landscape
            name: "non-vertical"
            when: !vertical
            PropertyChanges { target: title; visible: true }
        },
        State {
            name: "vertical dragged"
            when: vertical && dragged
            extend: "vertical expanded"
            PropertyChanges { target: toolbar; width: spaceX + handleWidth }
            AnchorChanges { target: handle; anchors.right: undefined }
        },
        State {
            name: "vertical expanded"
            when: vertical && expanded
            extend: "vertical"
            PropertyChanges { target: toolbar; width: maximumSpaceX + handleWidth }
            PropertyChanges { target: scoreTextVertical; x: Math.min(-scoreTextVertical.nameWidth + spaceX - minimumSpaceX, 0) }
            PropertyChanges { target: elapsedTextVertical; x: Math.min(-elapsedTextVertical.nameWidth + spaceX - minimumSpaceX, 0) }
            PropertyChanges { target: mainButtons; width: maximumSpaceX }
        },
        State {
            name: "vertical"
            when: vertical

            PropertyChanges {
                target: toolbar

                height: Screen.width
                width: totalSpaceX
            }
            PropertyChanges {
                target: handle

                height: toolbar.height
                width: handleWidth
                visible: true
            }
            AnchorChanges {
                target: handleIcon

                anchors {
                    bottom: undefined
                    right: parent.right
                    horizontalCenter: undefined
                    verticalCenter: parent.verticalCenter
                }
            }
            PropertyChanges {
                target: handleIcon

                anchors.rightMargin: (handleIcon.height - handleIcon.width) / 2
                rotation: 90
            }
            PropertyChanges {
                target: handleBorderTop

                height: Screen.width
                width: 1
            }
            PropertyChanges {
                target: handleBorderBottom

                height: Screen.width
                width: 1
            }
            PropertyChanges {
                target: toolbarArea

                anchors {
                    rightMargin: handleWidth
                    bottomMargin: { return 0 }
                }
            }
            PropertyChanges { target: dragArea; drag.axis: Drag.XAxis; enabled: true }
            PropertyChanges { target: title; visible: false }
            PropertyChanges { target: titleVertical; visible: true }
            PropertyChanges {
                target: mainButtonsContainer

                height: {
                    var spaceAvailable = toolbarArea.height - titleVertical.height
                    if (spaceAvailable > Theme.itemSizeLarge * buttonCountVertical) {
                        return Theme.itemSizeLarge * buttonCountVertical
                    }
                    var minItemSize = 0.2 * Theme.itemSizeLarge
                    var maxItemSize = Theme.itemSizeLarge - minItemSize
                    var lastButtonSpace = spaceAvailable % Theme.itemSizeLarge
                    if (lastButtonSpace < minItemSize + Theme.paddingSmall) {
                        return spaceAvailable - lastButtonSpace - minItemSize
                    } else if (lastButtonSpace > maxItemSize - Theme.paddingSmall) {
                        return spaceAvailable - (lastButtonSpace - maxItemSize) - Theme.paddingSmall
                    } else {
                        return spaceAvailable - Theme.paddingSmall
                    }
                }
                enabled: true
                clip: height < mainButtons.height
            }
            AnchorChanges {
                target: mainButtonsContainer

                anchors {
                    top: undefined
                    bottom: parent.bottom
                }
            }
            PropertyChanges {
                target: mainButtons

                height: Theme.itemSizeLarge * buttonCountVertical
                width: minimumSpaceX
                x: { return 0 }
                y: mainButtons.y
            }
            PropertyChanges { target: extraButtons; visible: false }
            PropertyChanges { target: undoButton; showText: expanded || animating; parent: mainButtons }
            PropertyChanges { target: redoButton; showText: expanded || animating; parent: mainButtons }
            PropertyChanges { target: hintButton; showText: expanded || animating; parent: mainButtons }
            PropertyChanges { target: dealButton; showText: expanded || animating; parent: mainButtons }
            PropertyChanges { target: restartButton; showText: expanded || animating; parent: mainButtons }
        }
    ]
    transitions: [
        Transition {
            id: horizontalTransition
            from: "dragged"
            to: ""

            ParallelAnimation {
                SmoothedAnimation {
                    target: toolbar
                    properties: "height"
                    from: spaceY + handleWidth
                    to: minimumSpaceY + handleWidth
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    target: handle
                    properties: "y"
                    from: spaceY
                    to: minimumSpaceY
                    velocity: toolbarVelocity
                }
            }
        },
        Transition {
            id: horizontalExpandedTransition
            from: "dragged"
            to: "expanded"

            ParallelAnimation {
                SmoothedAnimation {
                    target: toolbar
                    properties: "height"
                    from: spaceY + handleWidth
                    to: maximumSpaceY + handleWidth
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    target: handle
                    properties: "y"
                    from: spaceY
                    to: maximumSpaceY
                    velocity: toolbarVelocity
                }
            }
        },
        Transition {
            id: verticalTransition
            from: "vertical dragged"
            to: "vertical"

            ParallelAnimation {
                SmoothedAnimation {
                    target: toolbar
                    properties: "width"
                    from: spaceX + handleWidth
                    to: minimumSpaceX + handleWidth
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    target: handle
                    properties: "x"
                    from: spaceX
                    to: minimumSpaceX
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    target: titleVertical
                    properties: "width"
                    from: spaceX - Theme.paddingSmall
                    to: minimumSpaceX - Theme.paddingSmall
                    velocity: toolbarVelocity
                }
                SequentialAnimation {
                    SmoothedAnimation {
                        from: spaceX
                        to: Math.max(scoreTextVertical.contentWidth, spaceX)
                        velocity: toolbarVelocity
                    }
                    SmoothedAnimation {
                        target: scoreTextVertical
                        properties: "x"
                        from: Math.min(-scoreTextVertical.nameWidth + spaceX - minimumSpaceX, 0)
                        to: -scoreTextVertical.nameWidth
                        velocity: toolbarVelocity
                    }
                }
                SequentialAnimation {
                    SmoothedAnimation {
                        from: spaceX
                        to: Math.max(elapsedTextVertical.contentWidth, spaceX)
                        velocity: toolbarVelocity
                    }
                    SmoothedAnimation {
                        target: elapsedTextVertical
                        properties: "x"
                        from: Math.min(-elapsedTextVertical.nameWidth + spaceX - minimumSpaceX, 0)
                        to: -elapsedTextVertical.nameWidth
                        velocity: toolbarVelocity
                    }
                }
            }
        },
        Transition {
            id: verticalExpandedTransition
            from: "vertical dragged"
            to: "vertical expanded"

            ParallelAnimation {
                SmoothedAnimation {
                    target: toolbar
                    properties: "width"
                    from: spaceX + handleWidth
                    to: maximumSpaceX + handleWidth
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    target: handle
                    properties: "x"
                    from: spaceX
                    to: maximumSpaceX
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    target: titleVertical
                    properties: "width"
                    from: spaceX - Theme.paddingSmall * 2
                    to: maximumSpaceX - Theme.paddingSmall * 2
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    target: scoreTextVertical
                    properties: "x"
                    from: Math.min(-scoreTextVertical.nameWidth + spaceX - minimumSpaceX, 0)
                    to: Math.min(-scoreTextVertical.nameWidth + maximumSpaceX - minimumSpaceX, 0)
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    target: elapsedTextVertical
                    properties: "x"
                    from: Math.min(-elapsedTextVertical.nameWidth + spaceX - minimumSpaceX, 0)
                    to: Math.min(-elapsedTextVertical.nameWidth + maximumSpaceX - minimumSpaceX, 0)
                    velocity: toolbarVelocity
                }
            }
        }
    ]

    MouseArea {
        id: dragArea
        anchors.fill: parent
        drag {
            target: handle
            axis: Drag.YAxis
            minimumY: minimumSpaceY
            maximumY: maximumSpaceY
            minimumX: minimumSpaceX
            maximumX: maximumSpaceX
            filterChildren: true
        }
        enabled: buttonCountHorizontal < 5

        MouseArea {
            id: toolbarArea

            anchors {
                fill: parent
                bottomMargin: handleWidth
            }
            clip: animating

            MouseArea {
                id: mainButtonsContainer

                anchors {
                    left: parent.left
                    top: parent.top
                }
                height: mainButtons.height
                width: mainButtons.width
                drag {
                    target: mainButtons
                    axis: Drag.YAxis
                    minimumY: height - mainButtons.height
                    maximumY: 0
                    filterChildren: true
                }

                Flow {
                    id: mainButtons

                    // Button graphics have some padding, thus remove some of that page margin
                    x: Theme.horizontalPageMargin - Theme.paddingMedium
                    y: 0
                    height: minimumSpaceY
                    width: Theme.itemSizeLarge * buttonCountHorizontal

                    ToolbarButton {
                        id: undoButton

                        //% "Undo"
                        text: qsTrId("patience-bt-undo")
                        imageSource: "../../buttons/icon-m-undo.svg"
                        showText: buttonCountHorizontal < 1
                        parent: buttonCountHorizontal >= 1 ? mainButtons : extraButtons
                        disabled: !Patience.canUndo
                        onClicked: if (!disabled) Patience.undoMove()
                    }

                    ToolbarButton {
                        id: redoButton

                        //% "Redo"
                        text: qsTrId("patience-bt-redo")
                        imageSource: "../../buttons/icon-m-redo.svg"
                        showText: buttonCountHorizontal < 2
                        parent: buttonCountHorizontal >= 2 ? mainButtons : extraButtons
                        disabled: !Patience.canRedo
                        onClicked: if (!disabled) Patience.redoMove()
                    }

                    ToolbarButton {
                        id: hintButton

                        //% "Hint"
                        text: qsTrId("patience-bt-hint")
                        imageSource: "../../buttons/icon-m-hint.svg"
                        showText: buttonCountHorizontal < 3
                        parent: buttonCountHorizontal >= 3 ? mainButtons : extraButtons
                        disabled: Patience.state !== Patience.StartingState && Patience.state !== Patience.RunningState
                        onClicked: if (!disabled) Patience.getHint()
                    }

                    ToolbarButton {
                        id: dealButton

                        //% "Deal"
                        text: qsTrId("patience-bt-deal")
                        imageSource: "../../buttons/icon-m-deal.svg"
                        showText: buttonCountHorizontal < 4
                        parent: buttonCountHorizontal >= 4 ? mainButtons : extraButtons
                        disabled: !Patience.canDeal
                        visible: Patience.showDeal
                        onClicked: if (!disabled) Patience.dealCard()
                    }

                    ToolbarButton {
                        id: restartButton

                        //% "Restart"
                        text: qsTrId("patience-bt-restart")
                        imageSource: "../../buttons/icon-m-restart.svg"
                        showText: buttonCountHorizontal < 5
                        parent: buttonCountHorizontal >= 5 ? mainButtons : extraButtons
                        onClicked: Patience.restartGame()
                    }
                }
            }

            Flow {
                id: extraButtons

                // Button graphics have some padding, thus remove some of that page margin
                x: Theme.horizontalPageMargin - Theme.paddingMedium
                y: mainButtonsContainer.y + mainButtons.height
                width: Screen.width - x - Theme.horizontalPageMargin
                visible: expanded || animating
            }

            Item {
                id: title

                height: minimumSpaceY
                width: Screen.width - 2 * Theme.horizontalPageMargin + Theme.paddingMedium - Theme.itemSizeLarge * buttonCountHorizontal - Theme.paddingSmall
                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    top: parent.top
                }
                clip: width < gameTitle.contentWidth

                Label {
                    id: gameTitle

                    text: Patience.gameName
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    verticalAlignment: Text.AlignBottom
                    truncationMode: parent.width < contentWidth ? TruncationMode.Fade : TruncationMode.None
                    width: Math.min(parent.width, contentWidth)
                    x: title.width - width
                    y: scoreText.y - height
                }


                ScoreText {
                    id: scoreText

                    //% "Score:"
                    text: qsTrId("patience-la-score")
                    value: Patience.score
                    anchors.bottom: parent.bottom
                    x: parent.width - width - elapsedText.width - spacer.width - 2 * Theme.paddingSmall
                    maximumWidth: title.width - elapsedText.width - spacer.width - 3 * Theme.paddingSmall
                    nameVisible: false
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
                    visible: Patience.showScore
                }

                ScoreText {
                    id: elapsedText

                    //% "Time:"
                    text: qsTrId("patience-la-time")
                    value: Patience.elapsedTime
                    anchors.bottom: parent.bottom
                    x: parent.width - width
                    maximumWidth: title.width
                    nameVisible: false
                }
            }

            Item {
                id: titleVertical

                visible: false
                height: gameTitleVertical.height + (Patience.showScore ? scoreTextVertical.height : 0)
                                                 + elapsedTextVertical.height
                width: spaceX - Theme.paddingSmall
                x: Theme.paddingSmall
                anchors.top: parent.top
                clip: width < gameTitleVertical.contentWidth

                Label {
                    id: gameTitleVertical

                    text: Patience.gameName
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    verticalAlignment: Text.AlignBottom
                    truncationMode: TruncationMode.None
                    x: 0
                    y: 0
                }


                ScoreText {
                    id: scoreTextVertical

                    //% "Score:"
                    text: qsTrId("patience-la-score")
                    value: Patience.score
                    anchors.bottom: elapsedTextVertical.top
                    x: -scoreTextVertical.nameWidth + spaceX - minimumSpaceX
                    truncationMode: TruncationMode.None
                    nameVisible: true
                    visible: Patience.showScore
                }

                ScoreText {
                    id: elapsedTextVertical

                    //% "Time:"
                    text: qsTrId("patience-la-time")
                    value: Patience.elapsedTime
                    anchors.bottom: parent.bottom
                    x: -elapsedTextVertical.nameWidth + spaceX - minimumSpaceX
                    truncationMode: TruncationMode.None
                    nameVisible: true
                }
            }
        }
    }

    Item {
        id: handle

        property int prevX: x
        property int prevY: y

        anchors {
            bottom: parent.bottom
            right: parent.right
        }
        height: handleWidth
        width: Screen.width
        visible: buttonCountHorizontal < 5

        onXChanged: {
            if (dragged) expanded = prevX < x
            prevX = x
        }

        onYChanged: {
            if (dragged) expanded = prevY < y
            prevY = y
        }

        Rectangle {
            id: handleBorderTop

            anchors {
                left: parent.left
                top: parent.top
            }
            height: 1
            width: Screen.width
            color: Theme.colorScheme === Theme.LightOnDark ? "white" : "black"
        }

        Icon {
            id: handleIcon

            source: "../../buttons/handle.svg"
            sourceSize.height: handleWidth
            sourceSize.width: handleWidth * (88 / 48)
            anchors {
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }
        }

        Rectangle {
            id: handleBorderBottom

            anchors {
                right: parent.right
                bottom: parent.bottom
            }
            height: 1
            width: Screen.width
            color: handleBorderTop.color
        }
    }
}
