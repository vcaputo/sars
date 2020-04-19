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

#ifndef _V3F_H
#define _V3F_H

#include <math.h>

typedef struct v3f_t {
	float	x, y, z;
} v3f_t;


static inline int v3f_equal(const v3f_t *a, const v3f_t *b)
{
	return (a->x == b->x && a->y == b->y && a->z == b->z);
}


static inline v3f_t v3f_add(const v3f_t *a, const v3f_t *b)
{
	return (v3f_t){a->x + b->x, a->y + b->y, a->z + b->z};
}


static inline v3f_t v3f_sub(const v3f_t *a, const v3f_t *b)
{
	return (v3f_t){a->x - b->x, a->y - b->y, a->z - b->z};
}


static inline v3f_t v3f_mult(const v3f_t *a, const v3f_t *b)
{
	return (v3f_t){a->x * b->x, a->y * b->y, a->z * b->z};
}


static inline v3f_t v3f_mult_scalar(const v3f_t *v, float scalar)
{
	return (v3f_t){v->x * scalar, v->y * scalar, v->z * scalar};
}


static inline float v3f_dot(v3f_t *a, v3f_t *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}


static inline float v3f_length(v3f_t *v)
{
	return sqrtf(v3f_dot(v, v));
}


static inline v3f_t v3f_normalize(v3f_t *v)
{
	return v3f_mult_scalar(v, 1.0f / v3f_length(v));
}


static inline v3f_t v3f_cross(const v3f_t *a, const v3f_t *b)
{
	v3f_t	v;

	v.x = a->y * b->z - a->z * b->y;
	v.y = a->z * b->x - a->x * b->z;
	v.z = a->x * b->y - a->y * b->x;

	return v;
}


static inline v3f_t v3f_lerp(const v3f_t *a, const v3f_t *b, float t)
{
	v3f_t	lerp_a, lerp_b;

	lerp_a = v3f_mult_scalar(a, 1.0f - t);
	lerp_b = v3f_mult_scalar(b, t);

	return v3f_add(&lerp_a, &lerp_b);
}


static inline v3f_t v3f_nlerp(const v3f_t *a, const v3f_t *b, float t)
{
	v3f_t	lerp = v3f_lerp(a, b, t);

	return v3f_normalize(&lerp);
}

#endif
