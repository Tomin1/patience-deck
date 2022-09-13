/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2022 Tomi Leppänen
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

#ifndef SVGDOCUMENT_H
#define SVGDOCUMENT_H

#include <QBuffer>
#include <QHash>
#include <QString>

class SvgDocument : public QBuffer
{
    Q_OBJECT
public:
    explicit SvgDocument(const QString &path, QObject *parent = nullptr);
    void load(QHash<QString, QString> colours);

private:
    QString m_path;
};

#endif // SVGDOCUMENT_H
