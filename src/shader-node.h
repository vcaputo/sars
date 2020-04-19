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

#ifndef _SHADER_NODE_H
#define _SHADER_NODE_H

typedef struct shader_t shader_t;
typedef struct stage_t stage_t;
typedef struct stage_conf_t stage_conf_t;
typedef struct m4f_t m4f_t;

typedef void (shader_node_uniforms_func_t)(void *uniforms_ctxt, void *render_ctxt, unsigned n_uniforms, const int *uniforms, const m4f_t *transform, float alpha);

stage_t * shader_node_new_shader(const stage_conf_t *conf, shader_t *shader, const m4f_t *transform, shader_node_uniforms_func_t *uniforms_func, void *uniforms_ctxt);
stage_t * shader_node_new_src(const stage_conf_t *conf, const char *vs_src, const char *fs_src, const m4f_t *transform, shader_node_uniforms_func_t *uniforms_func, void *uniforms_ctxt, unsigned n_uniforms, const char **uniforms);

#endif
