import QtQuick 2.0
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

                    IconButton {
                        icon.source: "image://theme/icon-m-back"
                        enabled: Patience.canUndo
                        onClicked: Patience.undoMove()
                    }

                    IconButton {
                        icon.source: "image://theme/icon-m-forward"
                        enabled: Patience.canRedo
                        onClicked: Patience.redoMove()
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
