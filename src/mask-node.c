/*
 *  Copyright (C) 2022 - Vito Caputo - <vcaputo@pengaru.com>
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

#include "ansr-tex.h"
#include "mask-node.h"
#include "tex.h"
#include "tex-node.h"

static tex_t	*mask_tex;

stage_t * mask_node_new(stage_conf_t *conf, m4f_t *projection_x, m4f_t *model_x)
{
	if (!mask_tex)
		mask_tex = ansr_tex_new("assets/mask.ans", "assets/mask.mask.ans");

	return tex_node_new_tex(conf, mask_tex, projection_x, model_x);
}
