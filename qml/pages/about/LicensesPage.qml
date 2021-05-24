/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021 Tomi Leppänen
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

Page {
    id: page

    allowedOrientations: Orientation.All

    SilicaListView {
        anchors.fill: parent
        spacing: Theme.paddingLarge

        header: Component {
            PageHeader {
                //% "Licenses"
                title: qsTrId("patience-he-licenses")
            }
        }

        model: ListModel {
            ListElement {
                text: "Patience Deck code is licensed under <a href='#'>%1</a>."
                abbr: "GNU GPLv3"
                name: "GNU General Public License version 3"
                full: "../../../COPYING.GPL3"
            }

            ListElement {
                text: "GNOME Aisleriot games and distributed artwork are licensed under <a href='#'>%1</a> or later."
                abbr: "GNU GPLv3"
                name: "GNU General Public License version 3"
                full: "../../../COPYING.GPL3"
            }

            ListElement {
                text: "GNOME Aisleriot manual pages are licensed under <a href='#'>%1</a> or later."
                abbr: "GFDL 1.3"
                name: "GNU Free Documentation License version 1.3"
                full: "../../../COPYING.GFDL1.3"
            }
        }

        delegate: Component {
            Paragraph {
                linkColor: Theme.primaryColor
                text: model.text.arg(model.name)
                onLinkActivated: pageStack.push(licensePage, {
                    "contentFile": Qt.resolvedUrl(model.full),
                    "title": model.abbr,
                    "description": model.name
                })
                width: page.width - 2*Theme.horizontalPageMargin
            }
        }
    }

    Component {
        id: licensePage

        Page {
            property alias title: licensePageHeader.title
            property alias description: licensePageHeader.description
            property url contentFile

            allowedOrientations: Orientation.All

            function load() {
                var xhr = new XMLHttpRequest
                xhr.open("GET", contentFile)
                xhr.onreadystatechange = function() {
                    if (xhr.readyState == XMLHttpRequest.DONE) {
                        content.text = xhr.responseText
                    }
                }
                xhr.send()
            }

            onStatusChanged: if (status == PageStatus.Active) load()

            BusyIndicator {
                anchors.centerIn: parent
                running: content.text === ""
                size: BusyIndicatorSize.Large
            }

            SilicaFlickable {
                anchors.fill: parent
                contentHeight: authorsContent.height

                Column {
                    id: authorsContent
                    width: parent.width

                    PageHeader {
                        id: licensePageHeader
                    }

                    Paragraph {
                        id: content
                        opacity: content.text !== "" ? 1.0 : 0.0
                        Behavior on opacity {
                            FadeAnimator {
                                duration: 500
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }

                VerticalScrollDecorator { }
            }
        }
    }
}
