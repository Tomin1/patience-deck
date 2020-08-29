import QtQuick 2.0
import Sailfish.Silica 1.0
import Aisleriot 1.0

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
                title: qsTr("Aisleriot")
            }
            Board {
                height: page.height - header.height
                width: parent.width
                horizontalMargin: isPortrait ? Theme.paddingSmall : Theme.paddingLarge;
                verticalMargin: isPortrait ? Theme.paddingLarge : Theme.paddingSmall;
            }
        }
    }

    Connections {
        target: Aisleriot
        onStateChanged: {
            if (Aisleriot.state === Aisleriot.LoadedState) {
                Aisleriot.startNewGame()
            }
        }
    }
}
