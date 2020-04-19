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

#include "glad.h"
#include "macros.h"
#include "shader.h"


typedef struct shader_t {
	unsigned	program, refcnt;
	unsigned	n_uniforms, n_attributes;
	int		*uniforms, *attributes;

	int		locations[];
} shader_t;


unsigned int shader_pair_new_bare(const char *vs_src, const char *fs_src)
{
	unsigned int	vertex_shader, fragment_shader, shader;
	int		shader_success;
	char		shader_info[4096];

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vs_src, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_success);
	if (!shader_success) {
		glGetShaderInfoLog(vertex_shader, sizeof(shader_info), NULL, shader_info);
		fatal_if(1, "Error compiling vertex shader: \"%s\"", shader_info);
	}

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fs_src, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_success);
	if (!shader_success) {
		glGetShaderInfoLog(fragment_shader, sizeof(shader_info), NULL, shader_info);
		fatal_if(1, "Error compiling fragment shader: \"%s\"", shader_info);
	}

	shader = glCreateProgram();
	glAttachShader(shader, vertex_shader);
	glAttachShader(shader, fragment_shader);
	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &shader_success);
	if (!shader_success) {
		glGetProgramInfoLog(shader, sizeof(shader_info), NULL, shader_info);
		fatal_if(1, "Error linking shader program: \"%s\"", shader_info);
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return shader;
}


shader_t * shader_pair_new(const char *vs_src, const char *fs_src, unsigned n_uniforms, const char **uniforms, unsigned n_attributes, const char **attributes)
{
	shader_t	*shader;

	assert(vs_src);
	assert(fs_src);
	assert(uniforms || !n_uniforms);
	assert(attributes || !n_attributes);

	shader = calloc(1, sizeof(shader_t) + (n_uniforms + n_attributes) * sizeof(int));
	fatal_if(!shader, "Unable to allocate shader");

	shader->program = shader_pair_new_bare(vs_src, fs_src);
	shader->refcnt++;
	shader->n_uniforms = n_uniforms;
	shader->n_attributes = n_attributes;
	shader->uniforms = shader->locations;
	shader->attributes = &shader->locations[n_uniforms];

	for (unsigned i = 0; i < n_uniforms; i++)
		shader->uniforms[i] = glGetUniformLocation(shader->program, uniforms[i]);

	for (unsigned i = 0; i < n_attributes; i++)
		shader->attributes[i] = glGetAttribLocation(shader->program, attributes[i]);

	return shader;
}


void shader_ref(shader_t *shader)
{
	assert(shader);

	shader->refcnt++;
}


shader_t * shader_free(shader_t *shader)
{
	assert(shader);

	shader->refcnt--;
	if (shader->refcnt > 0)
		return shader;

	glDeleteProgram(shader->program);
	free(shader);

	return NULL;
}


void shader_use(shader_t *shader, unsigned *res_n_uniforms, int **res_uniforms, unsigned *res_n_attributes, int **res_attributes)
{
	assert(shader);

	if (res_n_uniforms)
		*res_n_uniforms = shader->n_uniforms;

	if (res_uniforms)
		*res_uniforms = shader->uniforms;

	if (res_n_attributes)
		*res_n_attributes = shader->n_attributes;

	if (res_attributes)
		*res_attributes = shader->attributes;

	glUseProgram(shader->program);
}
