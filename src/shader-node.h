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

typedef struct shader_conf_t {
	shader_t			*shader;
	const m4f_t			*transform;
	shader_node_uniforms_func_t	*uniforms_func;
	void				*uniforms_ctxt;
} shader_conf_t;

typedef struct shader_src_conf_t {
	const char			*vs_src, *fs_src;
	const m4f_t			*transform;
	shader_node_uniforms_func_t	*uniforms_func;
	void				*uniforms_ctxt;
	unsigned			n_uniforms;
	const char			**uniforms;
} shader_src_conf_t;

stage_t * shader_node_new_shaderv(const stage_conf_t *conf, unsigned n_shader_confs, const shader_conf_t *shader_confs, unsigned *index_ptr);
stage_t * shader_node_new_shader(const stage_conf_t *conf, const shader_conf_t *shader_conf);

stage_t * shader_node_new_srcv(const stage_conf_t *conf, unsigned n_shader_src_confs, const shader_src_conf_t *shader_src_conf, unsigned *index_ptr);
stage_t * shader_node_new_src(const stage_conf_t *conf, const shader_src_conf_t *shader_src_conf);

#endif
