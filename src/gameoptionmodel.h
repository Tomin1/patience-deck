#ifndef GAMEOPTIONLIST_H
#define GAMEOPTIONLIST_H

#include <QAbstractListModel>
#include "enginedata.h"

class GameOptionModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged);

public:
    explicit GameOptionModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QHash<int, QByteArray> roleNames() const;
    Q_INVOKABLE void select(int index);

    enum Roles {
        DisplayRole = Qt::DisplayRole,
        SetRole = Qt::UserRole,
        TypeRole = Qt::UserRole+1
    };

    enum Types {
        CheckType,
        RadioType
    };

signals:
    void doRequestGameOptions();
    void doSetGameOption(const GameOption &option);
    void doSetGameOptions(const GameOptionList &options);
    void countChanged();

private slots:
    void handleGameOptions(GameOptionList options);

private:
    static QHash<int, QByteArray> s_roleNames;

    GameOptionList m_options;
};

#endif // GAMEOPTIONLIST_H
