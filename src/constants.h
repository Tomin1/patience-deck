#ifndef DATA_H
#define DATA_H

#include <QString>

#define _QUOTE(word) #word
#define QUOTE(word) _QUOTE(word)

namespace Constants {
extern const qint64 ClickTimeout;
extern const qreal DragDistance;
extern const QString GameDirectory;
extern const QString DataDirectory;
};

#endif // DATA_H
