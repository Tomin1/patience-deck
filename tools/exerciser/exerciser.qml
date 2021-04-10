/*
 * Exerciser for Patience Deck engine class.
 * Copyright (C) 2021  Tomi Leppänen
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
import Patience 1.0

Item {
    property var moveToFoundation: /Move ([\w\s]+) onto (the|an empty) foundation/
    property var moveToTableau: /Move ([\w\s]+) onto (the|an empty) tableau/
    property var moveToReserve: /Move ([\w\s]+) onto (the|an empty) reserve/
    property var moveOnto: /Move ([\w\s]+) onto ([\w\s]+)/
    property var tryMovingCards: /Try moving cards down from the foundation/
    property var dealNew: /Deal a new card from the deck/
    property var removeCard: /Remove ([\w\s]+)/
    property var ranks: ["joker", "ace", "two", "three", "four", "five", "six",
                         "seven", "eight", "nine", "ten", "jack", "queen", "king"]
    property var suits: ["clubs", "diamonds", "hearts", "spades"]
    property int score

    function quit() {
        Qt.quit()
    }

    function getCard(text) {
        var words = text.split(" ")
        if (words[0] != "the" || words[2] != "of") {
            return undefined
        }
        var rank = ranks.indexOf(words[1])
        if (rank < 0 || rank > 13) {
            return undefined
        }
        var suit = suits.indexOf(words[3])
        if (suit < 0 || suit > 3) {
            return undefined
        }
        return { "rank": rank, "suit": suit }
    }

    function parseAndActOnHint(hint) {
        if (moveToFoundation.test(hint)) {
            var matches = hint.match(moveToFoundation)
            console.log("Should move", matches[1], "to foundation")
            helper.move(getCard(matches[1]), {
                "type": EngineHelper.Foundation,
                "empty": (matches[2] == "an empty")
            })
        } else if (moveToTableau.test(hint)) {
            var matches = hint.match(moveToTableau)
            console.log("Should move", matches[1], "to tableau")
            helper.move(getCard(matches[1]), {
                "type": EngineHelper.Tableau,
                "empty": (matches[2] == "an empty")
            })
        } else if (moveToReserve.test(hint)) {
            var matches = hint.match(moveToReserve)
            console.log("Should move", matches[1], "to reserve")
            helper.move(getCard(matches[1]), {
                "type": EngineHelper.Reserve,
                "empty": (matches[2] == "an empty")
            })
        } else if (moveOnto.test(hint)) {
            var matches = hint.match(moveOnto)
            console.log("Should move", matches[1], "to", matches[2])
            helper.move(getCard(matches[1]), getCard(matches[2]))
        } else if (dealNew.test(hint)) {
            console.log("Should deal a new card")
            helper.engine.dealCard()
        } else if (tryMovingCards.test(hint)) {
            console.log("No more hints, unsuccessful")
            console.log("Final score was:", score)
            quit()
        } else if (removeCard.test(hint)) {
            var matches = hint.match(removeCard)
            console.log("Removing", matches[1])
            helper.click(getCard(matches[1]))
        } else {
            console.warn("Unknown hint:", hint)
            quit()
        }
    }

    Component.onCompleted: if (!helper.parseArgs()) quit()

    EngineHelper {
        id: helper
    }

    Timer {
        id: newMove
        interval: 0
        onTriggered: helper.engine.getHint()
    }

    Connections {
        target: helper.engine

        function onEngineFailure(message) {
            console.log("Engine failure:", message)
            quit()
        }

        function onScore(newScore) {
            score = newScore
        }

        function onGameLoaded(game) {
            console.log("Loaded", game)
            helper.engine.start()
        }

        function onGameStarted() {
            console.log("Game started with seed", helper.getSeed())
            newMove.start()
        }

        function onHint(hint) {
            parseAndActOnHint(hint)
        }

        function onMoveEnded() {
            newMove.start()
        }

        function onGameOver(won) {
            console.log("Game over, won:", won)
            console.log("Final score was:", score)
            quit()
        }
    }
}
