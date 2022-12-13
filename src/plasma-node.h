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

#ifndef _PLASMA_NODE_H
#define _PLASMA_NODE_H

typedef struct m4f_t m4f_t;
typedef struct stage_t stage_t;
typedef struct stage_conf_t stage_conf_t;

stage_t * plasma_node_new(const stage_conf_t *conf, m4f_t *projection_x, float *gloom, int *maga);

#endif
