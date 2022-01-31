/*
 * Patience Deck is a collection of patience games.
 * Copyright (C) 2020-2022 Tomi Leppänen
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

#ifndef LOGGING_H
#define LOGGING_H

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(lcPatience);
Q_DECLARE_LOGGING_CATEGORY(lcTable);
Q_DECLARE_LOGGING_CATEGORY(lcDrag);
Q_DECLARE_LOGGING_CATEGORY(lcManager);
Q_DECLARE_LOGGING_CATEGORY(lcQueue);
Q_DECLARE_LOGGING_CATEGORY(lcSlot);
Q_DECLARE_LOGGING_CATEGORY(lcGameList);
Q_DECLARE_LOGGING_CATEGORY(lcOptionList);
Q_DECLARE_LOGGING_CATEGORY(lcHelpModel);
Q_DECLARE_LOGGING_CATEGORY(lcTimer);
Q_DECLARE_LOGGING_CATEGORY(lcMouse);
Q_DECLARE_LOGGING_CATEGORY(lcEngine);
Q_DECLARE_LOGGING_CATEGORY(lcOptions);
Q_DECLARE_LOGGING_CATEGORY(lcScheme);

#endif // LOGGING_H
