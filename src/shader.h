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

#ifndef _SHADER_H
#define _SHADER_H

typedef struct shader_t shader_t;

unsigned int shader_pair_new_bare(const char *vs_src, const char *fs_src);
shader_t * shader_pair_new(const char *vs_src, const char *fs_src, unsigned n_uniforms, const char **uniforms, unsigned n_attributes, const char **attributes);
void shader_ref(shader_t *shader);
shader_t * shader_free(shader_t *shader);
void shader_use(shader_t *shader, unsigned *res_n_uniforms, int **res_uniforms, unsigned *res_n_attributes, int **res_attributes);

#endif
