import QtQuick 2.6
import Sailfish.Silica 1.0
import Patience 1.0

Page {
    id: page

    allowedOrientations: Orientation.All
    property bool isPortrait: orientation & Orientation.PortraitMask

    SilicaFlickable {
        anchors.fill: parent

        PullDownMenu {
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }

            MenuItem {
                text: qsTr("Select game")
                onClicked: pageStack.push(Qt.resolvedUrl("SelectGame.qml"))
            }
        }

        contentHeight: column.height

        Column {
            id: column

            width: page.width

            PageHeader {
                id: header
                title: Patience.gameName

                Row {
                    anchors {
                        left: parent.left
                        leftMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    spacing: Theme.paddingSmall

                    IconButton {
                        icon.source: "../images/icon-m-undo.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        enabled: Patience.canUndo
                        onClicked: Patience.undoMove()
                    }

                    IconButton {
                        icon.source: "../images/icon-m-redo.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        enabled: Patience.canRedo
                        onClicked: Patience.redoMove()
                    }

                    IconButton {
                        icon.source: "../images/icon-m-deal.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        enabled: Patience.canDeal
                        onClicked: Patience.dealCard()
                        visible: false // TODO
                    }

                    IconButton {
                        icon.source: "../images/icon-m-hint.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        onClicked: Patience.hint()
                        visible: false // TODO
                    }

                    IconButton {
                        icon.source: "../images/icon-m-restart.svg"
                        icon.height: Theme.itemSizeSmall
                        icon.width: Theme.itemSizeSmall
                        onClicked: Patience.restartGame()
                    }
                }
            }

            Table {
                height: page.height - header.height - message.height
                width: parent.width
                minimumSideMargin: Theme.horizontalPageMargin
                horizontalMargin: isPortrait ? Theme.paddingSmall : Theme.paddingLarge
                maximumHorizontalMargin: Theme.paddingLarge
                verticalMargin: isPortrait ? Theme.paddingLarge : Theme.paddingSmall
                maximumVerticalMargin: Theme.paddingLarge
                Component.onCompleted: Patience.loadGame("klondike.scm")
            }

            Label {
                id: message
                anchors {
                    left: parent.left
                    right: parent.right
                }
                text: Patience.message
            }
        }
    }

    Connections {
        target: Patience
        onStateChanged: {
            if (Patience.state === Patience.LoadedState) {
                Patience.startNewGame()
            }
        }
    }
}
