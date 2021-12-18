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

#include <QCommandLineParser>
#include <QPainter>
#include <QSvgRenderer>
#include "constants.h"
#include "logging.h"
#include "texturerenderer.h"

namespace {

const QString CardStyleConf = QStringLiteral("/cardStyle");
const QString RegularStyle = QStringLiteral("regular");
const QString OptimizedStyle = QStringLiteral("optimized");
const QString SimplifiedStyle = QStringLiteral("simplified");
const QString FileTemplate = QStringLiteral("%1/anglo%2.svg");

} // namespace

TextureRenderer::TextureRenderer(QObject *parent)
    : QObject(parent)
    , m_renderer(nullptr)
    , m_cardStyleConf(Constants::ConfPath + CardStyleConf)
{

    connect(&m_cardStyleConf, &MGConfItem::valueChanged, this, [&] {
        qCDebug(lcTable) << "Card style changed, rendering new card texture";
        resetRenderer();
        renderTexture(m_size);
    });
}

void TextureRenderer::addArguments(QCommandLineParser *parser)
{
    parser->addOptions({
        {{"c", "cards"}, "Set card style", "regular|optimized|simplified"},
    });
}

void TextureRenderer::setArguments(QCommandLineParser *parser)
{
    if (parser->isSet("cards")) {
        MGConfItem cardsConf(Constants::ConfPath + CardStyleConf);
        QString value = parser->value("cards");
        if (value == RegularStyle || value == OptimizedStyle || value == SimplifiedStyle) {
            cardsConf.set(value);
            cardsConf.sync();
        }
    }
}

QSvgRenderer *TextureRenderer::renderer()
{
    if (!m_renderer) {
        QString style = m_cardStyleConf.value().toString();
        QString variant;
        if (style == OptimizedStyle || style == SimplifiedStyle)
            variant = QStringLiteral("-%1").arg(style);
        m_renderer = new QSvgRenderer(FileTemplate.arg(Constants::DataDirectory).arg(variant), this);
    }
    return m_renderer;
}

void TextureRenderer::resetRenderer()
{
    if (m_renderer) {
        m_renderer->deleteLater();
        m_renderer = nullptr;
    }
}

void TextureRenderer::renderTexture(const QSize &size)
{
    if (size.isValid()) {
        m_size = size;
        QImage image(size, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&image);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::transparent);
        painter.setRenderHint(QPainter::Antialiasing);
        renderer()->render(&painter);
        qCDebug(lcTable) << "Drew new texture of size" << size;
        emit textureRendered(image, size);
    }
}
