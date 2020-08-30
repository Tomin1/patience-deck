#ifndef GAMELIST_H
#define GAMELIST_H

#include <QAbstractListModel>

class GameList : public QAbstractListModel
{
    Q_OBJECT

public:
    static QString displayable(const QString &fileName);
    static QString name(const QString &fileName);

    explicit GameList(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;

    enum Roles {
        DisplayRole = Qt::DisplayRole,
        FileNameRole = Qt::UserRole,
        NameRole = Qt::UserRole + 1,
    };

private:
    static QHash<int, QByteArray> s_roleNames;

    QStringList m_games;
};

#endif // GAMELIST_H
