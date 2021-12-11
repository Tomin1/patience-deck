#!/usr/bin/env python3
#
# Generate a different set of cards
# Copyright (C) 2021 Tomi Leppänen
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# DISCLAIMER
#
# This does not try to follow svg spec unless it makes things easier.
# Everything here was written on basis of do whatever gets the job done.
# If you make changes that you want to contribute back, please check
# that they won't result in unintended changes to existing graphics.

import argparse
import json
import lxml.etree as ET
import re

def zero(value):
    return abs(value) < 0.001

TRANSLATE = re.compile(r'translate\((?P<x>-?[\d\.]+),(?P<y>-?[\d\.]+)\)')
MATRIX = re.compile(r'matrix\((?P<a>-?[\d\.]+),(?P<b>-?[\d\.]+),(?P<c>-?[\d\.]+),'
                            r'(?P<d>-?[\d\.]+),(?P<e>-?[\d\.]+),(?P<f>-?[\d\.]+)\)')
SCALE = re.compile(r'scale\((?P<x>-?[\d\.]+),(?P<y>-?[\d\.]+)\)')

def translate(t):
    match = TRANSLATE.match(t)
    return float(match.group('x')), float(match.group('y'))

def matrix(t):
    match = MATRIX.match(t)
    a = float(match.group('a'))
    b = float(match.group('b'))
    c = float(match.group('c'))
    d = float(match.group('d'))
    e = float(match.group('e'))
    f = float(match.group('f'))
    return a, b, c, d, e, f

def scale(t):
    match = SCALE.match(t)
    return float(match.group('x')), float(match.group('y'))

def transform(elem, x, y):
    t = elem.get('transform')
    if t:
        assert ' ' not in t
        if t.startswith('translate('):
            dx, dy = translate(t)
            return x + dx, y + dy
        elif t.startswith('matrix('):
            a, b, c, d, e, f = matrix(t)
            return a * x + c * y + e, b * x + d * y + f
        elif t.startswith('scale('):
            u, v = scale(t)
            return x * u, y * v
        else:
            assert False, f"Unknown transformation: {t}"
    return x, y

def position(elem):
    x = float(elem.get('x', 0))
    y = float(elem.get('y', 0))
    return x, y

SPLIT_PATH = re.compile(r'[A-Za-z\s]+')
POINT = re.compile(r'(?P<x>-?[\d\.]+)(?:,(?P<y>-?[\d\.]+))?')

def iterate_path_points(elem):
    assert elem.tag == '{http://www.w3.org/2000/svg}path', f"{elem.get('id', elem.tag)} is not a path"
    points = SPLIT_PATH.split(elem.get('d', 'M0,0'))
    for point in points:
        if point:
            p = POINT.match(point)
            assert p is not None, f"Invalid point in {elem.get('id')}: {point}"
            yield float(p.group('x')), float(p.group('y') if p.group('y') is not None else p.group('x'))

def calculate_extremes(elem):
    if elem.tag == '{http://www.w3.org/2000/svg}path':
        min_x, min_y, max_x, max_y = 2**32-1, 2**32-1, 0, 0
        for x, y in iterate_path_points(elem):
            min_x = min(min_x, x)
            min_y = min(min_y, y)
            max_x = max(max_x, x)
            max_y = max(max_y, y)
        return min_x, min_y, max_x, max_y
    elif elem.tag == '{http://www.w3.org/2000/svg}g':
        return float(elem.get('x', 0)), float(elem.get('y', 0)), 0, 0
    elif elem.tag == '{http://www.w3.org/2000/svg}rect':
        return (float(elem.get('x', 0)), float(elem.get('y', 0)),
                float(elem.get('width', 0)), float(elem.get('height', 0)))
    else:
        assert False, f"Unknown tag: {elem.tag}"

def calculate_topleft(elem):
    x, y, *_ = calculate_extremes(elem)
    return x, y

def calculate_transformation(tree, name):
    ox, oy = 0, 0
    elem = tree.find(f"//*[@id='{name}']")
    if elem.tag == '{http://www.w3.org/2000/svg}use':
        id = elem.get('{http://www.w3.org/1999/xlink}href')[1:]
        used = tree.find(f"//*[@id='{id}']")
        ox, oy = calculate_transformation(tree, id)
    else:
        ox, oy = calculate_topleft(elem)
    return transform(elem, ox, oy)

def calculate_position(tree, name):
    ox, oy = calculate_transformation(tree, name)
    x, y = position(tree.find(f"//*[@id='{name}']"))
    return ox + x, oy + y

def style(elem, name, default):
    for attr in elem.get('style', '').split(';'):
        if attr.startswith(f'{name}:'):
            _, value = attr.split(':', 2)
            return value
    return default

