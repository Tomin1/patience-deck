#include <QDir>
#include "gamelist.h"
#include "constants.h"

QHash<int, QByteArray> GameList::s_roleNames = {
    { Qt::DisplayRole, "display" },
    { FileNameRole, "filename" },
    { NameRole, "name" }
};

GameList::GameList(QObject *parent)
    : QAbstractListModel(parent)
{
    QDir gameDirectory(Constants::GameDirectory);
    m_games = gameDirectory.entryList(QStringList() << QStringLiteral("*.scm"),
                                      QDir::Files | QDir::Readable, QDir::Name);
}

int GameList::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_games.count();
}

QVariant GameList::data(const QModelIndex &index, int role) const
{
    if (index.row() > m_games.count())
        return QVariant();

    switch (role) {
    case DisplayRole:
        return displayable(m_games[index.row()]);
    case FileNameRole:
        return m_games[index.row()];
    case NameRole:
        return name(m_games[index.row()]);
    default:
        return QVariant();
    }
}

QString GameList::displayable(const QString &fileName)
{
    QString displayName(name(fileName));
    bool startOfWord = true;
    for (int i = 0; i < displayName.length(); i++) {
        if (startOfWord) {
            displayName[i] = QChar(displayName[i]).toUpper();
            startOfWord = false;
        } else if (displayName[i] == QChar('-')) {
            startOfWord = true;
            displayName[i] = QChar(' ');
        }
    }
    return displayName;
}

QString GameList::name(const QString &fileName)
{
    return fileName.left(fileName.length()-4);
}

QHash<int, QByteArray> GameList::roleNames() const
{
    return s_roleNames;
}
