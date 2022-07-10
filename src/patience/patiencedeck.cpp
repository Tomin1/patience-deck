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

#include <glob.h>
#include <libintl.h>
#include <QCoreApplication>
#include <QFile>
#include <QJSEngine>
#include <QQmlEngine>
#include "constants.h"
#include "gamelist.h"
#include "logging.h"
#include "patiencedeck.h"

const QString Constants::DataDirectory = QStringLiteral(QUOTE(DATADIR) "/data");
const QString HarbourPrefix = QLatin1String("harbour-");
const QString IconPathTemplate = QStringLiteral("/usr/share/icons/hicolor/%1x%1/apps/%2.png");

PatienceDeck* PatienceDeck::s_app = nullptr;

QObject* PatienceDeck::instance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    if (!s_app)
        s_app = new PatienceDeck();
    return s_app;
}

PatienceDeck::PatienceDeck(QObject *parent)
    : QObject(parent)
{
}

PatienceDeck::~PatienceDeck()
{
}

QString PatienceDeck::readFile(const QString &path) const
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(lcPatience) << "Can not open" << file.fileName() << "for reading";
        return QString();
    }

    QString content = file.readAll();
    if (content.endsWith('\n'))
        content.truncate(content.length() - 1);
    return content;
}

QString PatienceDeck::aisleriotAuthors() const
{
    return readFile(Constants::DataDirectory + QStringLiteral("/AUTHORS"));
}

QString PatienceDeck::aisleriotTranslatorInfo() const
{
    const char *info = gettext("translator-credits");
    if (!info || strcmp(info, "translator-credits") == 0)
        return QString();
    return QString(info);
}

QString PatienceDeck::translators() const
{
    return readFile(Constants::DataDirectory + QStringLiteral("/TRANSLATORS"));
}

QString PatienceDeck::appName()
{
    return QCoreApplication::instance()->arguments().first().section('/', -1);
}

bool PatienceDeck::showLibraryLicenses() const
{
    return appName().startsWith(HarbourPrefix);
}

QString PatienceDeck::getIconPath(int size) const
{
    static QString mostSuitable;
    if (mostSuitable.isEmpty()) {
        QString name = appName();
        glob_t globbuf;
        QByteArray nameGlob = IconPathTemplate.arg("*").arg(name).toLocal8Bit();
        int bestSize = 86;
        if (glob(nameGlob.constData(), GLOB_NOSORT, NULL, &globbuf) == 0) {
            QStringList files;
            for (size_t i = 0; i < globbuf.gl_pathc; i++) {
                int foundSize = atoi(globbuf.gl_pathv[i] + 25);
                if (foundSize == size) {
                    bestSize = size;
                    break;
                } else if (foundSize > size) {
                    if (bestSize < size || foundSize < bestSize)
                        bestSize = foundSize;
                } else /* foundSize < size */ {
                    if (foundSize > bestSize)
                        bestSize = foundSize;
                }
            }
        }
        mostSuitable = IconPathTemplate.arg(bestSize).arg(name);
        qCDebug(lcPatience) << "Found icon at" << mostSuitable;
    }
    return mostSuitable;
}

int PatienceDeck::gamesCount() const
{
    return GameList::count();
}
