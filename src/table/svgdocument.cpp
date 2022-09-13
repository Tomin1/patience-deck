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

#include <QDomDocument>
#include <QFile>
#include "logging.h"
#include "svgdocument.h"

namespace {

const QString ClassAttribute = QStringLiteral("class");
const QString StyleAttribute = QStringLiteral("style");
const QString FillStyle = QStringLiteral("fill");
const QString StrokeStyle = QStringLiteral("stroke");
const QString StyleTemplate = QStringLiteral("%1:%2");

class DomDocumentIterator {
    DomDocumentIterator(QDomDocument &document, bool atEnd = false)
        : mDocument(document)
    {
        if (!atEnd) {
            mCurrent = document.documentElement();
        } else {
            mCurrent = QDomElement();
            if (!mCurrent.isNull())
                qCCritical(lcTable) << "End iterator is not at end!";
        }
    }

public:
    static DomDocumentIterator begin(QDomDocument &document)
    {
        return DomDocumentIterator(document);
    }

    static DomDocumentIterator end(QDomDocument &document)
    {
        return DomDocumentIterator(document, true);
    }

    void operator=(const DomDocumentIterator& other)
    {
        mDocument = other.mDocument;
        mCurrent = other.mCurrent;
    }

    bool operator!=(const DomDocumentIterator& other) const
    {
        if (&mDocument != &other.mDocument)
            return true;
        if (mCurrent.isNull() && other.mCurrent.isNull())
            return false;
        return mCurrent != other.mCurrent;
    }

    DomDocumentIterator &operator++()
    {
        if (!mCurrent.isNull()) {
            /*
             * Try child, if available use that.
             * Otherwise try sibling, if available use that.
             * Otherwise if element doesn't have a parent, end iteration.
             * Otherwise go to parent and try its sibling, if available use that.
             * Otherwise repeat from parent check.
             */
            QDomElement next = mCurrent.firstChildElement();
            if (next.isNull())
                next = mCurrent.nextSiblingElement();
            QDomNode parent = mCurrent.parentNode();
            while (next.isNull()) {
                if (parent.isNull()) {
                    qCDebug(lcTable) << "End of iteration";
                    next = QDomElement(); // Ensure that next is null
                    break;
                }
                next = parent.nextSiblingElement();
                parent = parent.parentNode();
            }
            mCurrent = next;
        }
        return *this;
    }

    QDomElement &operator*()
    {
        return mCurrent;
    }

    QDomElement *operator->()
    {
        return &mCurrent;
    }

private:
    QDomDocument &mDocument;
    QDomElement mCurrent;
};

QDomDocument getDocument(const QString &path) {
    QDomDocument document;
    QFile file(path);
    if (file.exists())
        document.setContent(&file);
    return document;
}

void setStyle(QDomElement &element, bool fill, bool stroke, QString value) {
    auto styles = element.attribute(StyleAttribute).split(';', QString::SkipEmptyParts);
    for (auto it = styles.begin(); it != styles.end() && fill && stroke; ++it) {
        auto parts = it->split(':');
        if (parts.length() != 2) {
            qCCritical(lcTable) << "Invalid style";
            break;
        }
        if (fill && parts[0] == FillStyle) {
            *it = StyleTemplate.arg(FillStyle, value);
            fill = false;
        } else if (stroke && parts[0] == StrokeStyle) {
            *it = StyleTemplate.arg(StrokeStyle, value);
            stroke = false;
        }
    }
    if (fill)
        styles.append(StyleTemplate.arg(FillStyle, value));
    if (stroke)
        styles.append(StyleTemplate.arg(StrokeStyle, value));
    element.setAttribute(StyleAttribute, styles.join(';'));
}

void modifyDocument(QDomDocument &document, const QHash<QString, QString> &colours) {
    if (colours.isEmpty())
        return;
    for (auto it = DomDocumentIterator::begin(document); it != DomDocumentIterator::end(document); ++it) {
        const auto classes = it->attribute(ClassAttribute).split(' ', QString::SkipEmptyParts);
        for (const auto &className : classes) {
            if (colours.contains(className)) {
                setStyle(*it, classes.contains(FillStyle), classes.contains(StrokeStyle),
                         colours.value(className));
                break;
            }
        }
    }
}

} // namespace

SvgDocument::SvgDocument(const QString &path, QObject *parent)
    : QBuffer(parent)
    , m_path(path)
{
}

void SvgDocument::load(QHash<QString, QString> colours)
{
    auto document = getDocument(m_path);
    modifyDocument(document, colours);
    open(QIODevice::WriteOnly);
    QTextStream stream(this);
    document.save(stream, 0);
    close();
    open(QIODevice::ReadOnly);
}
