#!/bin/sh
#
# Store current state into a file
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

FILE="$1"
if [ "$FILE" = "" ];
then
    echo "Usage: $0 <filename>"
    exit 1
fi
shift
COMMENT="$*"

STATE=$(dconf read /site/tomin/apps/PatienceDeck/state | tr -d \')
GAME="${STATE%%;*}"
SEED=$(echo "$STATE" | cut -d\; -f2)
MOVES="${STATE##*;}"
OPTIONS=$(dconf read "/site/tomin/apps/PatienceDeck/options/${GAME%.scm}" | tr -d \')

COMMAND="patience-deck --game '$GAME' --seed $SEED --moves '$MOVES'"
if [ "$OPTIONS" != "" ];
then
    COMMAND="$COMMAND --options '$OPTIONS'"
fi
if [ "$COMMENT" != "" ];
then
    COMMAND="$COMMAND # $COMMENT"
fi
echo "$COMMAND" >> "$FILE"
