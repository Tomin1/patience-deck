#!/bin/sh
#
# Screenshot games
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

if [ "$(id -g)" != "$(getent group privileged | cut -d: -f3)" ]
then
    echo "This must be run with privileged group"
    echo "Usage: devel-su -p $0 GAME [GAME ...]"
    exit 2
fi

if [ $# -eq 0 ]
then
    echo "Usage: $0 GAME [GAME ...]"
    echo "for example:"
    echo "    $0 \$(ls /usr/share/patience-deck/games/*.scm)"
    exit 1
fi

DIR="$HOME/Pictures/Screenshots/patience-deck/$(date -I)"

mkdir -p "$DIR"

for game in $@
do
    name=$(basename "$game")
    path="$DIR/${game%.*}.png"
    echo "Taking a screenshot of $path"
    patience-deck --seed 0 --game "$name" 2> /dev/null &
    sleep 2
    if [ -f "$path" ]
    then
        rm "$path"
    fi
    dbus-send --session --dest=org.nemomobile.lipstick --type=method_call --print-reply \
            /org/nemomobile/lipstick/screenshot org.nemomobile.lipstick.saveScreenshot \
            "string:$path" > /dev/null
    kill %1
    wait
    sleep 5
done
