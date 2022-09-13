/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2021 - 2022 Tomi Leppänen
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

#include <QColor>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QPainter>
#include <QSvgRenderer>
#include <QXmlStreamReader>
#include "constants.h"
#include "logging.h"
#include "perftimer.h"
#include "svgdocument.h"
#include "texturerenderer.h"

namespace {

const QString CardStyleConf = QStringLiteral("/cardStyle");
const QString CardColorConf = QStringLiteral("/cardColors");
const QString RegularStyle = QStringLiteral("regular");
const QString OptimizedStyle = QStringLiteral("optimized");
const QString SimplifiedStyle = QStringLiteral("simplified");
const QString FileTemplate = QStringLiteral("%1/anglo%2.svg");
const QString DefaultColors = QStringLiteral("default");
const QVector<QString> ColorClasses = {
    QStringLiteral("back"),
    QStringLiteral("club"),
    QStringLiteral("diamond"),
    QStringLiteral("heart"),
    QStringLiteral("spade")
};

} // namespace

TextureRenderer::TextureRenderer(QObject *parent)
    : QObject(parent)
    , m_document(nullptr)
    , m_renderer(nullptr)
    , m_cardStyleConf(Constants::ConfPath + CardStyleConf)
    , m_cardColorConf(Constants::ConfPath + CardColorConf)
{
    connect(&m_cardStyleConf, &MGConfItem::valueChanged, this, [&] {
        qCDebug(lcRenderer) << "Card style changed, rendering new card texture";
        resetDocument();
        loadDocument();
    });
    connect(&m_cardColorConf, &MGConfItem::valueChanged, this, [&] {
        qCDebug(lcRenderer) << "Card colors changed, rendering new card texture";
        loadDocument();
    });
}

void TextureRenderer::addArguments(QCommandLineParser *parser)
{
    parser->addOptions({
        {{"c", "cards"}, "Set card style", "regular|optimized|simplified"},
        {{"C", "colors"}, "Set card colors", "default|back,club,diamond,heart,spade"},
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
    if (parser->isSet("colors")) {
        MGConfItem colorsConf(Constants::ConfPath + CardColorConf);
        QString value = parser->value("colors");
        if (value == DefaultColors) {
            colorsConf.set(QVariant());
            colorsConf.sync();
        } else {
            QStringList values = value.split(',');
            for (auto it = values.begin(); it != values.end(); ++it) {
                QColor color(*it);
                if (color.isValid())
                    *it = color.name(QColor::HexRgb);
            }
            colorsConf.set(values.join(';'));
            colorsConf.sync();
        }
    }
}

QString TextureRenderer::getVariant() const
{
    QString style = m_cardStyleConf.value().toString();
    QString variant;
    if (style == OptimizedStyle || style == SimplifiedStyle)
        variant = QStringLiteral("-%1").arg(style);
    return variant;
}

void TextureRenderer::resetDocument()
{
    if (m_document) {
        m_document->deleteLater();
        m_document = nullptr;
    }
}

QHash<QString, QString> TextureRenderer::getColors() const
{
    QHash<QString, QString> colors;
    QString value = m_cardColorConf.value().toString();
    if (!value.isEmpty()) {
        QStringList values = value.split(';', QString::KeepEmptyParts);
        auto vIt = values.constBegin();
        auto cIt = ColorClasses.constBegin();
        for (; vIt != values.constEnd() && cIt != ColorClasses.constEnd(); ++vIt, ++cIt) {
            if (vIt->isEmpty())
                continue;
            QColor color(*vIt);
            if (color.isValid())
                colors.insert(*cIt, color.name(QColor::HexRgb));
            else
                qCWarning(lcRenderer) << "Invalid color in config:" << *vIt;
        }
    }
    return colors;
}

void TextureRenderer::loadDocument()
{
    measurePerf();
    if (!m_document)
        m_document = new SvgDocument(FileTemplate.arg(Constants::DataDirectory).arg(getVariant()));
    m_document->load(getColors());
    emit documentLoaded();
    resetRenderer();
    renderTexture(m_size);
}

QSvgRenderer *TextureRenderer::renderer()
{
    if (!m_renderer) {
        QXmlStreamReader reader(m_document);
        m_renderer = new QSvgRenderer(&reader, this);
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
    if (m_document && size.isValid()) {
        emit textureRenderingStarted();
        m_size = size;
        QImage image(size, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&image);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(0, 0, size.width(), size.height(), Qt::transparent);
        painter.setRenderHint(QPainter::Antialiasing);
        renderer()->render(&painter);
        qCDebug(lcRenderer) << "Drew new texture of size" << size;
        emit textureRendered(image, size);
    }
}

void TextureRenderer::measurePerf()
{
    if (lcRendererPerf().isDebugEnabled()) {
        PerfTimer *timer = new PerfTimer(this);
        connect(this, &TextureRenderer::documentLoaded, timer, [timer] {
            quint64 time = timer->nsecsElapsed() / 1000000;
            qCDebug(lcRendererPerf) << "Document loaded in" << time << "ms";
        });
        connect(this, &TextureRenderer::textureRenderingStarted, timer, [timer] {
            quint64 time = timer->nsecsElapsed() / 1000000;
            qCDebug(lcRendererPerf) << "Texture drawing started at" << time << "ms";
        });
        connect(this, &TextureRenderer::textureRendered, timer, [timer] {
            quint64 time = timer->nsecsElapsed() / 1000000;
            qCDebug(lcRendererPerf) << "Texture drawn in" << time << "ms";
            timer->deleteLater();
        });
        timer->start();
    }
}
