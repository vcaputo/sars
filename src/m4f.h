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

#ifndef _M4F_H
#define _M4F_H

#include <assert.h>

#include "v4f.h"
#include "v3f.h"

typedef struct m4f_t {
	float	m[4][4];
} m4f_t;

/* XXX: note this is column-major, which reflects OpenGL expectations. */


/* returns an identity matrix */
static inline m4f_t m4f_identity(void)
{
	return (m4f_t){ .m = {
		{ 1.f, 0.f, 0.f, 0.f },
		{ 0.f, 1.f, 0.f, 0.f },
		{ 0.f, 0.f, 1.f, 0.f },
		{ 0.f, 0.f, 0.f, 1.f },
	}};
}


/* 4x4 X 4x4 matrix multiply */
static inline m4f_t m4f_mult(const m4f_t *a, const m4f_t *b)
{
	m4f_t	r;

	/* In matrix multiplication the output has the number of rows from a and number
	 * of columns from b, and a must have the same number of columns as b has rows.
	 *
	 * So this covers a very specific form of matrix multiplication that is of limitd
	 * use.  It's desirable to be able to have inputs of m4f_t and v4f_t with a v4f_t
	 * output.  I think that will be moved into a combined matrix+vector header
	 * that will depend on m4f.h and v4f.h.
	 */
	
	r.m[0][0] = (a->m[0][0] * b->m[0][0]) + (a->m[1][0] * b->m[0][1]) + (a->m[2][0] * b->m[0][2]) + (a->m[3][0] * b->m[0][3]);
	r.m[0][1] = (a->m[0][1] * b->m[0][0]) + (a->m[1][1] * b->m[0][1]) + (a->m[2][1] * b->m[0][2]) + (a->m[3][1] * b->m[0][3]);
	r.m[0][2] = (a->m[0][2] * b->m[0][0]) + (a->m[1][2] * b->m[0][1]) + (a->m[2][2] * b->m[0][2]) + (a->m[3][2] * b->m[0][3]);
	r.m[0][3] = (a->m[0][3] * b->m[0][0]) + (a->m[1][3] * b->m[0][1]) + (a->m[2][3] * b->m[0][2]) + (a->m[3][3] * b->m[0][3]);

	r.m[1][0] = (a->m[0][0] * b->m[1][0]) + (a->m[1][0] * b->m[1][1]) + (a->m[2][0] * b->m[1][2]) + (a->m[3][0] * b->m[1][3]);
	r.m[1][1] = (a->m[0][1] * b->m[1][0]) + (a->m[1][1] * b->m[1][1]) + (a->m[2][1] * b->m[1][2]) + (a->m[3][1] * b->m[1][3]);
	r.m[1][2] = (a->m[0][2] * b->m[1][0]) + (a->m[1][2] * b->m[1][1]) + (a->m[2][2] * b->m[1][2]) + (a->m[3][2] * b->m[1][3]);
	r.m[1][3] = (a->m[0][3] * b->m[1][0]) + (a->m[1][3] * b->m[1][1]) + (a->m[2][3] * b->m[1][2]) + (a->m[3][3] * b->m[1][3]);

	r.m[2][0] = (a->m[0][0] * b->m[2][0]) + (a->m[1][0] * b->m[2][1]) + (a->m[2][0] * b->m[2][2]) + (a->m[3][0] * b->m[2][3]);
	r.m[2][1] = (a->m[0][1] * b->m[2][0]) + (a->m[1][1] * b->m[2][1]) + (a->m[2][1] * b->m[2][2]) + (a->m[3][1] * b->m[2][3]);
	r.m[2][2] = (a->m[0][2] * b->m[2][0]) + (a->m[1][2] * b->m[2][1]) + (a->m[2][2] * b->m[2][2]) + (a->m[3][2] * b->m[2][3]);
	r.m[2][3] = (a->m[0][3] * b->m[2][0]) + (a->m[1][3] * b->m[2][1]) + (a->m[2][3] * b->m[2][2]) + (a->m[3][3] * b->m[2][3]);

	r.m[3][0] = (a->m[0][0] * b->m[3][0]) + (a->m[1][0] * b->m[3][1]) + (a->m[2][0] * b->m[3][2]) + (a->m[3][0] * b->m[3][3]);
	r.m[3][1] = (a->m[0][1] * b->m[3][0]) + (a->m[1][1] * b->m[3][1]) + (a->m[2][1] * b->m[3][2]) + (a->m[3][1] * b->m[3][3]);
	r.m[3][2] = (a->m[0][2] * b->m[3][0]) + (a->m[1][2] * b->m[3][1]) + (a->m[2][2] * b->m[3][2]) + (a->m[3][2] * b->m[3][3]);
	r.m[3][3] = (a->m[0][3] * b->m[3][0]) + (a->m[1][3] * b->m[3][1]) + (a->m[2][3] * b->m[3][2]) + (a->m[3][3] * b->m[3][3]);

	return r;
}


