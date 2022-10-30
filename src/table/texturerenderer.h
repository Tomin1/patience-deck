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

#ifndef TEXTURERENDERER_H
#define TEXTURERENDERER_H

#include <MGConfItem>
#include <QImage>
#include <QObject>
#include <QSize>

class QCommandLineParser;
class QSvgRenderer;
class SvgDocument;
class TextureRenderer : public QObject
{
    Q_OBJECT

public:
    explicit TextureRenderer(QObject *parent = nullptr);

    static void addArguments(QCommandLineParser *parser);
    static void setArguments(QCommandLineParser *parser);

public slots:
    void loadDocument();
    void renderTexture(const QSize &size, bool drawDoubleSize);

signals:
    void textureRendered(QImage image, const QSize &size);
    void doubleSizeTextureRendered(QImage image, const QSize &size);
    // These are only for measuring performance
    void documentLoaded();
    void textureRenderingStarted();

private:
    QString getVariant() const;
    QHash<QString, QString> getColors() const;
    void resetDocument();
    QSvgRenderer *renderer();
    void resetRenderer();
    QImage drawTexture(const QSize &size);
    void measurePerf();

    SvgDocument *m_document;
    QSvgRenderer *m_renderer;
    MGConfItem m_cardStyleConf;
    MGConfItem m_cardColorConf;
    QSize m_size;
    bool m_drawDoubleSize;
};

#endif // TEXTURERENDERER_H
