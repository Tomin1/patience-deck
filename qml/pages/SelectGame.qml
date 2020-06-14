import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaListView {
        id: listView
        model: 20
        anchors.fill: parent
        header: PageHeader {
            title: qsTr("Games")
        }
        delegate: BackgroundItem {
            id: delegate

            Label {
                x: Theme.horizontalPageMargin
                text: qsTr("Game") + " " + index
                anchors.verticalCenter: parent.verticalCenter
                color: delegate.highlighted ? Theme.highlightColor : Theme.primaryColor
            }
            onClicked: console.log("Selected " + index)
        }
        VerticalScrollDecorator {}
    }
}
