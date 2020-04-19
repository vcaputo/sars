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

#ifndef _V4F_H
#define _V4F_H

#include <math.h>

typedef struct v4f_t {
	float	x, y, z, w;
} v4f_t;


static inline v4f_t v4f_add(const v4f_t *a, const v4f_t *b)
{
	return (v4f_t){a->x + b->x, a->y + b->y, a->z + b->z, a->w + b->w};
}


static inline v4f_t v4f_sub(const v4f_t *a, const v4f_t *b)
{
	return (v4f_t){a->x - b->x, a->y - b->y, a->z - b->z, a->w - b->w};
}


static inline v4f_t v4f_mult(const v4f_t *a, const v4f_t *b)
{
	return (v4f_t){a->x * b->x, a->y * b->y, a->z * b->z, a->w * b->w};
}


static inline v4f_t v4f_mult_scalar(const v4f_t *v, float scalar)
{
	return (v4f_t){v->x * scalar, v->y * scalar, v->z * scalar, v->w * scalar};
}


static inline float v4f_dot(v4f_t *a, v4f_t *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}


static inline float v4f_length(v4f_t *v)
{
	return sqrtf(v4f_dot(v, v));
}


static inline v4f_t v4f_normalize(v4f_t *v)
{
	return v4f_mult_scalar(v, 1.0f / v4f_length(v));
}


static inline v4f_t v4f_lerp(const v4f_t *a, const v4f_t *b, float t)
{
	v4f_t	lerp_a, lerp_b;

	lerp_a = v4f_mult_scalar(a, 1.0f - t);
	lerp_b = v4f_mult_scalar(b, t);

	return v4f_add(&lerp_a, &lerp_b);
}


static inline v4f_t v4f_nlerp(const v4f_t *a, const v4f_t *b, float t)
{
	v4f_t	lerp = v4f_lerp(a, b, t);

	return v4f_normalize(&lerp);
}

#endif
