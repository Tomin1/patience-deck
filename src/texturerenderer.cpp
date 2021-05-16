/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021 Tomi Leppänen
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

#include <QPainter>
#include <QSvgRenderer>
#include "constants.h"
#include "logging.h"
#include "texturerenderer.h"

TextureRenderer::TextureRenderer(QObject *parent)
    : QObject(parent)
{
}

QSvgRenderer *TextureRenderer::renderer()
{
    static QSvgRenderer *renderer = nullptr;
    if (!renderer)
        renderer = new QSvgRenderer(Constants::DataDirectory + QStringLiteral("/anglo.svg"));
    return renderer;
}

void TextureRenderer::renderTexture(const QSize &size)
{
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(0, 0, size.width(), size.height(), Qt::transparent);
    painter.setRenderHint(QPainter::Antialiasing);
    renderer()->render(&painter);
    qCDebug(lcTable) << "Drew new texture of size" << size;
    emit textureRendered(image, size);
}
