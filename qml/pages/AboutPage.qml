import QtQuick 2.6
import Sailfish.Silica 1.0
import Patience 1.0

Page {
    id: aboutPage

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height
        contentWidth: content.width

        Column {
            id: content

            width: aboutPage.width

            PageHeader {
                title: qsTr("About")
                description: qsTr("Patience Deck")
            }

            Label {
                color: Theme.highlightColor
                text: qsTr("This is a collection of patience games for Sailfish OS written by %1.\nIt utilises GNOME Aisleriot's implementations of patience games and artwork.").arg("Tomi Leppänen")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            SectionHeader {
                text: qsTr("Thanks")
            }

            Label {
                color: Theme.highlightColor
                text: qsTr("Thank you to GNOME Aisleriot authors")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            Label {
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: Patience.aisleriotAuthors
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            Label {
                color: Theme.highlightColor
                text: qsTr("Also thank you to card graphics creator")
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }

            Label {
                color: Theme.secondaryHighlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: "Aike Reyer"
                wrapMode: Text.Wrap
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                }
                width: aboutPage.width - 2*Theme.horizontalPageMargin
            }
        }

        VerticalScrollDecorator {}
    }
}
