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

#include <stage.h>

#include "tv-node.h"
#include "gfx/gfx-tv.h"
#include "tex.h"
#include "tex-node.h"

static tex_t	*tv_tex;

stage_t * tv_node_new(stage_conf_t *conf, m4f_t *model_x)
{
	if (!tv_tex)
		tv_tex = tex_new(gfx_tv.width, gfx_tv.height, gfx_tv.pixel_data);

	return tex_node_new_tex(conf, tv_tex, model_x);
}