/* 4x4 X 1x4 matrix multiply */
static inline v4f_t m4f_mult_v4f(const m4f_t *a, const v4f_t *b)
{
	v4f_t	v;

	v.x = (a->m[0][0] * b->x) + (a->m[1][0] * b->y) + (a->m[2][0] * b->z) + (a->m[3][0] * b->w);
	v.y = (a->m[0][1] * b->x) + (a->m[1][1] * b->y) + (a->m[2][1] * b->z) + (a->m[3][1] * b->w);
	v.z = (a->m[0][2] * b->x) + (a->m[1][2] * b->y) + (a->m[2][2] * b->z) + (a->m[3][2] * b->w);
	v.w = (a->m[0][3] * b->x) + (a->m[1][3] * b->y) + (a->m[2][3] * b->z) + (a->m[3][3] * b->w);

	return v;
}


/* 4x4 X 1x3 matrix multiply */
static inline v3f_t m4f_mult_v3f(const m4f_t *a, const v3f_t *b)
{
	v3f_t	v;

	/* TODO: verify correctness/sanity of this */

	v.x = (a->m[0][0] * b->x) + (a->m[1][0] * b->y) + (a->m[2][0] * b->z) + (a->m[3][0]);
	v.y = (a->m[0][1] * b->x) + (a->m[1][1] * b->y) + (a->m[2][1] * b->z) + (a->m[3][1]);
	v.z = (a->m[0][2] * b->x) + (a->m[1][2] * b->y) + (a->m[2][2] * b->z) + (a->m[3][2]);

	return v;
}


