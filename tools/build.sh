#!/bin/bash
#
# Build packages for publishing
# Copyright (c) 2021 Tomi Leppänen
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

usage() {
    2>&1 echo "Usage: $0 [--tag TAG] --dir DIRECTORY TARGET [TARGET...]"
    exit 1
}

clean() {
    git reset --hard HEAD
    sb2 make clean || :
    rm -f Makefile *.list
    find data/ qml/ src/ tools/ translations/ \( \
        -name Makefile -o \
        -name '*.list' -o \
        -name '*.o' -o \
        -name '*patience-deck.qm' -o \
        -name '*patience-deck.ts' -o \
        -name '*patience-deck' -o \
        -name 'moc_*.cpp' \) -delete -print
    pushd aisleriot/
    git reset --hard HEAD
    popd
}

build() {
    mb2 -t "$1" build -p -d $2
}

DIR=""
TAG=""
HARBOUR=""

while [[ "$1" == --* ]]
do
    case $1 in
    --dir)
        shift
        DIR="$1"
        ;;
    --tag)
        shift
        TAG="$1"
        ;;
    --harbour)
        HARBOUR="--with harbour"
        ;;
    --help)
        usage
        ;;
    esac
    shift
done

TARGETS=$@

if [ "$DIR" = "" ] || [[ "${#TARGETS[@]}" -eq 0 ]]
then
    usage
fi

set -e

clean
if [ -n "$TAG" ] && ! git tag "$TAG" && [ "$TAG" != "$(git describe --tags)" ]
then
    2>&1 echo "$TAG does not match HEAD"
    exit 2
fi

for target in $TARGETS
do
    echo "Building for $target"
    build "$target" "$HARBOUR"
    cp -v RPMS/*patience-deck-[0-9]*.rpm "$DIR/"
    clean
done
