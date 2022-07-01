/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021-2022 Tomi Leppänen
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
import PatienceDeck 1.0

Page {
    id: page
    /* This page is mostly untranslated on purpose */ 

    readonly property var licenses: [
        ({
            //% "Patience Deck is free software, "
            //% "and you are welcome to redistribute it under certain conditions. "
            //% "This software comes with ABSOLUTELY NO WARRANTY. "
            //% "Patience Deck contains work licensed under multiple different licenses."
            "text": qsTrId("patience-la-free_software_and_redistribution_multiple_licenses"),
            "count": 0,
            "type": "header"
        }),
        ({
            "text": "Patience Deck itself is licensed under <a href='#1'>%1</a>.",
            "abbrs": { 0: "GNU GPLv3" },
            "names": { 0: "GNU General Public License version 3" },
            "links": { 0: "COPYING.GPL3" },
            "count": 1,
            "type": "regular"
        }),
        ({
            "text": "GNOME Aisleriot games and distributed artwork are licensed under <a href='#1'>%1</a> or later.",
            "abbrs": { 0: "GNU GPLv3" },
            "names": { 0: "GNU General Public License version 3" },
            "links": { 0: "COPYING.GPL3" },
            "count": 1,
            "type": "regular"
        }),
        ({
            "text": "GNOME Aisleriot manual pages are licensed under <a href='#1'>%1</a> (1.1 or later).",
            "abbrs": { 0: "GFDL 1.3" },
            "names": { 0: "GNU Free Documentation License version 1.3" },
            "links": { 0: "COPYING.GFDL1.3" },
            "count": 1,
            "type": "last"
        })
    ]
    readonly property var libLicenses: [
        ({
            //% "Harbour distribution of Patience Deck bundles several libraries that have their own licenses."
            "text": qsTrId("patience-la-harbour_distribution_bundles_libraries"),
            "count": 0,
            "type": "header"
        }),
        ({
            "text": "Guile, GNU MP and libunistring libraries are distributed under <a href='#1'>%1</a>. See also <a href='#2'>%2</a>.",
            "abbrs": { 0: "GNU LGPLv3", 1: "GNU GPLv3" },
            "names": {
                0: "GNU Lesser General Public License version 3",
                1: "GNU General Public License version 3"
            },
            "links": { 0: "lib/licenses/COPYING.LESSER", 1: "lib/licenses/COPYING.GPL3" },
            "count": 2,
            "type": "regular"
        }),
        ({
            "text": "Gc library is permissively licensed. See <a href='#1'>%1</a> for more details.",
            "abbrs": { 0: "README" },
            "names": { 0: "gc readme file" },
            "links": { 0: "lib/licenses/gc.README" },
            "count": 1,
            "type": "regular"
        }),
        ({
            "text": "Libffi library is permissively licensed. See <a href='#1'>%1</a> for more details.",
            "abbrs": { 0: "LICENSE" },
            "names": { 0: "libffi license file" },
            "links": { 0: "lib/licenses/libffi.LICENSE" },
            "count": 1,
            "type": "regular"
        }),
        ({
            "text": "Libltdl library is distributed under <a href='#1'>%1</a>.",
            "abbrs": { 0: "GNU LGPLv2.1" },
            "names": { 0: "GNU Lesser General Public License version 2.1" },
            "links": { 0: "lib/licenses/COPYING.LIB" },
            "count": 1,
            "type": "last"
        })
    ]

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
            id: licenseModel

            Component.onCompleted: {
                for (var i = 0; i < page.licenses.length; i++) {
                    licenseModel.append(page.licenses[i])
                }
                if (PatienceDeck.showLibraryLicenses) {
                    for (var i = 0; i < page.libLicenses.length; i++) {
                        licenseModel.append(page.libLicenses[i])
                    }
                }
            }
        }

        delegate: Component {
            Paragraph {
                color: model.type == "header" ? Theme.secondaryHighlightColor : Theme.highlightColor
                linkColor: Theme.primaryColor
                text: {
                    var text = model.text
                    for (var i = 0; i < model.count; i++) {
                        text = text.arg(model.names[i])
                    }
                    return text
                }
                onLinkActivated: {
                    var index = parseInt(link.slice(1))-1
                    pageStack.push(licensePage, {
                        "contentFile": Qt.resolvedUrl("../../../" + model.links[index]),
                        "title": model.abbrs[index],
                        "description": model.names[index]
                    })
                }
                spacing: model.type == "header" || model.type == "last" ? Theme.paddingLarge : 0
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
