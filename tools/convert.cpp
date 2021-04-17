/*
 * Convert SVG icon to png
 * Copyright (c) 2021 Tomi Leppänen
 *
 * Permission to use, copy, modify, and/or distribute this file for any purpose
 * with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <QtGlobal>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QList>
#include <QPainter>
#include <QSize>
#include <QString>
#include <QSvgRenderer>
#include <stdio.h>

QSize size(const QString &str)
{
    QStringList vals = QString(str).split("x");
    if (vals.size() != 2)
        return QSize();
    bool ok;
    int width = vals[0].toInt(&ok);
    if (!ok)
        return QSize();
    int height = vals[1].toInt(&ok);
    if (!ok)
        return QSize();
    return QSize(width, height);
}

bool renderIcon(const QString &svgFile, const QSize &size, const QString &outputFile)
{
    if (!QFileInfo(outputFile).dir().mkpath("."))
        return false;
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    QSvgRenderer(svgFile).render(&painter);
    return image.save(outputFile);
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr, "Too few arguments\n");
        return 2;
    }

    QString svgFile = argv[1];
    if (!QFileInfo::exists(svgFile)) {
        fprintf(stderr, "SVG file does not exist: %s\n", qPrintable(svgFile));
        return 2;
    }

    QString outputTemplate = argv[2];
    if (!outputTemplate.contains("%1")) {
        fprintf(stderr, "Output file template must contain '%1' for size: %s\n",
                qPrintable(outputTemplate));
        return 2;
    }

    QList<QPair<QString, QSize>> sizes;
    for (int i = 3; i < argc; i++) {
        QString iconName(argv[i]);
        QSize iconSize = size(iconName);
        if (iconSize.isNull()) {
            fprintf(stderr, "Invalid size, must be <width>x<height>: %s\n", qPrintable(argv[i]));
            return 2;
        }
        sizes.append(QPair<QString, QSize>(iconName, iconSize));
    }

    for (const auto &icon : sizes) {
        QString filepath = outputTemplate.arg(icon.first);
        if (!renderIcon(svgFile, icon.second, filepath))
            fprintf(stderr, "Could not save: %s\n", qPrintable(filepath));
    }

    return 0;
}

/* vim: set expandtab smarttab ts=4 sw=4: */
