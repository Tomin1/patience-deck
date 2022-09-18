#!/usr/bin/env python3
#
# Generate a different set of cards
# Copyright (C) 2021-2022 Tomi Leppänen
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
import copy
import functools
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
        # QtSvg does not support multiple transformations in one transform attribute
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

POINT = re.compile(r'([A-Za-z])((?: ?-?[\d\.]+(?:,-?[\d\.]+)?)*)?')
COORDINATE = re.compile(r'(-?[\d\.]+)(?:,(-?[\d\.]+))?')

def iterate_path_points(elem):
    assert elem.tag == '{http://www.w3.org/2000/svg}path', f"{elem.get('id', elem.tag)} is not a path"
    for point in POINT.findall(elem.get('d', 'M0,0')):
        command, coordinates = point
        yield command, [tuple(map(float, filter(lambda c: c != '', c))) for c in COORDINATE.findall(coordinates)]

@functools.lru_cache
def calculate_extremes(elem):
    if elem.tag == '{http://www.w3.org/2000/svg}path':
        c_x, c_y, min_x, min_y, max_x, max_y = 0, 0, 2**32-1, 2**32-1, 0, 0
        for command, pos in iterate_path_points(elem):
            if command in ('z', 'Z'):
                pass # NOP
            elif command in ('l', 'L', 'm', 'M'):
                x, y = pos[0]
                if command == 'l' or command == 'm':
                    c_x += x
                    c_y += y
                elif command == 'L' or command == 'M':
                    c_x = x
                    c_y = y
                min_x = min(min_x, c_x)
                min_y = min(min_y, c_y)
                max_x = max(max_x, c_x)
                max_y = max(max_y, c_y)
            elif command in ('h', 'H', 'v', 'V'):
                for a in pos:
                    if a[0]:
                        if command == 'h':
                            c_x += a[0]
                        elif command == 'H':
                            c_x = a[0]
                        elif command == 'v':
                            c_y += a[0]
                        elif command == 'V':
                            c_y = a[0]
                    min_x = min(min_x, c_x)
                    min_y = min(min_y, c_y)
                    max_x = max(max_x, c_x)
                    max_y = max(max_y, c_y)
            elif command in ('A', 'a', 'c', 'C'):
                # FIXME: This only considers end points
                x, y = pos[2 if command in ('c', 'C') else 4]
                if command == 'a' or command == 'c':
                    c_x += x
                    c_y += y
                elif command == 'A' or command == 'C':
                    c_x = x
                    c_y = y
                min_x = min(min_x, c_x)
                min_y = min(min_y, c_y)
                max_x = max(max_x, c_x)
                max_y = max(max_y, c_y)
            else:
                print(f"Unsupported command: {command} {pos}")
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

@functools.lru_cache(64)
def find_element(tree, name):
    return tree.find(f"//*[@id='{name}']")

def calculate_transformation(tree, name, elem=None):
    ox, oy = 0, 0
    if elem is None:
        elem = find_element(tree, name)
    if elem.tag == '{http://www.w3.org/2000/svg}use':
        id = elem.get('{http://www.w3.org/1999/xlink}href')[1:]
        ox, oy = calculate_transformation(tree, id)
    else:
        ox, oy = calculate_topleft(elem)
    return transform(elem, ox, oy)

def calculate_position(tree, name, elem=None):
    if elem is None:
        elem = find_element(tree, name)
    ox, oy = calculate_transformation(tree, name, elem)
    x, y = position(elem)
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
        return calculate_size(tree, find_element(tree, id))
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
    x2, y2 = calculate_position(tree, name, elem)
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

    if not __debug__:
        return

    xt, yt = calculate_position(copy_tree, name)
    if dx == 0 and dy == 0:
        xt2, yt2 = calculate_position(tree, name)
        assert zero(xt2 - xt), f"{name} moved by {xt2 - xt:.03g} in x in tree instead of 0"
        assert zero(yt2 - yt), f"{name} moved by {yt2 - yt:.03g} in y in tree instead of 0"
    else:
        xt2, yt2 = calculate_position(tree, name)
        assert zero(xt + dx - xt2), f"{name} moved by wrong amount {xt + dx - xt2:.03g} in x tree"
        assert zero(yt + dy - yt2), f"{name} moved by wrong amount {yt + dy - yt2:.03g} in y tree"

def substitute(elem, attr, value):
    elem.set(attr, value)

def adjust(elem, style, value):
    styles = elem.get('style').split(';')
    for i, current in enumerate(styles):
        if current.startswith(style + ':'):
            break
    if value[0] == "×":
        value = float(styles[i].split(':', 1)[1]) * float(value[1:])
    styles[i] = f"{style}:{value:.03g}"
    elem.set('style', ";".join(styles))

def get_matrix_for_transform(t):
    if t.startswith('translate('):
        dx, dy = translate(t)
        return 1, 0, 0, 1, dx, dy
    elif t.startswith('matrix('):
        return matrix(t)
    elif t.startswith('scale('):
        u, v = scale(t)
        return u, 0, 0, v, 0, 0
    else:
        assert False, f"Unknown transformation: {t}"

