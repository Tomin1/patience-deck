#!/usr/bin/env python3
#
# Simple testing tool for Patience Deck
# Copyright (C) 2022 Tomi Leppänen
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
import os
import re
import shlex
import subprocess
import sys

TIMEOUT=5*60

RESULTS = [
    "WON",   # 0
    "INVAL", # 1
    "LOST",  # 2
    "STUCK", # 3
    "NEW",   # 4
    "OTHER", # 5
]

COMMENT = re.compile(r"(?P<code>WON|INVAL|LOST|STUCK|NEW|OTHER)(?:, errors (?P<errors>\d+))?(?:, CRC (?P<crc>A\d+))?")
CRC_LINE = re.compile(br"\[D\] [^ ]+:\d+ - CRC is (?P<crc>\d+)")
CRC_FAIL = re.compile(br"\[W\] [^ ]+:\d+ - CRC failure after replaying, expected \d+ but got (?P<crc>\d+)")

def result_text(code):
    if code < len(RESULTS):
        return RESULTS[code]
    return f"UNKNOWN({code})"

def read_saved_states(filename):
    result_codes = dict(map(lambda x: (x[1], x[0]), enumerate(RESULTS)))
    parser = argparse.ArgumentParser('patience-deck', add_help=False)
    parser.add_argument('-g', '--game')
    parser.add_argument('-s', '--seed')
    parser.add_argument('-m', '--moves')
    parser.add_argument('-c', '--crc')
    parser.add_argument('-o', '--options', default="")
    parser.add_argument('-v', '--version')
    with open(filename, 'r') as file:
        for line in filter(lambda l: not l.startswith('#'), file):
            parts = line.partition('#')
            comment = COMMENT.search(parts[2])
            code = 0
            expected_errors = 0
            expected_crc = None
            if comment:
                code = result_codes[comment.group("code")]
                if comment.group("errors"):
                    expected_errors = int(comment.group("errors"))
                if comment.group("crc"):
                    expected_crc = comment.group("crc")
            yield parser.parse_args(shlex.split(parts[0])[1:]), code, expected_errors, expected_crc

def filter_errors(line):
    # Only critical and warning lines
    if not line.startswith(b'[W] ') and not line.startswith(b'[C] '):
        return False
    # May happen but does not affect the result
    if line.endswith(b'No window. Deleting texture later. This may leak fds!'):
        return False
    return True

def filter_crc(line):
    return bool(CRC_LINE.match(line) or CRC_FAIL.match(line))

def get_crc_line(line, crc):
    if line is not None:
        match = CRC_LINE.match(line) or CRC_FAIL.match(line)
        if match:
            return match.group("crc")
        assert False, "Not reached"
    return crc

def get_crc(lines):
    crc_lines = filter(filter_crc, lines)
    crc = get_crc_line(next(crc_lines, None), None)
    crc = get_crc_line(next(crc_lines, None), crc)
    return crc

def run(test, expected_result, expected_errors, expected_crc):
    print(f"Testing with {test.game} (seed {test.seed})...", end="")
    sys.stdout.flush()
    command = ["patience-deck", "--test", "--game", test.game, "--seed", test.seed,
               "--moves", test.moves, "--options", test.options]
    if test.crc is not None:
        command.extend(["--crc", test.crc])
    if test.version is not None:
        command.extend(["--version", test.version])
    if expected_crc is None:
        expected_crc = test.crc
    env = dict(os.environ)
    env["QT_LOGGING_RULES"] = "site.tomin.patience.engine.recorder.crc=true"
    result = subprocess.run(command, timeout=TIMEOUT, capture_output=True, env=env)
    errors = []
    for line in filter(filter_errors, result.stderr.split(b'\n')):
        errors.append(line.decode('UTF-8'))
    crc = get_crc(result.stderr.split(b'\n'))
    if crc is not None:
        crc = 'A' + crc.decode('UTF-8')
    succeeded = result.returncode == expected_result and len(errors) == expected_errors and crc == expected_crc
    if succeeded:
        status = "succeeded" if expected_errors == 0 else f"succeeded with {len(errors)} expected errors"
        print(f" {status}, result {result_text(result.returncode)} with crc {crc}")
    else:
        status = "FAILED" if errors == expected_errors else f"FAILED with {len(errors)} errors"
        print(f" {status}, result {result_text(result.returncode)}, expected {result_text(expected_result)}, crc {crc}, expected {expected_crc}")
    for line in errors:
        print("-->", line)
    return succeeded

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('filename')
    args = parser.parse_args()
    count = 0
    successful = 0
    for test, expected_result, expected_errors, expected_crc in read_saved_states(args.filename):
        count += 1
        if run(test, expected_result, expected_errors, expected_crc):
            successful += 1
    if count == successful:
        print(f"All {count} tests passed!")
        exit(0)
    else:
        print(f"Failure: {successful}/{count} tests passed, failed {count - successful}")
        exit(1)

if __name__ == "__main__":
    main()
