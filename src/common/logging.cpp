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

#include <QLoggingCategory>
#include "logging.h"

Q_LOGGING_CATEGORY(lcPatience, "site.tomin.patience", QtWarningMsg);
Q_LOGGING_CATEGORY(lcTestMode, "site.tomin.patience.test", QtInfoMsg);
Q_LOGGING_CATEGORY(lcTable, "site.tomin.patience.table", QtWarningMsg);
Q_LOGGING_CATEGORY(lcRenderer, "site.tomin.patience.table.renderer", QtWarningMsg);
Q_LOGGING_CATEGORY(lcRendererPerf, "site.tomin.patience.table.renderer.perf", QtWarningMsg);
Q_LOGGING_CATEGORY(lcDrag, "site.tomin.patience.table.drag", QtWarningMsg);
Q_LOGGING_CATEGORY(lcSelection, "site.tomin.patience.table.selection", QtWarningMsg);
Q_LOGGING_CATEGORY(lcManager, "site.tomin.patience.table.manager", QtWarningMsg);
Q_LOGGING_CATEGORY(lcQueue, "site.tomin.patience.table.manager.queue", QtWarningMsg);
Q_LOGGING_CATEGORY(lcSlot, "site.tomin.patience.table.slot", QtWarningMsg);
Q_LOGGING_CATEGORY(lcGameList, "site.tomin.patience.gamelist", QtWarningMsg);
Q_LOGGING_CATEGORY(lcOptionList, "site.tomin.patience.optionlist", QtWarningMsg);
Q_LOGGING_CATEGORY(lcHelpModel, "site.tomin.patience.help", QtWarningMsg);
Q_LOGGING_CATEGORY(lcTimer, "site.tomin.patience.timer", QtWarningMsg);
Q_LOGGING_CATEGORY(lcMouse, "site.tomin.patience.mouse", QtWarningMsg);
Q_LOGGING_CATEGORY(lcEngine, "site.tomin.patience.engine", QtWarningMsg);
Q_LOGGING_CATEGORY(lcRecorder, "site.tomin.patience.engine.recorder", QtWarningMsg);
Q_LOGGING_CATEGORY(lcOptions, "site.tomin.patience.engine.options", QtWarningMsg);
Q_LOGGING_CATEGORY(lcScheme, "site.tomin.patience.scheme", QtWarningMsg);