/* 4x4 square matrix inversion */
static inline m4f_t m4f_invert(const m4f_t *m)
{
	m4f_t	inv;
	float	det;

	inv.m[0][0] = m->m[1][1] * m->m[2][2] * m->m[3][3] -
		      m->m[1][1] * m->m[2][3] * m->m[3][2] -
		      m->m[2][1] * m->m[1][2] * m->m[3][3] +
		      m->m[2][1] * m->m[1][3] * m->m[3][2] +
		      m->m[3][1] * m->m[1][2] * m->m[2][3] -
		      m->m[3][1] * m->m[1][3] * m->m[2][2];

	inv.m[1][0] = -m->m[1][0] * m->m[2][2] * m->m[3][3] +
		       m->m[1][0] * m->m[2][3] * m->m[3][2] +
		       m->m[2][0] * m->m[1][2] * m->m[3][3] -
		       m->m[2][0] * m->m[1][3] * m->m[3][2] -
		       m->m[3][0] * m->m[1][2] * m->m[2][3] +
		       m->m[3][0] * m->m[1][3] * m->m[2][2];

	inv.m[2][0] = m->m[1][0] * m->m[2][1] * m->m[3][3] -
		      m->m[1][0] * m->m[8][3] * m->m[3][1] -
		      m->m[2][0] * m->m[1][1] * m->m[3][3] +
		      m->m[2][0] * m->m[1][3] * m->m[3][1] +
		      m->m[3][0] * m->m[1][1] * m->m[2][3] -
		      m->m[3][0] * m->m[1][3] * m->m[2][1];

	inv.m[3][0] = -m->m[1][0] * m->m[2][1] * m->m[3][2] +
		       m->m[1][0] * m->m[2][2] * m->m[3][1] +
		       m->m[2][0] * m->m[1][1] * m->m[3][2] -
		       m->m[2][0] * m->m[1][2] * m->m[3][1] -
		       m->m[3][0] * m->m[1][1] * m->m[2][2] +
		       m->m[3][0] * m->m[1][2] * m->m[2][1];

	inv.m[0][1] = -m->m[0][1] * m->m[2][2] * m->m[3][3] +
		       m->m[0][1] * m->m[2][3] * m->m[3][2] +
		       m->m[2][1] * m->m[0][2] * m->m[3][3] -
		       m->m[2][1] * m->m[0][3] * m->m[3][2] -
		       m->m[3][1] * m->m[0][2] * m->m[2][3] +
		       m->m[3][1] * m->m[0][3] * m->m[2][2];

	inv.m[1][1] = m->m[0][0] * m->m[2][2] * m->m[3][3] -
		      m->m[0][0] * m->m[2][3] * m->m[3][2] -
		      m->m[2][0] * m->m[0][2] * m->m[3][3] +
		      m->m[2][0] * m->m[0][3] * m->m[3][2] +
		      m->m[3][0] * m->m[0][2] * m->m[2][3] -
		      m->m[3][0] * m->m[0][3] * m->m[2][2];

	inv.m[2][1] = -m->m[0][0] * m->m[2][1] * m->m[3][3] +
		       m->m[0][0] * m->m[2][3] * m->m[3][1] +
		       m->m[2][0] * m->m[0][1] * m->m[3][3] -
		       m->m[2][0] * m->m[0][3] * m->m[3][1] -
		       m->m[3][0] * m->m[0][1] * m->m[2][3] +
		       m->m[3][0] * m->m[0][3] * m->m[2][1];

	inv.m[3][1] = m->m[0][0] * m->m[2][1] * m->m[3][2] -
		      m->m[0][0] * m->m[2][2] * m->m[3][1] -
		      m->m[2][0] * m->m[0][1] * m->m[3][2] +
		      m->m[2][0] * m->m[0][2] * m->m[3][1] +
		      m->m[3][0] * m->m[0][1] * m->m[2][2] -
		      m->m[3][0] * m->m[0][2] * m->m[2][1];

	inv.m[0][2] = m->m[0][1] * m->m[1][2] * m->m[3][3] -
		      m->m[0][1] * m->m[1][3] * m->m[3][2] -
		      m->m[1][1] * m->m[0][2] * m->m[3][3] +
		      m->m[1][1] * m->m[0][3] * m->m[3][2] +
		      m->m[2][1] * m->m[0][2] * m->m[1][3] -
		      m->m[2][1] * m->m[0][3] * m->m[1][2];

	inv.m[1][2] = -m->m[0][0] * m->m[1][2] * m->m[3][3] +
		       m->m[0][0] * m->m[1][3] * m->m[3][2] +
		       m->m[1][0] * m->m[0][2] * m->m[3][3] -
		       m->m[1][0] * m->m[0][3] * m->m[3][2] -
		       m->m[3][0] * m->m[0][2] * m->m[1][3] +
		       m->m[3][0] * m->m[0][3] * m->m[1][2];

	inv.m[2][2] = m->m[0][0] * m->m[1][1] * m->m[3][3] -
		      m->m[0][0] * m->m[1][3] * m->m[3][1] -
		      m->m[1][0] * m->m[0][1] * m->m[3][3] +
		      m->m[1][0] * m->m[0][3] * m->m[3][1] +
		      m->m[3][0] * m->m[0][1] * m->m[1][3] -
		      m->m[3][0] * m->m[0][3] * m->m[1][1];

	inv.m[3][2] = -m->m[0][0] * m->m[1][1] * m->m[3][2] +
		       m->m[0][0] * m->m[1][2] * m->m[3][1] +
		       m->m[1][0] * m->m[0][1] * m->m[3][2] -
		       m->m[1][0] * m->m[0][2] * m->m[3][1] -
		       m->m[3][0] * m->m[0][1] * m->m[1][2] +
		       m->m[3][0] * m->m[0][2] * m->m[1][1];

	inv.m[0][3] = -m->m[0][1] * m->m[1][2] * m->m[2][3] +
		       m->m[0][1] * m->m[1][3] * m->m[2][2] +
		       m->m[1][1] * m->m[0][2] * m->m[2][3] -
		       m->m[1][1] * m->m[0][3] * m->m[2][2] -
		       m->m[2][1] * m->m[0][2] * m->m[1][3] +
		       m->m[2][1] * m->m[0][3] * m->m[1][2];

	inv.m[1][3] = m->m[0][0] * m->m[1][2] * m->m[2][3] -
		      m->m[0][0] * m->m[1][3] * m->m[2][2] -
		      m->m[1][0] * m->m[0][2] * m->m[2][3] +
		      m->m[1][0] * m->m[0][3] * m->m[2][2] +
		      m->m[2][0] * m->m[0][2] * m->m[1][3] -
		      m->m[2][0] * m->m[0][3] * m->m[1][2];

	inv.m[2][3] = -m->m[0][0] * m->m[1][1] * m->m[2][3] +
		       m->m[0][0] * m->m[1][3] * m->m[2][1] +
		       m->m[1][0] * m->m[0][1] * m->m[2][3] -
		       m->m[1][0] * m->m[0][3] * m->m[2][1] -
		       m->m[2][0] * m->m[0][1] * m->m[1][3] +
		       m->m[2][0] * m->m[0][3] * m->m[1][1];

	inv.m[3][3] = m->m[0][0] * m->m[1][1] * m->m[2][2] -
		      m->m[0][0] * m->m[1][2] * m->m[2][1] -
		      m->m[1][0] * m->m[0][1] * m->m[2][2] +
		      m->m[1][0] * m->m[0][2] * m->m[2][1] +
		      m->m[2][0] * m->m[0][1] * m->m[1][2] -
		      m->m[2][0] * m->m[0][2] * m->m[1][1];

	det = m->m[0][0] * inv.m[0][0] + m->m[0][1] * inv.m[1][0] + m->m[0][2] * inv.m[2][0] + m->m[0][3] * inv.m[3][0];

	/* XXX: this could instead return a success/fail status, but in games it's not
	 * like anything useful will be possible when it's non-invertible, hence the assert
	 */
	assert(det != 0.f);

	det = 1.f / det;

	inv.m[0][0] *= det;
	inv.m[0][1] *= det;
	inv.m[0][2] *= det;
	inv.m[0][3] *= det;
	inv.m[1][0] *= det;
	inv.m[1][1] *= det;
	inv.m[1][2] *= det;
	inv.m[1][3] *= det;
	inv.m[2][0] *= det;
	inv.m[2][1] *= det;
	inv.m[2][2] *= det;
	inv.m[2][3] *= det;
	inv.m[3][0] *= det;
	inv.m[3][1] *= det;
	inv.m[3][2] *= det;
	inv.m[3][3] *= det;

	return inv;
}

#endif
