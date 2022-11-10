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

#ifndef _BONUS_NODE_H
#define _BONUS_NODE_H

#define BONUS_NODE_RELEASE_MS	100	/* FIXME: this needs to be actual time units, right now it's frames */

typedef struct stage_conf_t stage_conf_t;
typedef struct m4f_t m4f_t;
typedef struct v2f_t v2f_t;

stage_t * bonus_node_new(stage_conf_t *conf, unsigned value, m4f_t *projection_x, v2f_t *position, float scale, unsigned **release, v2f_t **release_position);

#endif

