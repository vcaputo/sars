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

#ifndef _BB3F_H
#define _BB3F_H

#include "v3f.h"

typedef struct bb3f_t {
	v3f_t	min, max;
} bb3f_t;


/* linearly interpolate between a and b by t */
static inline bb3f_t bb3f_lerp(const bb3f_t *a, const bb3f_t *b, float t)
{
	bb3f_t	bb3f;

	bb3f.min = v3f_lerp(&a->min, &b->min, t);
	bb3f.max = v3f_lerp(&a->max, &b->max, t);

	return bb3f;
}

#endif