def combine(transform1, transform2):
    a11, a21, a12, a22, a13, a23 = get_matrix_for_transform(transform1)
    b11, b21, b12, b22, b13, b23 = get_matrix_for_transform(transform2)
    c11 = a11 * b11 + a12 * b21
    c12 = a11 * b12 + a12 * b22
    c13 = a11 * b13 + a12 * b23 + a13
    c21 = a21 * b11 + a22 * b21
    c22 = a21 * b12 + a22 * b22
    c23 = a21 * b13 + a22 * b23 + a23
    return f"matrix({c11:.04f},{c21:.04f},{c12:.04f},{c22:.04f},{c13:.04f},{c23:.04f})"

def find_original(elem, tree):
    original = find_element(tree, elem.get('{http://www.w3.org/1999/xlink}href')[1:])
    transform = original.get('transform')
    if original.tag == '{http://www.w3.org/2000/svg}use':
        original, transform_ = find_original(original, tree)
        if transform and transform_:
            transform = combine(transform, transform_)
        elif transform_:
            transform = transform_
    return original, transform

def isolate(elem, tree, class_names=None):
    assert elem.tag == '{http://www.w3.org/2000/svg}use'
    original, transform = find_original(elem, tree)
    clone = copy.deepcopy(original)
    for attr in ('id', 'x', 'y', 'width', 'height'):
        if elem.get(attr):
            clone.set(attr, elem.get(attr))
    if clone.tag == '{http://www.w3.org/2000/svg}g':
        for child in clone.iterchildren():
            child.set('id', clone.get('id') + '_' + child.get('id'))
            child.set('class', class_names)
    else:
        clone.set('class', class_names)
    if elem.get('transform'):
        if transform:
            transform = combine(elem.get('transform'), transform)
        else:
            transform = elem.get('transform')
        clone.set('transform', transform)
    elem.getparent().replace(elem, clone)
    return clone

def set_classes(elem, class_names):
    elem.set('class', class_names)

def set_style(elem, name, value):
    styles = elem.get('style', '').split(';')
    for i, style in enumerate(styles):
        if style == "":
            continue
        n, v = style.split(':', 1)
        if (n == name):
            styles[i] = f"{name}:{value}"
            break
    else:
        styles.append(f"{name}:{value}")
    elem.set('style', ";".join(styles))

def modify_element(tree, copy_tree, params, element, actions):
    appended = []
    for elem in element.iterchildren():
        for action in actions.get(elem.get('id'), []):
            command = action[0]
            if command == 'D': # Delete
                element.remove(elem)
            elif command == 'C': # Copy
                assert len(params[action]) in (2, 4), f"Wrong number of params for {action}, expected 2 or 4"
                appended.append((elem, *params[action]))
            elif command == 'M': # Move
                assert len(params[action]) == 2, f"Wrong number of params for {action}, expected 2"
                dx, dy = params[action]
                move(tree, elem, dx, dy, copy_tree)
            elif command == 'S': # Substitute
                assert len(params[action]) in (2, 3, 4), f"Wrong number of params for {action}, expected 2, 3 or 4"
                substitute(elem, *params[action])
            elif command == 'A': # Adjust style value
                assert len(params[action]) == 2, f"Wrong number of params for {action}, expected 2"
                adjust(elem, *params[action])
            elif command == 'I': # Isolate (/separate)
                assert len(params[action]) == 1, f"Wrong number of params for {action}, expected 1"
                elem = isolate(elem, tree, *params[action])
            elif command == 'L': # set cLasses
                assert len(params[action]) == 1, f"Wrong number of params for {action}, expected 1"
                set_classes(elem, *params[action])
            elif command == 'T': # set sTyle
                assert len(params[action]) == 2, f"Wrong number of params for {action}, expected 2"
                set_style(elem, *params[action])
        if elem.tag == '{http://www.w3.org/2000/svg}g':
            modify_element(tree, copy_tree, params, elem, actions)
    for elem, *pos in appended:
        used = elem.get('id')
        attrib = { '{http://www.w3.org/1999/xlink}href': '#' + used }
        if len(pos) == 2:
            dx, dy = pos
            attrib['transform'] = f"translate({dx:.04f},{dy:.04f})"
        elif len(pos) == 4:
            x, y = calculate_position(tree, used, elem)
            w, h = calculate_size(tree, elem)
            dx, dy, u, v = pos
            attrib['transform'] = (f"matrix({u:.04f},0,0,{v:.04f},"
                                   f"{u * -x + x - (u * w - w) / 2 + dx:.04f},"
                                   f"{v * -y + y - (v * h - h) / 2 + dy:.04f})")
        elem.addnext(elem.makeelement('use', attrib))

def modify(tree, copy_tree, actions):
    root = tree.getroot()
    params = dict(filter(lambda x: x[0][0] in ('A', 'C', 'I', 'L', 'M', 'S', 'T'), actions.items()))
    for card in root:
        name = card.get('id')
        if name in actions.keys():
            modify_element(tree, copy_tree, params, card, actions[name])

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
