#include <QDir>
#include "gamelist.h"
#include "constants.h"

QHash<int, QByteArray> GameList::s_roleNames = {
    { Qt::DisplayRole, "display" },
    { FileNameRole, "filename" }
};

GameList::GameList(QObject *parent)
    : QAbstractListModel(parent)
{
    QDir gameDirectory(Constants::GameDirectory);
    for (QString &name : gameDirectory.entryList(QStringList() << QStringLiteral("*.scm"),
                                                 QDir::Files | QDir::Readable, 
                                                 QDir::Name)) {
        m_games.append({ name, displayable(name) });
    }
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
        return m_games[index.row()].displayName;
    case FileNameRole:
        return m_games[index.row()].fileName;
    default:
        return QVariant();
    }
}

QString GameList::displayable(const QString &fileName)
{
    QString displayName(fileName.left(fileName.length()-4));
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

QHash<int, QByteArray> GameList::roleNames() const
{
    return s_roleNames;
}
