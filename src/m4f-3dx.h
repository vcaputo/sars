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

#ifndef _M4F_3DX_H
#define _M4F_3DX_H

#include <math.h>

#include "m4f.h"
#include "v3f.h"

/* helpers for manipulating 3D transformation matrices */

/* XXX: note this is column-major */

/* adjust the matrix m to translate by v, returning the resulting matrix */
/* if m is NULL the identity vector is assumed */
static inline m4f_t m4f_translate(const m4f_t *m, const v3f_t *v)
{
	m4f_t	identity = m4f_identity();
	m4f_t	translate = m4f_identity();

	if (!m)
		m = &identity;

	translate.m[3][0] = v->x;
	translate.m[3][1] = v->y;
	translate.m[3][2] = v->z;

	return m4f_mult(m, &translate);
}


/* adjust the matrix m to scale by v, returning the resulting matrix */
/* if m is NULL the identity vector is assumed */
static inline m4f_t m4f_scale(const m4f_t *m, const v3f_t *v)
{
	m4f_t	identity = m4f_identity();
	m4f_t	scale = {};

	if (!m)
		m = &identity;

	scale.m[0][0] = v->x;
	scale.m[1][1] = v->y;
	scale.m[2][2] = v->z;
	scale.m[3][3] = 1.f;

	return m4f_mult(m, &scale);
}


/* adjust the matrix m to rotate around the specified axis by radians, returning the resulting matrix */
/* axis is expected to be a unit vector */
/* if m is NULL the identity vector is assumed */
static inline m4f_t m4f_rotate(const m4f_t *m, const v3f_t *axis, float radians)
{
	m4f_t	identity = m4f_identity();
	float	cos_r = cosf(radians);
	float	sin_r = sinf(radians);
	m4f_t	rotate;

	if (!m)
		m = &identity;

	rotate.m[0][0] = cos_r + axis->x * axis->x * (1.f - cos_r);
	rotate.m[0][1] = axis->y * axis->x * (1.f - cos_r) + axis->z * sin_r;
	rotate.m[0][2] = axis->z * axis->x * (1.f - cos_r) - axis->y * sin_r;
	rotate.m[0][3] = 0.f;

	rotate.m[1][0] = axis->x * axis->y * (1.f - cos_r) - axis->z * sin_r;
	rotate.m[1][1] = cos_r + axis->y * axis->y * (1.f - cos_r);
	rotate.m[1][2] = axis->z * axis->y * (1.f - cos_r) + axis->x * sin_r;
	rotate.m[1][3] = 0.f;

	rotate.m[2][0] = axis->x * axis->z * (1.f - cos_r) + axis->y * sin_r;
	rotate.m[2][1] = axis->y * axis->z * (1.f - cos_r) - axis->x * sin_r;
	rotate.m[2][2] = cos_r + axis->z * axis->z * (1.f - cos_r);
	rotate.m[2][3] = 0.f;

	rotate.m[3][0] = 0.f;
	rotate.m[3][1] = 0.f;
	rotate.m[3][2] = 0.f;
	rotate.m[3][3] = 1.f;

	return m4f_mult(m, &rotate);
}


/* this is a simple perpsective projection matrix taken from an opengl tutorial */
static inline m4f_t m4f_frustum(float bot, float top, float left, float right, float nnear, float ffar)
{
	m4f_t	m = {};

	m.m[0][0] = 2 * nnear  / (right - left);

	m.m[1][1] = 2 * nnear / (top - bot);

	m.m[2][0] = (right + left) / (right - left);;
	m.m[2][1] = (top + bot) / (top - bot);
	m.m[2][2] = -(ffar + nnear) / (ffar - nnear);
	m.m[2][3] = -1;

	m.m[3][2] = -2 * ffar * nnear / (ffar - nnear);

	return m;
}

#endif
