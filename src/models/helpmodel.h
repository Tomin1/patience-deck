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

#ifndef HELPMODEL_H
#define HELPMODEL_H

#include <QAbstractListModel>
#include <QHash>

class HelpModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(QString helpFile READ helpFile WRITE setHelpFile NOTIFY helpFileChanged)

public:
    explicit HelpModel(QObject *parent = nullptr);

    bool ready() const;
    QString helpFile() const;
    void setHelpFile(const QString &helpFile);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

    enum Roles {
        DisplayRole = Qt::DisplayRole,
        TypeRole = Qt::UserRole,
        ContentsRole,
        ColumnsRole,
        PictureRole,
        PhraseRole,
    };

    enum Elements {
        InvalidElement,
        ParaElement,
        TitleElement,
        InformalTableElement,
        VariableListElement,
        ItemizedListElement,
        ScreenshotElement,
    };
    Q_ENUM(Elements)

signals:
    void readyChanged();
    void helpFileChanged();

private slots:
    void load();

private:
    enum XmlContentType {
        InvalidContent,
        Section,
        InformalTable,
        InformalTableThreeColumns,
            TGroup,
            TBody,
            Row,
            Entry,
        ItemizedList,
            ListItem,
        Para,
        Screenshot,
            MediaObject,
            ImageObject,
            ImageData,
            TextObject,
            Phrase,
        Title,
        VariableList,
            VarListEntry,
            Term,
    };

    friend QDebug operator<<(QDebug debug, const XmlContentType type);

    struct XmlContent {
        XmlContentType type;
        QStringList texts;

        XmlContent(XmlContentType type = InvalidContent)
            : type(type) {}
    };

    static QVariantList reformat(const QStringList &list, const QStringList &names);
    static XmlContentType contentType(const QStringRef &name);

    static QHash<int, QByteArray> s_roleNames;

    bool m_ready;
    QString m_helpFile;
    QVector<XmlContent> m_content;
};

#endif // HELPMODEL_H
