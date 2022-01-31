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

#include <QFile>
#include <QRegularExpression>
#include <QUrl>
#include <QXmlStreamReader>
#include "helpmodel.h"
#include "logging.h"

QHash<int, QByteArray> HelpModel::s_roleNames = {
    { Qt::DisplayRole, "text" },
    { TypeRole, "type" },
    { ContentsRole, "contents" },
    { ColumnsRole, "columns" },
    { PictureRole, "picture" },
    { PhraseRole, "phrase" },
};

HelpModel::HelpModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_ready(false)
{
}

bool HelpModel::ready() const
{
    return m_ready;
}

QString HelpModel::helpFile() const
{
    return m_helpFile;
}

void HelpModel::setHelpFile(const QString &helpFile)
{
    if (m_helpFile != helpFile) {
        m_helpFile = helpFile;
        if (!m_content.isEmpty()) {
            qCWarning(lcHelpModel) << "Clearing help model";
            beginRemoveRows(QModelIndex(), 0, m_content.count()-1);
            m_content.clear();
            endRemoveRows();
        }
        emit helpFileChanged();
        if (m_ready) {
            m_ready = false;
            emit readyChanged();
        }
        QMetaObject::invokeMethod(this, "load", Qt::QueuedConnection);
    }
}

int HelpModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_content.count();
}

QVariant HelpModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    XmlContentType type = m_content[index.row()].type;
    switch (role) {
    case DisplayRole:
        if (type == Para || type == Title)
            return m_content[index.row()].texts.at(0);
        break;
    case TypeRole:
        switch (type) {
        case Para:
            return ParaElement;
        case Title:
            return TitleElement;
        case InformalTable:
        case InformalTableThreeColumns:
            return InformalTableElement;
        case VariableList:
            return VariableListElement;
        case ItemizedList:
            return ItemizedListElement;
        case Screenshot:
            return ScreenshotElement;
        default:
            return InvalidElement;
        }
    case ContentsRole:
        switch (type) {
        case InformalTable:
        case InformalTableThreeColumns:
        case ItemizedList:
            return m_content[index.row()].texts;
        case VariableList:
            return reformat(m_content[index.row()].texts, { QStringLiteral("label"), QStringLiteral("value") });
        default:
            break;
        }
        break;
    case ColumnsRole:
        if (type == InformalTable)
            return 2;
        if (type == InformalTableThreeColumns)
            return 3;
        break;
    case PictureRole:
        if (type == Screenshot)
            return QUrl::fromLocalFile(m_helpFile).resolved(m_content[index.row()].texts.at(0));
        break;
    case PhraseRole:
        if (type == Screenshot)
            return m_content[index.row()].texts.at(1);
        break;
    }

    return QVariant();
}

QHash<int, QByteArray> HelpModel::roleNames() const
{
    return s_roleNames;
}

QVariantList HelpModel::reformat(const QStringList &list, const QStringList &names)
{
    QVariantList result;
    for (int i = 0; i < list.length() / names.length(); ++i) {
        QVariantMap map;
        for (int j = 0; j < names.length(); ++j)
            map[names.at(j)] = list.at(i * names.length() + j);
        result << map;
    }
    return result;
}

HelpModel::XmlContentType HelpModel::contentType(const QStringRef &name)
{
    if (name.startsWith(QLatin1String("sect"))) {
        return Section;
    } else if (name == QLatin1String("informaltable")) {
        return InformalTable;
    } else if (name == QLatin1String("tgroup")) {
        return TGroup;
    } else if (name == QLatin1String("tbody")) {
        return TBody;
    } else if (name == QLatin1String("row")) {
        return Row;
    } else if (name == QLatin1String("entry")) {
        return Entry;
    } else if (name == QLatin1String("itemizedlist")) {
        return ItemizedList;
    } else if (name == QLatin1String("listitem")) {
        return ListItem;
    } else if (name == QLatin1String("para")) {
        return Para;
    } else if (name == QLatin1String("screenshot")) {
        return Screenshot;
    } else if (name == QLatin1String("mediaobject")) {
        return MediaObject;
    } else if (name == QLatin1String("imageobject")) {
        return ImageObject;
    } else if (name == QLatin1String("imagedata")) {
        return ImageData;
    } else if (name == QLatin1String("textobject")) {
        return TextObject;
    } else if (name == QLatin1String("phrase")) {
        return Phrase;
    } else if (name == QLatin1String("title")) {
        return Title;
    } else if (name == QLatin1String("variablelist")) {
        return VariableList;
    } else if (name == QLatin1String("varlistentry")) {
        return VarListEntry;
    } else if (name == QLatin1String("term")) {
        return Term;
    } else {
        return InvalidContent;
    }
}

