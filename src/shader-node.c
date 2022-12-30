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

#include <assert.h>
#include <stdlib.h>

#include <stage.h>

#include "glad.h"
#include "m4f.h"
#include "macros.h"
#include "shader.h"
#include "shader-node.h"
#include "v2f.h"


typedef struct shader_node_t {
	unsigned			*index_ptr;
	unsigned			n_shaders;
	struct {
		shader_t			*shader;
		shader_node_uniforms_func_t	*uniforms_func;
		void				*uniforms_ctxt;
		const m4f_t			*transform;
	}				shaders[];
} shader_node_t;

static unsigned	vbo, tcbo;

static const float	vertices[] = {
	+1.f, +1.f, 0.f,
	+1.f, -1.f, 0.f,
	-1.f, +1.f, 0.f,
	+1.f, -1.f, 0.f,
	-1.f, -1.f, 0.f,
	-1.f, +1.f, 0.f,
};


/* TODO: verify that this is OK, I recall tutorials stating these are always
 * in the range 0-1, but it seems perfectly OK to use -1..+1 which is more
 * convenient here where these shader-textured quads appreciate being fed 
 * unit square coordinates with 0,0 @ the center.
 */
static const float	texcoords[] = {
	1.f, 1.f,
	1.f, -1.f,
	-1.f, 1.f,
	1.f, -1.f,
	-1.f, -1.f,
	-1.f, 1.f,
};


static stage_render_func_ret_t shader_node_render(const stage_t *stage, void *object, float alpha, void *render_ctxt)
{
	shader_node_t	*shader_node = object;
	unsigned	n_uniforms;
	int		*uniforms, *attributes;
	unsigned	idx = 0;

	assert(stage);
	assert(shader_node);

	if (shader_node->index_ptr)
		idx = *(shader_node->index_ptr);

	assert(idx < shader_node->n_shaders);

	shader_use(shader_node->shaders[idx].shader, &n_uniforms, &uniforms, NULL, &attributes);

	if (shader_node->shaders[idx].uniforms_func)
		shader_node->shaders[idx].uniforms_func(shader_node->shaders[idx].uniforms_ctxt, render_ctxt, n_uniforms, uniforms, shader_node->shaders[idx].transform, alpha);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(attributes[0], 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(attributes[0]);

	glBindBuffer(GL_ARRAY_BUFFER, tcbo);
	glVertexAttribPointer(attributes[1], 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(attributes[1]);

	/* XXX: this could be made optional, but since alpha is a constant throughout the stage
	 * integration I'm just always turning it on so stage_set_alpha() always works.  There
	 * are definitely full-screen full-opaque shader node situations where the pointless
	 * performance hit sucks though, especially on older hardware.
	 */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return STAGE_RENDER_FUNC_RET_CONTINUE;
}


static void shader_node_free(const stage_t *stage, void *object)
{
	shader_node_t	*shader_node = object;

	assert(stage);
	assert(shader_node);

	/* XXX FIXME: hmm, maybe the caller should supply a shader_t ** instead */
	for (unsigned i = 0; i < shader_node->n_shaders; i++)
		(void) shader_free(shader_node->shaders[i].shader);
	free(shader_node);
}


static const stage_ops_t shader_node_ops = {
	.render_func = shader_node_render,
	.free_func = shader_node_free,
};


/* return a new shader stage node from a vector of already compiled and linked shader programs.
 * When n_shader_confs is > 1, index_ptr must point somewhere valid for the lifetime of this node,
 * and serves as the index into the vector for selecting which shader_conf to use @ render time.
 * When n_shader_confs is 1, index_ptr may be NULL.
 */
stage_t * shader_node_new_shaderv(const stage_conf_t *conf, unsigned n_shader_confs, const shader_conf_t *shader_confs, unsigned *index_ptr)
{
	shader_node_t	*shader_node;
	stage_t		*stage;

	assert(conf);
	assert(shader_confs);
	assert(n_shader_confs > 0);
	assert(index_ptr || n_shader_confs == 1);

	if (!vbo) {
		/* common to all shader nodes */
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &tcbo);
		glBindBuffer(GL_ARRAY_BUFFER, tcbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	shader_node = calloc(1, sizeof(shader_node_t) + n_shader_confs * sizeof(shader_node->shaders[0]));
	fatal_if(!shader_node, "Unable to allocate shader_node");

	shader_node->index_ptr = index_ptr;
	shader_node->n_shaders = n_shader_confs;
	for (unsigned i = 0; i < n_shader_confs; i++) {
		shader_node->shaders[i].shader = shader_ref(shader_confs[i].shader);
		shader_node->shaders[i].uniforms_func = shader_confs[i].uniforms_func;
		shader_node->shaders[i].uniforms_ctxt = shader_confs[i].uniforms_ctxt;
		shader_node->shaders[i].transform = shader_confs[i].transform;
	}

	stage = stage_new(conf, &shader_node_ops, shader_node);
	fatal_if(!stage, "Unable to create stage \"%s\"", conf->name);

	return stage;
}


/* return a new shader stage node from an already compiled and linked shader program */
stage_t * shader_node_new_shader(const stage_conf_t *conf, const shader_conf_t *shader_conf)
{
	return shader_node_new_shaderv(conf, 1, shader_conf, NULL);
}


/* return a new shader stage node from source vector, see shader_node_new_shaderv() comment */
stage_t * shader_node_new_srcv(const stage_conf_t *conf, unsigned n_shader_src_confs, const shader_src_conf_t *shader_src_confs, unsigned *index_ptr)
{
	shader_conf_t	shader_confs[n_shader_src_confs];
	stage_t		*stage;

	assert(n_shader_src_confs > 0);
	assert(shader_src_confs);

	for (unsigned i = 0; i < n_shader_src_confs; i++) {
		shader_confs[i].shader = shader_pair_new(
						shader_src_confs[i].vs_src,
						shader_src_confs[i].fs_src,
						shader_src_confs[i].n_uniforms,
						shader_src_confs[i].uniforms,
						2,
						(const char *[]) {
							"vertex",
							"texcoord",
						}
					);
		shader_confs[i].transform = shader_src_confs[i].transform;
		shader_confs[i].uniforms_func = shader_src_confs[i].uniforms_func;
		shader_confs[i].uniforms_ctxt = shader_src_confs[i].uniforms_ctxt;
	}

	stage = shader_node_new_shaderv(conf, n_shader_src_confs, shader_confs, index_ptr);
	for (unsigned i = 0; i < n_shader_src_confs; i++)
		shader_free(shader_confs[i].shader);

	return stage;
}


/* return a new shader stage node from source */
stage_t * shader_node_new_src(const stage_conf_t *conf, const shader_src_conf_t *shader_src_conf)
{
	return shader_node_new_srcv(conf, 1, shader_src_conf, NULL);
}
