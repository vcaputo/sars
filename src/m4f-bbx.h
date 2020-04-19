/*
 *  Copyright (C) 2020 - Vito Caputo - <vcaputo@pengaru.com>
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

#ifndef _M4F_BBX_H
#define _M4F_BBX_H

#include <float.h>

#include "bb2f.h"
#include "bb3f.h"
#include "m4f.h"
#include "macros.h"


/* multiply a matrix and bb3f */
static bb3f_t * m4f_mult_bb3f(const m4f_t *transform, const bb3f_t *aabb, bb3f_t *res_aabb)
{
	v3f_t	box[8] = {
		{ aabb->min.x, aabb->min.y, aabb->min.z },
		{ aabb->min.x, aabb->min.y, aabb->max.z },
		{ aabb->min.x, aabb->max.y, aabb->max.z },
		{ aabb->min.x, aabb->max.y, aabb->min.z },
		{ aabb->max.x, aabb->min.y, aabb->min.z },
		{ aabb->max.x, aabb->min.y, aabb->max.z },
		{ aabb->max.x, aabb->max.y, aabb->max.z },
		{ aabb->max.x, aabb->max.y, aabb->min.z },
	};

	bb3f_t	_aabb = {
		.min = { FLT_MAX, FLT_MAX, FLT_MAX },
		.max = { -FLT_MAX, -FLT_MAX, -FLT_MAX},
	};

	for (int i = 0; i < NELEMS(box); i++) {
		v3f_t	X = m4f_mult_v3f(transform, &box[i]);

		if (_aabb.min.x > X.x)
			_aabb.min.x = X.x;
		if (_aabb.max.x < X.x)
			_aabb.max.x = X.x;

		if (_aabb.min.y > X.y)
			_aabb.min.y = X.y;
		if (_aabb.max.y < X.y)
			_aabb.max.y = X.y;

		if (_aabb.min.z > X.z)
			_aabb.min.z = X.z;
		if (_aabb.max.z < X.z)
			_aabb.max.z = X.z;
	}

	*res_aabb = _aabb;

	return res_aabb;
}


/* multiply a matrix and a bb3f but return only the x,y components in a bb2f */
/* Caller must supply the storage in res_aabb, for convenience this is also returned. */
static bb2f_t * m4f_mult_bb3f_bb2f(const m4f_t *transform, const bb3f_t *aabb, bb2f_t *res_aabb)
{
	bb3f_t	_aabb;

	m4f_mult_bb3f(transform, aabb, &_aabb);

	res_aabb->min.x = _aabb.min.x;
	res_aabb->min.y = _aabb.min.y;
	res_aabb->max.x = _aabb.max.x;
	res_aabb->max.y = _aabb.max.y;

	return res_aabb;
}


#endif