void HelpModel::load()
{
    QFile file(m_helpFile);

    if (!file.open(QFile::ReadOnly)) {
        qCCritical(lcHelpModel) << "Failed to open help file" << m_helpFile << file.errorString();
        return;
    }

    QXmlStreamReader reader;
    reader.setDevice(&file);

    QVector<XmlContent> newContent;
    XmlContent content;
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::StartElement) {
            XmlContentType type = contentType(reader.name());
            qCDebug(lcHelpModel) << "Found start of" << type;
            switch (type) {
            // NOPs
            case InvalidContent:
            case Section:
            case TBody: // InformalTable
            case Row: // InformalTable
            case ListItem: // ItemizedList/VariableList
            case MediaObject: // Screenshot
            case ImageObject: // Screenshot
            case TextObject: // Screenshot
            case VarListEntry: // VariableList
            case InformalTableThreeColumns:
                break;
            // Models
            case InformalTable:
            case ItemizedList:
            case Screenshot:
            case VariableList:
                content = XmlContent(type);
                break;
            // Attributes
            case TGroup: // InformalTable
                if (reader.attributes().value(QString(), QStringLiteral("cols")) == QLatin1String("3"))
                    content.type = InformalTableThreeColumns;
                break;
            case ImageData: // Screenshot
                content.texts.append(reader.attributes().value(QString(), QStringLiteral("fileref")).toString());
                break;
            // Text content
            case Para:
            case Title:
                if (content.type == InvalidContent)
                    content = XmlContent(type);
                [[fallthrough]];
            // List/Table content
            case Entry: // InformalTable
            case Phrase: // Screenshot
            case Term: // VariableList
                content.texts.append(reader.readElementText(QXmlStreamReader::IncludeChildElements));
                break;
            }
        }
        if (reader.tokenType() == QXmlStreamReader::EndElement) {
            XmlContentType type = contentType(reader.name());
            qCDebug(lcHelpModel) << "Found end of" << type;
            if (content.type == type || (content.type == InformalTableThreeColumns && type == InformalTable)) {
                qCInfo(lcHelpModel) << "Storing" << type;
                newContent.append(content);
                content = XmlContent();
            }
        }
    }

    if (reader.hasError()) {
        qCCritical(lcHelpModel) << "Failed to read help file" << m_helpFile << reader.errorString();
        return;
    }

    qCDebug(lcHelpModel) << "Loaded" << m_helpFile << "with" << newContent.count() << "elements";
    beginInsertRows(QModelIndex(), 0, newContent.count()-1);
    m_content.swap(newContent);
    endInsertRows();

    m_ready = true;
    emit readyChanged();
}

QDebug operator<<(QDebug debug, const HelpModel::XmlContentType type)
{
    static const QList<const char *> names = {
        "InvalidContent",
        "Section",
        "InformalTable",
        "InformalTableThreeColumns",
        "TGroup",
        "TBody",
        "Row",
        "Entry",
        "ItemizedList",
        "ListItem",
        "Para",
        "Screenshot",
        "MediaObject",
        "ImageObject",
        "ImageData",
        "TextObject",
        "Phrase",
        "Title",
        "VariableList",
        "VarListEntry",
        "Term"
    };
    debug.nospace() << "XmlContentType(" << names.at(type < names.length() ? type : 0) << ")";
    return debug.space();
}
