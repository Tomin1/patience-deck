#!/bin/sh
#
# Take reproducible screenshots
# Copyright (c) 2022 Tomi Leppänen
#
# Permission to use, copy, modify, and/or distribute this file for any purpose
# with or without fee is hereby granted.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

# Put these screenshots to dist/screenshots to use the svg templates.

if [ "$(id -g)" != "$(getent group privileged | cut -d: -f3)" ]
then
    echo "This must be run with privileged group"
    echo "Usage: devel-su -p $0"
    exit 1
fi

start_pd() {
    echo "Starting $*"
    LANGUAGE=en patience-deck "$@" 2> /dev/null &
    sleep 5
}

stop_pd() {
    kill %1
    wait
}

take_screenshot() {
    dbus-send --session --dest=org.nemomobile.lipstick --type=method_call --print-reply \
            /org/nemomobile/lipstick/screenshot org.nemomobile.lipstick.saveScreenshot \
            "string:$PWD/patience-deck-screenshots/$1" > /dev/null
    sleep 5
}

screenshot() {
    echo "Taking $1"
    file="$1"
    shift
    start_pd "$@"
    take_screenshot "$file"
    stop_pd
}

set_ambience() {
    dbus-send --session --dest=com.jolla.ambienced --type=method_call --print-reply \
            /com/jolla/ambienced com.jolla.ambienced.setAmbience \
            "string:file:///usr/share/ambience/$1/$1.ambience" > /dev/null
    sleep 3
}

screenshot_ambience() {
    echo "Taking $1 with $2 ambience"
    set_ambience "$2"
    take_screenshot "$1"
}

[ -d patience-deck-screenshots ] && rm -rf patience-deck-screenshots
mkdir -p patience-deck-screenshots

set_ambience earth
screenshot "1.png" --game 'klondike.scm' --seed 0 --moves 'AAAAVHicMzawMDay8rGy1PG1MjSyMrUyhDAMDYAsHyBLx9nKAMiwANNAKStLiBKICmSGEVgKqAwAA60SGg==' --options '1' --time 30000 --background adaptive --cards regular
screenshot "bear-river.png" --game 'bear-river.scm' --seed 101 --background adaptive --cards regular
screenshot "block-ten.png" --game 'block-ten.scm' --seed 102 --background adaptive --cards regular
screenshot "forty-thieves.png" --game 'forty-thieves.scm' --seed 103 --background adaptive --cards regular
screenshot "freecell.png" --game 'freecell.scm' --seed 104 --background adaptive --cards regular
screenshot "helsinki.png" --game 'helsinki.scm' --seed 105 --background adaptive --cards regular
screenshot "klondike.png" --game 'klondike.scm' --seed 106 --options '' --background adaptive --cards regular
screenshot "spider.png" --game 'spider.scm' --seed 107 --options '' --background adaptive --cards regular
screenshot "treize.png" --game 'treize.scm' --seed 108 --background adaptive --cards regular

screenshot "bakers-game-green.png" --game 'bakers-game.scm' --seed 1000 --background green --cards simplified
screenshot "bakers-game-seagreen.png" --game 'bakers-game.scm' --seed 1000 --background seagreen --cards simplified
screenshot "bakers-game-maroon.png" --game 'bakers-game.scm' --seed 1000 --background maroon --cards simplified
screenshot "bakers-game-sienna.png" --game 'bakers-game.scm' --seed 1000 --background sienna --cards simplified
screenshot "bakers-game-navy.png" --game 'bakers-game.scm' --seed 1000 --background navy --cards simplified
screenshot "bakers-game-steelblue.png" --game 'bakers-game.scm' --seed 1000 --background steelblue --cards simplified
screenshot "bakers-game-goldenrod.png" --game 'bakers-game.scm' --seed 1000 --background goldenrod --cards simplified
screenshot "bakers-game-dimgray.png" --game 'bakers-game.scm' --seed 1000 --background dimgray --cards simplified

start_pd --game 'bakers-game.scm' --seed 1000 --background adaptive --cards simplified
screenshot_ambience "bakers-game-airy-adaptive.png" airy
screenshot_ambience "bakers-game-fire-adaptive.png" fire
screenshot_ambience "bakers-game-sailfish3-adaptive.png" sailfish3
screenshot_ambience "bakers-game-earth-adaptive.png" earth
stop_pd

start_pd --game 'bakers-game.scm' --seed 1000 --background transparent --cards simplified
screenshot_ambience "bakers-game-airy-transparent.png" airy
screenshot_ambience "bakers-game-fire-transparent.png" fire
screenshot_ambience "bakers-game-sailfish3-transparent.png" sailfish3
screenshot_ambience "bakers-game-earth-transparent.png" earth
stop_pd

dconf reset /site/tomin/apps/PatienceDeck/state
dconf reset /site/tomin/apps/PatienceDeck/cardStyle
dconf reset /site/tomin/apps/PatienceDeck/backgroundColor
