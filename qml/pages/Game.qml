/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2024 Tomi Leppänen
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

import QtFeedback 5.0
import QtQml 2.2
import QtQuick 2.6
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import Nemo.KeepAlive 1.2
import Patience 1.0
import "../toolbar"
import "../components"

Page {
    id: page

    property bool active: page.status === PageStatus.Active
    property bool needsGameStart
    readonly property bool toolbarOnRight: settings.landscapeToolbarSide == "right"
                                       || (settings.landscapeToolbarSide != "left"
                                        && page.orientation == Orientation.LandscapeInverted)

    allowedOrientations: Orientation.All

    function resetHint() {
        message.hint = ""
    }

    function pauseOrResumeAnimation(pause) {
        if (table.animationPlaying) {
            table.animationPaused = pause
        }
    }

    onOrientationChanged: {
        message.x = 0
        if (Patience.state === Patience.WonState) {
            if (table.animationPlaying) {
                table.animateAfterOrientationChange = true
                table.stopAnimation()
            } else {
                table.playWinAnimation()
            }
        }
    }

    onActiveChanged: {
        Patience.paused = !active
        resetHint()
        if (needsGameStart) {
            Patience.startNewGame()
            needsGameStart = false
        }
        pauseOrResumeAnimation(!active)
    }

    Connections {
        target: Qt.application
        onStateChanged: {
            if (Qt.application.state === Qt.ApplicationActive) {
                Patience.paused = !page.active
                pauseOrResumeAnimation(!page.active)
            } else {
                Patience.paused = true
                pauseOrResumeAnimation(true)
                table.cancelDrag()
            }
            resetHint()
        }
    }

    SilicaFlickable {
        anchors.fill: parent

        states: [
            State {
                name: "landscape"
                when: page.isLandscape && !page.toolbarOnRight
                extend: "landscape-base"
                PropertyChanges { target: tableContainer; x: toolbar.width }
                AnchorChanges { target: messageBar; anchors.left: toolbar.right }
            },
            State {
                name: "landscape mirrored"
                when: page.isLandscape && page.toolbarOnRight
                extend: "landscape-base"
                PropertyChanges {
                    target: toolbar
                    mirror: true
                    x: Screen.height - toolbar.width
                }
                AnchorChanges { target: messageBar; anchors.right: toolbar.left }
            },
            State {
                name: "landscape-base"
                when: { return false }
                PropertyChanges { target: toolbar; landscape: true }
                PropertyChanges {
                    target: tableContainer
                    x: { return 0 }
                    y: { return 0 }
                    height: Screen.width - messageBar.height
                    width: Screen.height - toolbar.width
                }
                PropertyChanges {
                    target: table
                    height: Screen.width - messageBar.height
                    width: Screen.height - toolbar.totalSpaceX
                    horizontalMargin: Theme.paddingLarge
                    verticalMargin: Theme.paddingSmall
                }
            }
        ]

        PullDownMenu {
            id: pullDownMenu

            MenuItem {
                //% "About"
                text: qsTrId("patience-me-about")
                onClicked: pageStack.push(Qt.resolvedUrl("about/AboutPage.qml"))
            }

            MenuItem {
                //% "Select game"
                text: qsTrId("patience-me-select_game")
                enabled: !Patience.engineFailed
                onClicked: pageStack.push(Qt.resolvedUrl("SelectGame.qml"))
            }

            MenuItem {
                //% "Options & Rules"
                text: qsTrId("patience-me-game_options")
                enabled: !Patience.engineFailed
                onClicked: pageStack.push(Qt.resolvedUrl("OptionsAndRules.qml"))
            }
        }

        contentHeight: parent.height

        Toolbar {
            id: toolbar

            enabled: !Patience.engineFailed
            landscape: false
            z: 10
        }

        Item {
            id: tableContainer

            clip: toolbar.expanded || toolbar.animating || toolbar.magnify
            x: 0
            y: toolbar.height
            height: Screen.height - messageBar.height - toolbar.height
            width: Screen.width

            Table {
                id: table

                property bool animateAfterOrientationChange

                enabled: Patience.state < Patience.GameOverState && !Patience.engineFailed && !toolbar.magnify

                height: Screen.height - toolbar.totalSpaceY - messageBar.height
                width: Screen.width
                anchors.horizontalCenter: parent.horizontalCenter

                minimumSideMargin: Theme.horizontalPageMargin
                horizontalMargin: Theme.paddingSmall
                maximumHorizontalMargin: Theme.paddingLarge
                verticalMargin: Theme.paddingLarge
                maximumVerticalMargin: Theme.paddingLarge

                backgroundColor: settings.backgroundColor
                highlightColor: Theme.rgba(Theme.highlightColor, Theme.opacityLow)

                transform: Scale { id: magnifyTransform }
                doubleResolution: magnifyArea.pressed || animateShrink.running || animateGrow.running

                onAnimationPlayingChanged: {
                    if (!animationPlaying && animateAfterOrientationChange) {
                        playWinAnimation()
                        animateAfterOrientationChange = false
                    }
                }

                layer.enabled: pullDownMenu.active || (overlayLoader.active && !animationPlaying)

                FeedbackEvent.onClicked: feedback.playEffect()
                FeedbackEvent.onDropSucceeded: feedback.playEffect()
                FeedbackEvent.onDropFailed: feedback.playEffect(true)
                FeedbackEvent.onSelectionChanged: feedback.playEffect(true)

                Component.onCompleted: Patience.restoreSavedOrLoad("klondike.scm")

                NumberAnimation {
                    id: animateShrink
                    target: magnifyTransform
                    properties: "xScale,yScale"
                    to: 1
                    duration: 100
                    easing.type: Easing.InOutQuad
                }

                NumberAnimation {
                    id: animateGrow
                    target: magnifyTransform
                    properties: "xScale,yScale"
                    to: 2
                    duration: 100
                    easing.type: Easing.InOutQuad
                }
            }

            MouseArea {
                id: magnifyArea
                anchors {
                    left: table.left
                    top: table.top
                    right: table.right
                    bottom: table.bottom
                }
                enabled: toolbar.magnify
                preventStealing: true
                onPressed: {
                    clickTimer.running = true
                    if (magnifyTransform.xScale < 2) {
                        animateGrow.running = true
                    }
                }
                onReleased: if (magnifyTransform.xScale === 2 || !clickTimer.running) animateShrink.running = true
                onEnabledChanged: if (!enabled) animateShrink.running = true

                Timer {
                    id: clickTimer
                    interval: 100
                }

                Binding {
                    target: magnifyTransform
                    property: "origin.x"
                    value: Math.floor(magnifyArea.mouseX)
                    when: magnifyArea.pressed && (magnifyTransform.xScale === 1 || !clickTimer.running)
                }

                Binding {
                    target: magnifyTransform
                    property: "origin.y"
                    value: Math.floor(magnifyArea.mouseY)
                    when: magnifyArea.pressed && (magnifyTransform.xScale === 1 || !clickTimer.running)
                }
            }
        }

        MouseArea {
            id: messageBar

            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            height: message.height
            clip: message.contentWidth > width

            drag {
                target: message
                axis: Drag.XAxis
                minimumX: Math.min(width - message.contentWidth, 0)
                maximumX: 0
                threshold: 0
            }

            Label {
                id: message
                property string hint
                text: hint !== "" && hintTimer.running ? hint : Patience.message
                color: Theme.highlightColor
                anchors.bottom: parent.bottom
                onTextChanged: x = 0

                Timer {
                    id: hintTimer
                    interval: 60*1000
                }
            }
        }

        Loader {
            id: overlayLoader

            active: Patience.state === Patience.WonState || Patience.state === Patience.GameOverState
            source: "../components/GameOverOverlay.qml"
            x: tableContainer.x
            y: tableContainer.y
            height: tableContainer.height
            width: tableContainer.width
            z: 5
        }

        Loader {
            id: failureOverlayLoader
            active: Patience.engineFailed
            source: "../components/EngineFailureOverlay.qml"
            x: tableContainer.x
            y: tableContainer.y
            height: tableContainer.height
            width: tableContainer.width
            z: 10
        }
    }

    Connections {
        target: Patience
        onStateChanged: {
            if (Patience.state === Patience.LoadedState) {
                if (active) {
                    Patience.startNewGame()
                } else {
                    needsGameStart = true
                }
            } else if (Patience.state === Patience.StartingState) {
                resetHint()
            } else if (Patience.state === Patience.WonState) {
                table.playWinAnimation()
            }
        }
        onHint: {
            message.hint = hint
            hintTimer.restart()
        }
        onCardMoved: {
            resetHint()
            Patience.forgetPreviousGame()
        }
    }

    DisplayBlanking {
        preventBlanking: settings.preventBlanking && Patience.state === Patience.RunningState && !Patience.paused
    }

    ThemeEffect {
        id: feedback

        function playEffect(weak) {
            if (settings.feedbackEffects) {
                play(weak ? ThemeEffect.PressWeak : ThemeEffect.Press)
            }
        }
    }

    Settings { id: settings }

    TouchBlocker {
        anchors.fill: parent
        enabled: Patience.testMode
    }
}
