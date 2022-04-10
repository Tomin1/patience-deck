/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2021 Tomi Leppänen
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

import QtQml 2.2
import QtQuick 2.6
import Sailfish.Silica 1.0
import Patience 1.0

Page {
    id: aboutPage

    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: content.height
        contentWidth: content.width

        Column {
            id: content

            width: aboutPage.width

            PageHeader {
                //% "About"
                title: qsTrId("patience-he-about")
                //: Name of this application, %1 is version number
                //% "Patience Deck %1"
                description: qsTrId("patience-de-patience_deck").arg(Qt.application.version)
                height: Math.max(implicitHeight, icon.height + 2 * Theme.paddingLarge)
                rightMargin: Theme.paddingLarge + icon.width + Theme.horizontalPageMargin

                Image {
                    id: icon

                    anchors {
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: parent.verticalCenter
                    }
                    source: Patience.getIconPath(Theme.iconSizeLauncher)
                    sourceSize.height: Theme.iconSizeLauncher
                    sourceSize.width: Theme.iconSizeLauncher
                }
            }

            Paragraph {
                linkColor: Theme.primaryColor
                //% "Patience Deck is a collection of %1 supported patience games for Sailfish OS. "
                //% "It reimplements game engine from <a href=%2>GNOME Aisleriot</a> and utilises "
                //% "its implementations of patience games including translations, manual pages and artwork."
                text: qsTrId("patience-la-about_text")
                    .arg(Patience.gamesCount)
                    .arg("\"https://wiki.gnome.org/Apps/Aisleriot\"")
                onLinkActivated: Qt.openUrlExternally(link)
            }

            SectionHeader {
                //: Section to list developers and link to github
                //% "Development"
                text: qsTrId("patience-se-development")
            }

            Paragraph {
                linkColor: Theme.primaryColor
                //% "You may obtain source code and report bugs on Github: <a href=%2>%1</a>"
                text: qsTrId("patience-la-source_code_report_bugs_github")
                    .arg("github.com/Tomin1/patience-deck")
                    .arg("\"https://github.com/Tomin1/patience-deck/\"")
                onLinkActivated: Qt.openUrlExternally(link)
                spacing: Theme.paddingMedium
            }

            Button {
                //: Button to open the list of Patience Deck contributors
                //% "Contributors"
                text: qsTrId("patience-bt-contributors")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: pageStack.push(contributorsPage)
            }

            SectionHeader {
                //: Thank you section for developers of GNOME Aisleriot
                //% "Acknowledgements"
                text: qsTrId("patience-se-acknowledgements")
            }

            Paragraph {
                //% "Thank you to all GNOME Aisleriot authors for making such high quality games "
                //% "to enjoy! This project would not be possible without your work!"
                text: qsTrId("patience-la-thank_you_gnome_aisleriot_authors")
                spacing: Theme.paddingMedium
            }

            Button {
                //% "Authors"
                text: qsTrId("patience-bt-authors")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: pageStack.push(authorsPage)
            }

            SectionHeader {
                //: Section that mentions license(s) of Patience Deck
                //% "License"
                text: qsTrId("patience-se-license")
            }

            Paragraph {
                //% "Patience Deck is free software, "
                //% "and you are welcome to redistribute it under certain conditions. "
                //% "This software comes with ABSOLUTELY NO WARRANTY. "
                //% "Tap the button below for more details."
                text: qsTrId("patience-la-free_software_and_redistribution")
                spacing: Theme.paddingMedium
            }

            Button {
                //% "Licenses"
                text: qsTrId("patience-bt-licenses")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: pageStack.push(Qt.resolvedUrl("LicensesPage.qml"))
            }

            SectionHeader {
                //: Section for enabling experimental features
                //% "Experimental"
                text: qsTrId("patience-se-experimental")
            }

            TextSwitch {
                //% "Show all games"
                text: qsTrId("patience-la-show_all_games")
                //% "List also unsupported games in game selection"
                description: qsTrId("patience-de-list_unsupported_games")
                checked: Patience.showAllGames
                onClicked: Patience.showAllGames = !Patience.showAllGames
                height: implicitHeight + Theme.paddingLarge
            }
        }

        VerticalScrollDecorator {}
    }

    Component {
        id: contributorsPage

        Page {
            allowedOrientations: Orientation.All

            SilicaFlickable {
                anchors.fill: parent
                contentHeight: contributorsContent.height

                Column {
                    id: contributorsContent

                    bottomPadding: Theme.paddingLarge
                    spacing: Theme.paddingMedium
                    width: parent.width

                    PageHeader {
                        //: Page header for the list of Patience Deck contributors
                        //% "Contributors of Patience Deck"
                        title: qsTrId("patience-he-contributors")
                    }

                    SectionHeader {
                        //: Section header for list of developers of Patience Deck
                        //% "Developers"
                        text: qsTrId("patience-se-developers")
                    }

                    Paragraph {
                        //% "Main developer: %1"
                        text: qsTrId("patience-la-see_also_manuals")
                            .arg("Tomi Leppänen")
                        spacing: Theme.paddingSmall
                    }

                    SectionHeader {
                        //: Section header for list of translators of Patience Deck
                        //% "Translators"
                        text: qsTrId("patience-se-translators")
                    }

                    Paragraph {
                        text: Patience.translators
                    }
                }

                VerticalScrollDecorator { }
            }
        }
    }

    Component {
        id: authorsPage

        Page {
            allowedOrientations: Orientation.All

            SilicaFlickable {
                anchors.fill: parent
                contentHeight: authorsContent.height

                Column {
                    id: authorsContent

                    bottomPadding: Theme.paddingLarge
                    spacing: Theme.paddingMedium
                    width: parent.width

                    PageHeader {
                        //% "Thanks to"
                        title: qsTrId("patience-he-thanks_to")
                        //% "GNOME Aisleriot authors"
                        description: qsTrId("patience-de-aisleriot_authors")
                    }

                    Paragraph {
                        text: Patience.aisleriotAuthors
                    }

                    SectionHeader {
                        //: Section to show Aisleriot translator/translation information, substitute
                        //: word language with the name of the language of the translation.
                        //: Original is pretty generic on purpose, feel free to translate this to
                        //: something works for the result
                        //% "Aisleriot translator information for language"
                        text: qsTrId("patience-se-translator-information")
                        visible: Patience.aisleriotTranslatorInfo !== ""
                    }

                    Paragraph {
                        text: Patience.aisleriotTranslatorInfo
                        visible: Patience.aisleriotTranslatorInfo !== ""
                    }
                }

                VerticalScrollDecorator { }
            }
        }
    }
}
