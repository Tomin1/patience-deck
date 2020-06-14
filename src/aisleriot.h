#ifndef AISLERIOT_H
#define AISLERIOT_H

#include <QObject>

class AisleriotPrivate;
class QQmlEngine;
class QJSEngine;

class Aisleriot : public QObject
{
    Q_OBJECT

public:
    static Aisleriot* instance();
    static QObject* instance(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~Aisleriot();

private:
    friend AisleriotPrivate;

    explicit Aisleriot(QObject *parent = nullptr);
    static void interfaceInit(void *data);

    static Aisleriot *s_game;
    AisleriotPrivate *d_ptr;
};

#endif // AISLERIOT_H
