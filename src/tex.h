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

#ifndef _TEX_H
#define _TEX_H

#include <stdint.h>

typedef struct tex_t tex_t;
typedef struct m4f_t m4f_t;

void tex_render(tex_t *tex, float alpha, m4f_t *model_x);
tex_t * tex_new(int width, int height, const unsigned char *buf);
tex_t * tex_ref(tex_t *tex);
tex_t * tex_free(tex_t *tex);

#endif
