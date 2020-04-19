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

#ifndef _BB2F_H
#define _BB2F_H

#include "v2f.h"

typedef struct bb2f_t {
	v2f_t	min, max;
} bb2f_t;


/* linearly interpolate between a and b by t */
static inline bb2f_t bb2f_lerp(const bb2f_t *a, const bb2f_t *b, float t)
{
	bb2f_t	bb2f;

	bb2f.min = v2f_lerp(&a->min, &b->min, t);
	bb2f.max = v2f_lerp(&a->max, &b->max, t);

	return bb2f;
}

#endif
