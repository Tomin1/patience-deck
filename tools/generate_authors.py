#!/usr/bin/env python3
#
# Build AUTHORS file
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

import argparse
import itertools
import os
import re

class AuthorsParser:
    RE = re.compile(r"^[\w \.-]*\w")

    def __init__(self, file):
        self.file = file

    def __iter__(self):
        with open(self.file) as file:
            for line in file:
                match = self.RE.search(line)
                if match:
                    yield match.group(0)

class GamesDirParser:
    RE = re.compile(r"^;+\s+Copyright \(C\)\s+[0-9, -]+\s+([\w \.-]*\w)")

    def __init__(self, directory):
        self.directory = directory

    def __iter__(self):
        for entry in os.scandir(self.directory):
            if entry.name.endswith(".scm"):
                with open(entry) as file:
                    for line in file:
                        match = self.RE.search(line)
                        if match:
                            yield match.group(1)


class ManualDirParser:
    # This is not an XML parser
    RE = re.compile(r"""<author>\s*\n?\s*
                            <firstname>([\w \.-]*\w)</firstname>\s*\n?\s*
                            <surname>([\w \.-]*\w)</surname>\s*\n?\s*
                        </author>""",
        re.MULTILINE | re.VERBOSE)
    def __init__(self, directory):
        self.directory = directory

    def __iter__(self):
        for entry in os.scandir(self.directory):
            if entry.name.endswith(".xml"):
                with open(entry) as file:
                    for match in self.RE.finditer(file.read()):
                        yield match.group(1) + " " + match.group(2)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--authors", action='append', metavar="AUTHORS",
                        help="AUTHORS file to read", default=[])
    parser.add_argument("--games", action='append', metavar="DIRECTORY",
                        help="Games directory to read", default=[])
    parser.add_argument("--manual", action='append', metavar="DIRECTORY",
                        help="Manual directory to parse", default=[])
    parser.add_argument("--append", action='append', metavar="NAME",
                        help="Name to append to list of names", default=[])
    parser.add_argument("file", type=argparse.FileType('w'), metavar="OUTPUT",
                        help="File to generate")
    args = parser.parse_args()
    parsers = []
    for authors in args.authors:
        parsers.append(AuthorsParser(authors))
    for manual in args.manual:
        parsers.append(ManualDirParser(manual))
    for games in args.games:
        parsers.append(GamesDirParser(games))
    authors = set(itertools.chain(*parsers, args.append))
    for name in sorted(authors):
        args.file.write(name + "\n")

if __name__ == "__main__":
    main()
