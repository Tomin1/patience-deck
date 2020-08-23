#ifndef GAMELIST_H
#define GAMELIST_H

#include <QAbstractListModel>

class GameList : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit GameList(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

    enum Roles {
        DisplayRole = Qt::DisplayRole,
        FileNameRole = Qt::UserRole
    };

private:
    static QString displayable(const QString &fileName);
    static QHash<int, QByteArray> s_roleNames;

    struct Game {
        QString fileName;
        QString displayName;
    };

    QList<Game> m_games;
};

#endif // GAMELIST_H
