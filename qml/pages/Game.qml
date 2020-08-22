import QtQuick 2.0
import Sailfish.Silica 1.0
import Aisleriot 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

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
            spacing: Theme.paddingLarge
            PageHeader {
                title: qsTr("Aisleriot")
            }
            Label {
                x: Theme.horizontalPageMargin
                text: qsTr("To be done")
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeExtraLarge
            }

            Button {
                text: "Load Freecell"
                onClicked: Aisleriot.loadGame("freecell.scm")
            }
        }
    }
}
