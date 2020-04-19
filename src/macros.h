/*
 *  Copyright (C) 2018-2020 - Vito Caputo - <vcaputo@pengaru.com>
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 3 as published
 *  by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MACROS_H
#define _MACROS_H

#include <stdio.h>
#include <stdlib.h>

#define fatal_if(_cond, _fmt, ...) \
	if (_cond) { \
		fprintf(stderr, "Fatal error: " _fmt "\n", ##__VA_ARGS__); \
		exit(EXIT_FAILURE); \
	}

#define warn_if(_cond, _fmt, ...) \
	if (_cond) { \
		fprintf(stderr, "Warning: " _fmt "\n", ##__VA_ARGS__); \
	}

#define NELEMS(_a) \
	(sizeof(_a) / sizeof(_a[0]))

#define CSTRLEN(_str) \
	(sizeof(_str) - 1)

#ifndef MIN
#define MIN(_a, _b) \
	((_a) < (_b) ? (_a) : (_b))
#endif

#ifndef MAX
#define MAX(_a, _b) \
	((_a) > (_b) ? (_a) : (_b))
#endif

#ifndef STRINGIFY
#define _STRINGIFY(_str) \
	#_str
#define STRINGIFY(_str) \
	_STRINGIFY(_str)
#endif

#endif
