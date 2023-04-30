/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021-2023 Tomi Leppänen
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
import "../components"

Item {
    id: toolbar

    property bool landscape
    property bool mirror
    property bool expanded
    property bool magnify
    property int prevX: x
    readonly property bool dragged: dragArea.drag.active
    readonly property bool animating: dragged || portraitTransition.running
                                              || portraitExpandedTransition.running
                                              || landscapeTransition.running
                                              || landscapeExpandedTransition.running
                                              || landscapeMirroredTransition.running
                                              || landscapeMirroredExpandedTransition.running
    readonly property int buttonCount: 6 + (Patience.showDeal ? 1 : 0)
    readonly property int buttonCountPortrait: Math.min(Math.ceil(Screen.width / 2 / Theme.itemSizeLarge),
                                                        buttonCount)
    readonly property int spaceY: handle.y
    readonly property int minimumSpaceY: Theme.itemSizeLarge
    readonly property int maximumSpaceY: extraButtons.y + extraButtons.height + Theme.paddingSmall
    readonly property bool showHandleY: buttonCountPortrait < buttonCount
    readonly property int totalSpaceY: minimumSpaceY + (showHandleY ? handleWidth : 0)
    readonly property int buttonCountLandscape: Math.max(
                            Math.floor((Screen.width - titleLandscape.height) / Theme.itemSizeLarge),
                            buttonCount)
    readonly property int spaceX: mirror ? Screen.height - toolbar.x - handleWidth : handle.x
    readonly property int minimumSpaceX: Theme.itemSizeLarge
    readonly property int maximumSpaceX: Math.max(gameTitleLandscape.contentWidth,
                                                  scoreTextLandscape.contentWidth,
                                                  elapsedTextLandscape.contentWidth,
                                                  undoButton.contentWidth,
                                                  redoButton.contentWidth,
                                                  hintButton.contentWidth,
                                                  dealButton.contentWidth,
                                                  restartButton.contentWidth,
                                                  newGameButton.contentWidth,
                                                  magnifyButton.contentWidth) + Theme.paddingLarge
    readonly property int totalSpaceX: minimumSpaceX + handleWidth
    readonly property int handleWidth: Theme.itemSizeExtraSmall / 4
    readonly property int toolbarVelocity: Theme.pixelRatio * 3000

    height: buttonCount, totalSpaceY
    width: Screen.width
    state: "portrait"
    states: [
        State {
            name: "dragged"
            when: !landscape && dragged
            extend: "expanded"
            PropertyChanges { target: toolbar; height: spaceY + handleWidth }
            AnchorChanges { target: handle; anchors.bottom: undefined }
        },
        State {
            name: "expanded"
            when: !landscape && expanded
            extend: "portrait"
            PropertyChanges { target: toolbar; height: buttonCount, maximumSpaceY + handleWidth }
        },
        State {
            // To ensure that title is visible also on portrait when the app is started in landscape
            name: "portrait"
            when: !landscape
            PropertyChanges { target: title; visible: true }
        },
        State {
            name: "landscape mirrored dragged"
            when: landscape && dragged && mirror
            extend: "landscape mirrored expanded"
            PropertyChanges { target: toolbar; width: Screen.height - toolbar.x }
        },
        State {
            name: "landscape mirrored expanded"
            when: landscape && expanded && mirror
            extend: "landscape mirrored"
            PropertyChanges { target: toolbar; width: maximumSpaceX + handleWidth }
            PropertyChanges { target: mainButtons; width: maximumSpaceX }
        },
        State {
            name: "landscape mirrored"
            when: landscape && mirror
            extend: "landscape"
            AnchorChanges {
                target: handle;
                anchors {
                    left: parent.left
                    right: undefined
                }
            }
            PropertyChanges {
                target: toolbarArea

                anchors {
                    leftMargin: handleWidth - dragArea.anchors.leftMargin
                    rightMargin: { return 0 }
                }
            }
            PropertyChanges {
                target: dragArea

                anchors {
                    leftMargin: -table.horizontalMargin
                    rightMargin: { return 0 }
                }
                drag {
                    target: toolbar
                    minimumX: Screen.height - maximumSpaceX - handleWidth
                    maximumX: Screen.height - minimumSpaceX - handleWidth
                }
            }
        },
        State {
            name: "landscape dragged"
            when: landscape && dragged && !mirror
            extend: "landscape expanded"
            PropertyChanges { target: toolbar; width: spaceX + handleWidth }
            AnchorChanges { target: handle; anchors.right: undefined }
        },
        State {
            name: "landscape expanded"
            when: landscape && expanded && !mirror
            extend: "landscape"
            PropertyChanges { target: toolbar; width: maximumSpaceX + handleWidth }
            PropertyChanges { target: mainButtons; width: maximumSpaceX }
        },
        State {
            name: "landscape"
            when: landscape && !mirror

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
                    rightMargin: handleWidth - dragArea.anchors.rightMargin
                    bottomMargin: { return 0 }
                }
            }
            PropertyChanges {
                target: dragArea;
                anchors {
                    bottomMargin: { return 0 }
                    rightMargin: -table.horizontalMargin
                }
                drag.axis: Drag.XAxis;
                enabled: true
            }
            PropertyChanges { target: title; visible: false }
            PropertyChanges { target: titleLandscape; visible: true }
            PropertyChanges {
                target: mainButtonsContainer

                height: {
                    var spaceAvailable = toolbarArea.height - titleLandscape.height
                    if (spaceAvailable > Theme.itemSizeLarge * buttonCountLandscape) {
                        return Theme.itemSizeLarge * buttonCountLandscape
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

                height: Theme.itemSizeLarge * buttonCountLandscape
                width: minimumSpaceX
                x: { return 0 }
                y: mainButtons.y
            }
            PropertyChanges { target: extraButtons; visible: false }
            PropertyChanges {
                target: undoButton
                showText: expanded || animating
                parent: mainButtons
                width: mainButtons.width
            }
            PropertyChanges {
                target: redoButton
                showText: expanded || animating
                parent: mainButtons
                width: mainButtons.width
            }
            PropertyChanges {
                target: hintButton
                showText: expanded || animating
                parent: mainButtons
                width: mainButtons.width
            }
            PropertyChanges {
                target: dealButton
                showText: expanded || animating
                parent: mainButtons
                width: mainButtons.width
            }
            PropertyChanges {
                target: restartButton
                showText: expanded || animating
                parent: mainButtons
                width: mainButtons.width
            }
            PropertyChanges {
                target: newGameButton
                showText: expanded || animating
                parent: mainButtons
                width: mainButtons.width
            }
            PropertyChanges {
                target: magnifyButton
                showText: expanded || animating
                parent: mainButtons
                width: mainButtons.width
            }
        }
    ]
    transitions: [
        Transition {
            id: portraitTransition
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
            id: portraitExpandedTransition
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
            id: landscapeTransition
            from: "landscape dragged"
            to: "landscape"

            LandscapeShrinkParallelAnimation {
                SmoothedAnimation {
                    easing.type: Easing.Linear
                    target: toolbar
                    properties: "width"
                    from: spaceX + handleWidth
                    to: minimumSpaceX + handleWidth
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    easing.type: Easing.Linear
                    target: handle
                    properties: "x"
                    from: spaceX
                    to: minimumSpaceX
                    velocity: toolbarVelocity
                }
            }
        },
        Transition {
            id: landscapeExpandedTransition
            from: "landscape dragged"
            to: "landscape expanded"

            LandscapeExpansionParallelAnimation {
                SmoothedAnimation {
                    easing.type: Easing.Linear
                    target: toolbar
                    properties: "width"
                    from: spaceX + handleWidth
                    to: maximumSpaceX + handleWidth
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    easing.type: Easing.Linear
                    target: handle
                    properties: "x"
                    from: spaceX
                    to: maximumSpaceX
                    velocity: toolbarVelocity
                }
            }
        },
        Transition {
            id: landscapeMirroredTransition
            from: "landscape mirrored dragged"
            to: "landscape mirrored"

            LandscapeShrinkParallelAnimation {
                SmoothedAnimation {
                    easing.type: Easing.Linear
                    target: toolbar
                    properties: "width"
                    from: Screen.height - toolbar.x
                    to: minimumSpaceX + handleWidth
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    easing.type: Easing.Linear
                    target: toolbar
                    properties: "x"
                    to: Screen.height - minimumSpaceX - handleWidth
                    velocity: toolbarVelocity
                }
            }
        },
        Transition {
            id: landscapeMirroredExpandedTransition
            from: "landscape mirrored dragged"
            to: "landscape mirrored expanded"

            LandscapeExpansionParallelAnimation {
                SmoothedAnimation {
                    easing.type: Easing.Linear
                    target: toolbar
                    properties: "width"
                    from: Screen.height - toolbar.x
                    to: maximumSpaceX + handleWidth
                    velocity: toolbarVelocity
                }
                SmoothedAnimation {
                    easing.type: Easing.Linear
                    target: toolbar
                    properties: "x"
                    to: Screen.height - maximumSpaceX - handleWidth
                    velocity: toolbarVelocity
                }
            }
        }
    ]

    onXChanged: {
        if (dragged) expanded = prevX > x
        prevX = x
    }

    MouseArea {
        id: dragArea
        anchors {
            fill: parent
            bottomMargin: -table.verticalMargin
        }
        drag {
            target: handle
            axis: Drag.YAxis
            minimumY: minimumSpaceY
            maximumY: maximumSpaceY
            minimumX: minimumSpaceX
            maximumX: maximumSpaceX
            filterChildren: true
        }
        enabled: showHandleY

        MouseArea {
            id: toolbarArea

            anchors {
                fill: parent
                bottomMargin: handleWidth - dragArea.anchors.bottomMargin
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

                    onMinimumYChanged: mainButtons.y = Math.min(Math.max(mainButtons.y, drag.minimumY), drag.maximumY)
                }

                Flow {
                    id: mainButtons

                    // Button graphics have some padding, thus remove some of that page margin
                    x: Theme.horizontalPageMargin - Theme.paddingMedium
                    y: 0
                    height: minimumSpaceY
                    width: Theme.itemSizeLarge * buttonCountPortrait

                    ToolbarButton {
                        id: undoButton

                        //% "Undo"
                        text: qsTrId("patience-bt-undo")
                        imageSource: (!Patience.canUndo && Patience.previousGameStored)
                            ? "../../buttons/icon-m-restore.svg"
                            : "../../buttons/icon-m-undo.svg"
                        showText: buttonCountPortrait < 1
                        parent: buttonCountPortrait >= 1 ? mainButtons : extraButtons
                        disabled: !Patience.canUndo && !Patience.previousGameStored
                        onActionTriggered: {
                            if (Patience.canUndo) {
                                Patience.undoMove()
                            } else if (Patience.previousGameStored) {
                                Patience.restorePreviousGame()
                            }
                        }
                    }

                    ToolbarButton {
                        id: redoButton

                        //% "Redo"
                        text: qsTrId("patience-bt-redo")
                        imageSource: "../../buttons/icon-m-redo.svg"
                        showText: buttonCountPortrait < 2
                        parent: undoButton.parent, buttonCountPortrait >= 2 ? mainButtons : extraButtons
                        disabled: !Patience.canRedo
                        onActionTriggered: Patience.redoMove()
                    }

                    ToolbarButton {
                        id: hintButton

                        //% "Hint"
                        text: qsTrId("patience-bt-hint")
                        imageSource: "../../buttons/icon-m-hint.svg"
                        showText: buttonCountPortrait < 3
                        parent: redoButton.parent, buttonCountPortrait >= 3 ? mainButtons : extraButtons
                        disabled: Patience.state !== Patience.StartingState && Patience.state !== Patience.RunningState
                        onActionTriggered: Patience.getHint()
                    }

                    ToolbarButton {
                        id: dealButton

                        //% "Deal"
                        text: qsTrId("patience-bt-deal")
                        imageSource: "../../buttons/icon-m-deal.svg"
                        showText: buttonCountPortrait < 4
                        parent: hintButton.parent, buttonCountPortrait >= 4 ? mainButtons : extraButtons
                        disabled: !Patience.canDeal
                        visible: Patience.showDeal
                        onActionTriggered: Patience.dealCard()
                    }

                    ToolbarButton {
                        id: restartButton

                        //% "Restart"
                        text: qsTrId("patience-bt-restart")
                        imageSource: "../../buttons/icon-m-restart.svg"
                        showText: buttonCountPortrait < (Patience.showDeal ? 5 : 4)
                        parent: dealButton.parent, buttonCountPortrait >= (Patience.showDeal ? 5 : 4) ? mainButtons : extraButtons
                        disabled: !Patience.canRestart
                        onActionTriggered: Patience.restartGame()
                    }

                    ToolbarButton {
                        id: newGameButton

                        //% "New game"
                        text: qsTrId("patience-bt-new_game")
                        imageSource: "../../buttons/icon-m-new.svg"
                        showText: buttonCountPortrait < (Patience.showDeal ? 6 : 5)
                        parent: restartButton.parent, buttonCountPortrait >= (Patience.showDeal ? 6 : 5) ? mainButtons : extraButtons
                        onActionTriggered: Patience.startNewGame()
                    }

                    ToolbarButton {
                        id: magnifyButton

                        //% "Magnify"
                        text: qsTrId("patience-bt-magnify")
                        highlighted: (magnify && !down) || (!magnify && down)
                        imageSource: "../../buttons/icon-m-magnify.svg"
                        showText: buttonCountPortrait < buttonCount
                        parent: newGameButton.parent, buttonCountPortrait >= buttonCount ? mainButtons : extraButtons
                        onActionTriggered: magnify = !magnify
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
                width: Screen.width - 2 * Theme.horizontalPageMargin + Theme.paddingMedium - Theme.itemSizeLarge * buttonCountPortrait - Theme.paddingSmall
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
                    nameOpacity: expanded || animating ? 1.0 : 0.0
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
                    nameOpacity: expanded || animating ? 1.0 : 0.0
                }
            }

            Item {
                id: titleLandscape

                visible: false
                height: gameTitleLandscape.height + (Patience.showScore ? scoreTextLandscape.height : 0)
                                                 + elapsedTextLandscape.height
                width: spaceX - Theme.paddingSmall
                x: Theme.paddingSmall
                anchors.top: parent.top
                clip: width < gameTitleLandscape.contentWidth

                Label {
                    id: gameTitleLandscape

                    text: Patience.gameName
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    verticalAlignment: Text.AlignBottom
                    truncationMode: TruncationMode.None
                    x: 0
                    y: 0
                }


                ScoreText {
                    id: scoreTextLandscape

                    //% "Score:"
                    text: qsTrId("patience-la-score")
                    value: Patience.score
                    anchors.bottom: elapsedTextLandscape.top
                    x: Math.min(-scoreTextLandscape.nameWidth + spaceX - minimumSpaceX, 0)
                    truncationMode: TruncationMode.None
                    nameVisible: true
                    nameOpacity: expanded || animating ? 1.0 : 0.0
                    visible: Patience.showScore
                }

                ScoreText {
                    id: elapsedTextLandscape

                    //% "Time:"
                    text: qsTrId("patience-la-time")
                    value: Patience.elapsedTime
                    anchors.bottom: parent.bottom
                    x: Math.min(-elapsedTextLandscape.nameWidth + spaceX - minimumSpaceX, 0)
                    truncationMode: TruncationMode.None
                    nameVisible: true
                    nameOpacity: expanded || animating ? 1.0 : 0.0
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
        visible: showHandleY

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
