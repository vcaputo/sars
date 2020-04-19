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

#include <SDL.h>

#include <play.h>
#include <stage.h>

#include "glad.h"
#include "plasma-node.h"
#include "shader-node.h"


static const char	*plasma_vs = ""
	"#version 120\n"

	"attribute vec3 vertex;"
	"attribute vec2 texcoord;"

	"void main()"
	"{"
	"	gl_TexCoord[0].xy = texcoord;"
	//"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"	gl_Position = vec4(vertex, 1.f);"
	"}"
"";


// derived from https://www.bidouille.org/prog/plasma
static const char	*plasma_fs = ""
	"#version 120\n"

	"#define PI 3.1415926535897932384626433832795\n"

	"uniform float alpha;"
	"uniform float time;"

	"void main() {"
	"	float v;"
	"	float stime = sin(time * .01f) * 100.f;"

	"	vec2 c = gl_TexCoord[0].st;"

	// this zooms the texture coords in and out a bit with time
	"	c *= (sin(stime * .01f) *.5f + .5f) * 3.f + 1.f;"

	// plasma calculations, stime instead of time directly to vary directions and speed
	"	v = sin((c.x + stime));"
	"	v += sin((c.y + stime) * .5f);"
	"	v += sin((c.x + c.y +stime) * .5f);"

	"	c += vec2(sin(stime * .33f), cos(stime * .5f)) * 3.f;"

	"	v += sin(sqrt(c.x * c.x + c.y * c.y + 1.f) + stime);"

	"	vec3 col = vec3(cos(PI * v + sin(time)), sin(PI * v + cos(time * .33f)), cos(PI * v + sin(time * .66f)));"
	"	gl_FragColor = vec4(col * .5f + .5f, alpha);"
	"}"
"";


static void plasma_uniforms(void *uniforms_ctxt, void *render_ctxt, unsigned n_uniforms, const int *uniforms, const m4f_t *model_x, float alpha)
{
	play_t	*play = render_ctxt;

	glUniform1f(uniforms[0], alpha);
	glUniform1f(uniforms[1], play_ticks(play, PLAY_TICKS_TIMER0) * .001f); // FIXME KLUDGE ALERT
}


/* create plasma rendering stage */
stage_t * plasma_node_new(const stage_conf_t *conf)
{
	return	shader_node_new_src(conf, plasma_vs, plasma_fs, NULL, plasma_uniforms, NULL, 2,
			(const char *[]){
				"alpha",
				"time",
			}
		);
}
