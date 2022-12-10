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

#ifndef _V2F_H
#define _V2F_H

#include <math.h>

typedef struct v2f_t {
	float	x, y;
} v2f_t;


static inline v2f_t v2f_add(const v2f_t *a, const v2f_t *b)
{
	return (v2f_t){a->x + b->x, a->y + b->y};
}


static inline v2f_t v2f_sub(const v2f_t *a, const v2f_t *b)
{
	return (v2f_t){a->x - b->x, a->y - b->y};
}


static inline v2f_t v2f_mult_scalar(const v2f_t *v, float scalar)
{
	return (v2f_t){ v->x * scalar, v->y * scalar };
}


static inline v2f_t v2f_div_scalar(const v2f_t *v, float scalar)
{
	return v2f_mult_scalar(v, 1.f / scalar);
}


static inline float v2f_dot(v2f_t *a, v2f_t *b)
{
	return a->x * b->x + a->y * b->y;
}


static inline v2f_t v2f_invert(v2f_t *v)
{
	return (v2f_t){ .x = -v->x, .y = -v->y };

}


static inline float v2f_length(v2f_t *v)
{
	return sqrtf(v2f_dot(v, v));
}


static inline v2f_t v2f_normalize(v2f_t *v)
{
	float	f;

	f = 1.0f / v2f_length(v);

	return (v2f_t){ .x = f * v->x, .y = f * v->y };
}


static inline v2f_t v2f_lerp(const v2f_t *a, const v2f_t *b, float t)
{
	v2f_t	lerp_a, lerp_b;

	lerp_a = v2f_mult_scalar(a, 1.0f - t);
	lerp_b = v2f_mult_scalar(b, t);

	return v2f_add(&lerp_a, &lerp_b);
}


static inline v2f_t v2f_nlerp(const v2f_t *a, const v2f_t *b, float t)
{
	v2f_t	lerp = v2f_lerp(a, b, t);

	return v2f_normalize(&lerp);
}

#endif
