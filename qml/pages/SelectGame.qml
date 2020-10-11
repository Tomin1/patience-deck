import QtQuick 2.0
import Sailfish.Silica 1.0
import Patience 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView
        model: GameList { }
        anchors.fill: parent
        header: PageHeader {
            title: qsTr("Games")
        }
        delegate: BackgroundItem {
            id: delegate

            Label {
                x: Theme.horizontalPageMargin
                text: display
                anchors.verticalCenter: parent.verticalCenter
                color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
            }
            onClicked: {
                Patience.loadGame(filename)
                pageStack.pop()
            }
        }

        VerticalScrollDecorator {}
    }
}
