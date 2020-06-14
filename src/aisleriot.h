#ifndef AISLERIOT_H
#define AISLERIOT_H

#include <QObject>

class Aisleriot : public QObject
{
    Q_OBJECT
public:
    explicit Aisleriot(QObject *parent = nullptr);
    ~Aisleriot();

private:
    static void interface_init(void *data);
};

#endif // AISLERIOT_H