def calculate_size(tree, elem):
    if elem.tag == '{http://www.w3.org/2000/svg}path':
        stroke = style(elem, 'stroke-width', 1)
        min_x, min_y, max_x, max_y = calculate_extremes(elem)
        return max_x - min_x, max_y - min_y
    elif elem.tag == '{http://www.w3.org/2000/svg}use':
        id = elem.get('{http://www.w3.org/1999/xlink}href')[1:]
        return calculate_size(tree, tree.find(f"//*[@id='{id}']"))
    elif elem.tag == '{http://www.w3.org/2000/svg}g':
        min_x, min_y, max_x, max_y = 2**32-1, 2**32-1, -2**32, -2**32
        for child in elem.iterchildren():
            x1, y1, x2, y2 = calculate_extremes(child)
            x1, y1 = transform(child, x1, y1)
            x2, y2 = transform(child, x2, y2)
            min_x = min(min_x, x1)
            min_y = min(min_y, y1)
            max_x = max(max_x, x2)
            max_y = max(max_y, y2)
        min_x, min_y = transform(elem, min_x, min_y)
        max_x, max_y = transform(elem, max_x, max_y)
        return max_x - min_x, max_y - min_y
    else:
        assert False, f"Unknown tag: {elem.tag}"

def move(tree, elem, dx, dy, copy_tree):
    name = elem.get('id')

    x1, y1 = calculate_position(copy_tree, name)
    x2, y2 = calculate_position(tree, name)
    dx2 = x2 - x1
    dy2 = y2 - y1

    t = elem.get('transform')
    if t:
        assert ' ' not in t
        if t.startswith('translate('):
            x, y = translate(t)
            elem.set('transform', f"translate({x + dx - dx2:.04f},{y + dy - dy2:.04f})")
        elif t.startswith('matrix('):
            a, b, c, d, e, f = matrix(t)
            elem.set('transform', f"matrix({a},{b},{c},{d},"
                     f"{e - dx2 + dx:.04f},"
                     f"{f - dy2 + dy:.04f})")
        elif t.startswith('scale('):
            x, y = scale(t)
            elem.set('transform', f"matrix({x},0,0,{y},{x * -dx2 + dx:.04f},{y * -dy2 + dy:.04f})")
        else:
            assert False, f"Unknown transformation: {t}"
    elif dx != 0 or dy != 0 or dx2 != 0 or dy2 != 0:
        elem.set('transform', f"translate({dx - dx2:.04f},{dy - dy2:.04f})")

    xt, yt = calculate_position(copy_tree, name)
    if dx == 0 and dy == 0:
        xt2, yt2 = calculate_position(tree, name)
        assert zero(xt2 - xt), f"{name} moved by {xt2 - xt:.03g} in x in tree instead of 0"
        assert zero(yt2 - yt), f"{name} moved by {yt2 - yt:.03g} in y in tree instead of 0"
    else:
        xt2, yt2 = calculate_position(tree, name)
        assert zero(xt + dx - xt2), f"{name} moved by wrong amount {xt + dx - xt2:.03g} in x tree"
        assert zero(yt + dy - yt2), f"{name} moved by wrong amount {yt + dy - yt2:.03g} in y tree"

def modify(tree, copy_tree, actions):
    root = tree.getroot()
    params = dict(filter(lambda x: x[0][0] in ('C', 'M'), actions.items()))
    for card in root:
        name = card.get('id')
        if name not in actions.keys():
            continue
        appended = []
        for elem in card.iterchildren():
            for action in actions[name].get(elem.get('id'), []):
                command = action[0]
                if command == 'D':
                    card.remove(elem)
                elif command == 'C':
                    assert len(params[action]) in (2, 4), f"Wrong number of params for {action}, expected 2 or 4"
                    appended.append((elem, *params[action]))
                elif command == 'M':
                    assert len(params[action]) == 2, f"Wrong number of params for {action}, expected 2"
                    dx, dy = params[action]
                    move(tree, elem, dx, dy, copy_tree)
        for elem, *pos in appended:
            used = elem.get('id')
            attrib = { '{http://www.w3.org/1999/xlink}href': '#' + used }
            if len(pos) == 2:
                dx, dy = pos
                attrib['transform'] = f"translate({dx:.04f},{dy:.04f})"
            elif len(pos) == 4:
                x, y = calculate_position(tree, used)
                w, h = calculate_size(tree, tree.find(f"//*[@id='{used}']"))
                dx, dy, u, v = pos
                attrib['transform'] = (f"matrix({u:.04f},0,0,{v:.04f},"
                                       f"{u * -x + x - (u * w - w) / 2 + dx:.04f},"
                                       f"{v * -y + y - (v * h - h) / 2 + dy:.04f})")
            elem.addnext(elem.makeelement('use', attrib))

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("json")
    parser.add_argument("input")
    parser.add_argument("output")
    args = parser.parse_args()
    actions = None
    with open(args.json) as file:
        actions = json.load(file)
    tree = ET.parse(args.input)
    copy_tree = ET.parse(args.input)
    modify(tree, copy_tree, actions)
    tree.write(args.output)

if __name__ == "__main__":
    main()
