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

#include <SDL.h>
#include <assert.h>

#include <ix2.h>
#include <pad.h>
#include <play.h>
#include <stage.h>

#include "adult-node.h"
#include "adult-masked-node.h"
#include "baby-hatted-node.h"
#include "baby-node.h"
#include "bb2f.h"
#include "bb3f.h"
#include "bonus-node.h"
#include "digit-node.h"
#include "glad.h"
#include "m4f.h"
#include "m4f-3dx.h"
#include "m4f-bbx.h"
#include "macros.h"
#include "mask-node.h"
#include "plasma-node.h"
#include "sars.h"
#include "sfx.h"
#include "teepee-node.h"
#include "tv-node.h"
#include "v2f.h"
#include "virus-node.h"

#define	GAME_NUM_VIRUSES	8
#define GAME_NUM_BABIES		10

#define GAME_VIRUS_SPEED	.01f
#define GAME_ADULT_SPEED	.04f
#define GAME_MASK_SPEED		.02f

#define GAME_ENTITIES_DELAY_MS	20
#define GAME_ENTITIES_TIMER	PLAY_TICKS_TIMER1

#define GAME_NEWBABIES_DELAY_MS	500
#define GAME_NEWBABIES_TIMER	PLAY_TICKS_TIMER6

#define GAME_MASK_PROTECTION	3

#define GAME_TV_DELAY_MS	3000
#define GAME_TV_TIMER		PLAY_TICKS_TIMER3
#define GAME_TV_RANGE_MIN	.2f
#define GAME_TV_RANGE_MAX	.7f
#define GAME_TV_ATTRACTION	.005f

#define GAME_TP_WIN_THRESHOLD	256

#define GAME_KBD_DELAY_MS	20
#define GAME_KBD_TIMER		PLAY_TICKS_TIMER4

#define GAME_OVER_DELAY_MS	1000
#define GAME_OVER_TIMER		PLAY_TICKS_TIMER2

#define GAME_FLASHERS_DELAY_MS	75
#define GAME_FLASHERS_TIMER	PLAY_TICKS_TIMER5

#define GAME_ADULT_SCALE	(v3f_t){ .07f, .07f, .07f }
#define GAME_BABY_SCALE		(v3f_t){ .05f, .05f, .05f }
#define GAME_MASK_SCALE		(v3f_t){ .07f, .07f, .07f }
#define GAME_TEEPEE_SCALE	(v3f_t){ .07f, .07f, .07f }
#define GAME_TEEPEE_ICON_SCALE	(v3f_t){ .06f, .06f, .06f }
#define GAME_TV_SCALE		(v3f_t){ .15f, .15f, .15f }
#define GAME_VIRUS_SCALE	(v3f_t){ .05f, .05f, .05f }
#define GAME_DIGITS_SCALE	(v3f_t){ .05f, .05f, .05f }

/* every entity just starts with a unit cube AABB and is transformed with a matrix into its position,
 * so here's a convenient aabb to feed into those transformations as needed.
 */
static const bb3f_t	any_aabb = { .min = { -1.f, -1.f, -1.f }, .max = { 1.f, 1.f, 1.f } };

typedef enum game_state_t {
	GAME_STATE_PLAYING,
	GAME_STATE_OVER,
	GAME_STATE_OVER_DELAY,
	GAME_STATE_OVER_WAITING,
	GAME_STATE_OVER_WINNING,
	GAME_STATE_OVER_WINNING_DELAY,
	GAME_STATE_OVER_WINNING_WAITING,
} game_state_t;

typedef enum entity_type_t {
	ENTITY_TYPE_BABY,
	ENTITY_TYPE_ADULT,
	ENTITY_TYPE_VIRUS,
	ENTITY_TYPE_TV,
	ENTITY_TYPE_MASK,
	ENTITY_TYPE_TEEPEE,
	ENTITY_TYPE_TEEPEE_ICON,
} entity_type_t;

typedef union entity_t entity_t;
typedef struct entity_any_t entity_any_t;

struct entity_any_t {
	entity_type_t	type;
	stage_t		*node;
	ix2_object_t	*ix2_object;
	v2f_t		position;
	v3f_t		scale;
	m4f_t		model_x;
	bb2f_t		aabb_x;
	unsigned	flashing:1;
	entity_any_t	*flashers_next;
	unsigned	flashes_remaining;
};

typedef struct mask_t {
	entity_any_t	entity;
} mask_t;

typedef struct baby_t baby_t;
struct baby_t {
	entity_any_t	entity;
	baby_t		*rescues_next;
};

typedef struct virus_t {
	entity_any_t	entity;
	entity_t	*new_infections_next;
} virus_t;

typedef struct adult_t {
	entity_any_t	entity;
	unsigned	rescues;
	unsigned	captivated:1;
	unsigned	masked;
	entity_t	*holding;
} adult_t;

typedef struct teepee_t {
	entity_any_t	entity;
	unsigned	quantity;
	unsigned	*bonus_release;
	v2f_t		*bonus_release_position;
} teepee_t;

typedef struct teepee_icon_t {
	entity_any_t		entity;
	struct teepee_icon_t	*next;
} teepee_icon_t;

typedef struct tv_t {
	entity_any_t	entity;
} tv_t;

union entity_t {
	entity_any_t	any;
	adult_t		adult;
	baby_t		baby;
	mask_t		mask;
	teepee_t	teepee;
	tv_t		tv;
	virus_t		virus;
};

typedef struct game_t {
	game_state_t	state;

	struct {
		SDL_TouchID	touch_id;
		SDL_FingerID	finger_id;
		v2f_t		position; /* -1 .. +1 */
		int		active;
	} touch;

	sars_t		*sars;
	stage_t		*stage;
	stage_t		*game_node;
	stage_t		*babies_node;
	stage_t		*viruses_node;
	stage_t		*plasma_node;
	stage_t		*score_node;
	ix2_t		*ix2;
	pad_t		*pad;

	/* count of hoarded teepee and list of representative icons for animating @ win */
	unsigned	teepee_cnt;
	teepee_icon_t	*teepee_head;
	entity_any_t	*flashers_on_head, *flashers_off_head;
	baby_t		*rescues_head;
	unsigned	babies_cnt;

	adult_t		*adult;
	tv_t		*tv;
	mask_t		*mask;
	teepee_t	*teepee;
	entity_t	*new_infections;
	virus_t		*viruses[GAME_NUM_VIRUSES];
	m4f_t		score_digits_x[10];
} game_t;


static inline float randf(void)
{
	return 2.f / (float)RAND_MAX * rand() - 1.f;
}


/* update the entity's transformation and position in the index */
static void entity_update_x(game_t *game, entity_any_t *entity)
{
	/* icon entities aren't intended to get indexed spatially, so we don't really initialize them
	 * fully for that purpose.  Right now it's just the teepee icon, but assert it never manages to
	 * get passed here.  TODO: it probably makes sense to break icon tentities out to a separate
	 * non-entity type that doesn't even have ix2 related members.
	 */
	assert(entity->type != ENTITY_TYPE_TEEPEE_ICON);

	entity->model_x = m4f_translate(NULL, &(v3f_t){ entity->position.x, entity->position.y, 0.f });
	entity->model_x = m4f_scale(&entity->model_x, &entity->scale);

	/* apply the entities transform to any_aabb to get the current transformed aabb, cache it in the
	 * entity in case a search needs to be done... */
	m4f_mult_bb3f_bb2f(&entity->model_x, &any_aabb, &entity->aabb_x);

	if (!(entity->ix2_object)) {
		entity->ix2_object = ix2_object_new(game->ix2, NULL, NULL, &entity->aabb_x, entity);
	} else {
		entity->ix2_object = ix2_object_move(game->ix2, entity->ix2_object, NULL, NULL, &entity->aabb_x);
	}

	fatal_if(!entity->ix2_object, "Unable to update ix2 object");
}


/* this is unnecessary copy and paste junk, but I'm really falling asleep here
 * and need to get shit working before I pass out.
 */

static adult_t * adult_new(game_t *game, stage_t *parent)
{
	adult_t	*adult;

	adult = pad_get(game->pad, sizeof(entity_t));
	fatal_if(!adult, "unable to allocate adult_t");

	adult->entity.type = ENTITY_TYPE_ADULT;
	adult->entity.node = adult_node_new(&(stage_conf_t){ .parent = parent, .name = "adult", .layer = 3, .alpha = 1.f }, &game->sars->projection_x, &adult->entity.model_x);
	adult->entity.scale = GAME_ADULT_SCALE;
	entity_update_x(game, &adult->entity);

	return adult;
}


static baby_t * baby_new(game_t *game, stage_t *parent, baby_t *rescue)
{
	baby_t	*baby = rescue;

	if (!baby) {
		baby = pad_get(game->pad, sizeof(entity_t));
		fatal_if(!baby, "unable to allocate baby_t");
		baby->entity.type = ENTITY_TYPE_BABY;
		baby->entity.node = baby_node_new(&(stage_conf_t){ .parent = parent, .name = "baby", .active = 1, .alpha = 1.f }, &game->sars->projection_x, &baby->entity.model_x);
		baby->entity.scale = GAME_BABY_SCALE;
	} else
		stage_set_active(baby->entity.node, 1);

	baby->entity.position.x = randf();
	baby->entity.position.y = randf();
	entity_update_x(game, &baby->entity);

	return baby;
}


static mask_t * mask_new(game_t *game, stage_t *parent)
{
	mask_t	*mask;

	mask = pad_get(game->pad, sizeof(entity_t));
	fatal_if(!mask, "unable to allocate mask_t");

	mask->entity.type = ENTITY_TYPE_MASK;
	mask->entity.node = mask_node_new(&(stage_conf_t){ .parent = parent, .name = "mask", .layer = 4, .alpha = 1.f }, &game->sars->projection_x, &mask->entity.model_x);
	mask->entity.scale = GAME_MASK_SCALE;

	return mask;
}


static teepee_t * teepee_new(game_t *game, stage_t *parent)
{
	teepee_t	*teepee;

	teepee = pad_get(game->pad, sizeof(entity_t));
	fatal_if(!teepee, "unable to allocate teepee_t");

	teepee->entity.type = ENTITY_TYPE_TEEPEE;
	teepee->entity.node = teepee_node_new(&(stage_conf_t){ .parent = parent, .name = "teepee", .layer = 4, .alpha = 1.f }, &game->sars->projection_x, &teepee->entity.model_x);
	teepee->entity.scale = GAME_TEEPEE_SCALE;

	return teepee;
}


static tv_t * tv_new(game_t *game, stage_t *parent)
{
	tv_t	*tv;

	tv = pad_get(game->pad, sizeof(entity_t));
	fatal_if(!tv, "unable to allocate tv_t");

	tv->entity.type = ENTITY_TYPE_TV;
	tv->entity.node = tv_node_new(&(stage_conf_t){ .parent = parent, .name = "tv", .layer = 1, .alpha = 1.f }, &game->sars->projection_x, &tv->entity.model_x);
	tv->entity.scale = GAME_TV_SCALE;
	entity_update_x(game, &tv->entity);

	return tv;
}


static void randomize_virus(virus_t *virus)
{
	virus->entity.position.y = randf();
	virus->entity.position.x = randf();
}


static virus_t * virus_new(game_t *game, stage_t *parent)
{
	virus_t	*virus;

	virus = pad_get(game->pad, sizeof(entity_t));
	fatal_if(!virus, "unable to allocate virus_t");

	virus->entity.type = ENTITY_TYPE_VIRUS;
	virus->entity.node = virus_node_new(&(stage_conf_t){ .parent = parent, .name = "virus", .alpha = 1.f }, &game->sars->projection_x, &virus->entity.model_x);
	virus->entity.scale = GAME_VIRUS_SCALE;
	randomize_virus(virus);
	entity_update_x(game, &virus->entity);

	return virus;
}


static void reset_virus(virus_t *virus)
{
	stage_set_active(virus->entity.node, 0);
	randomize_virus(virus);
}


static void infect_entity(game_t *game, entity_t *entity, const char *name)
{
	/* convert entity into inanimate virus (off the viruses array) */
	(void) virus_node_new(&(stage_conf_t){ .stage = entity->any.node, .replace = 1, .name = name, .active = 1, .alpha = 1.f }, &game->sars->projection_x, &entity->any.model_x);
	sfx_play(sfx.baby_infected);
	entity->any.type = ENTITY_TYPE_VIRUS;

	/* stick entity on a new_infections list for potential propagation */
	entity->virus.new_infections_next = game->new_infections;
	game->new_infections = entity;
}


static void hat_baby(game_t *game, baby_t *baby, mask_t *mask)
{
	(void) baby_hatted_node_new(&(stage_conf_t){ .stage = baby->entity.node, .replace = 1, .name = "baby-hatted", .active = 1, .alpha = 1.f }, &game->sars->projection_x, &baby->entity.model_x);

	stage_set_active(mask->entity.node, 0);
}


static void mask_adult(game_t *game, adult_t *adult, mask_t *mask)
{
	(void) adult_masked_node_new(&(stage_conf_t){ .stage = adult->entity.node, .replace = 1, .name = "adult-masked", .active = 1, .alpha = 1.f }, &game->sars->projection_x, &adult->entity.model_x);

	adult->masked += GAME_MASK_PROTECTION;
	sfx_play(sfx.adult_mine);
	stage_set_active(mask->entity.node, 0);
}


static void flash_entity(game_t *game, entity_any_t *any, unsigned count)
{
	if (!any->flashing) {
		any->flashing = 1;
		any->flashers_next = game->flashers_on_head;
		game->flashers_on_head = any;
	}

	any->flashes_remaining = count;
}


static void more_teepee(game_t *game, teepee_t *teepee)
{

	if (game->adult->holding) {
		/* disallow picking up teepee if holding something, flash what's held and the teepee we missed */
		flash_entity(game, &teepee->entity, 5);
		flash_entity(game, &game->adult->holding->any, 5);

		return;
	}

	for (unsigned i = 0; i < teepee->quantity; i++) {
		teepee_icon_t	*tp;

		tp = pad_get(game->pad, sizeof(entity_t));
		fatal_if(!tp, "unable to allocate teepee_icon_t");

		tp->entity.type = ENTITY_TYPE_TEEPEE_ICON;
		tp->entity.scale = GAME_TEEPEE_ICON_SCALE;
		/* TODO FIXME: clean this magic number salad up, there should probably just be a m4f_scale_scalar() wrapper for m4f_scale() that
		 * takes a single scalar float and constructs the v3f_t{} to pass m4f_scale() using the input scalar for all dimensions... then
		 * we'd have convenient scalars for the _SCALE defines and not these v3fs...  This works fine for now.
		 */
		tp->entity.node = teepee_node_new(&(stage_conf_t){ .parent = game->game_node, .name = "tp-icon", .layer = 8, .alpha = 1.f, .active = 1 }, &game->sars->projection_x, &tp->entity.model_x);
		tp->entity.model_x = m4f_translate(NULL,
						&(v3f_t){
							.x = ((game->teepee_cnt % 16) * 0.0625f) * 2.f - .9375f,
							.y = (.9687f - ((game->teepee_cnt / 16) * 0.0625f)) * 1.9375f + -.9375f,
							.z = 0.f
						});
		tp->entity.model_x = m4f_scale(&tp->entity.model_x, &tp->entity.scale);

		tp->next = game->teepee_head;
		game->teepee_head = tp;
		game->teepee_cnt++;
		if (game->teepee_cnt >= GAME_TP_WIN_THRESHOLD)
			game->state = GAME_STATE_OVER_WINNING;
	}
	(*teepee->bonus_release) = BONUS_NODE_RELEASE_MS;
	(*teepee->bonus_release_position) = teepee->entity.position;
	sfx_play(sfx.adult_mine);
	stage_set_active(teepee->entity.node, 0);
}


typedef struct baby_search_t {
	game_t	*game;
	baby_t	*baby;
} baby_search_t;


static ix2_search_status_t baby_search(void *cb_context, ix2_object_t *ix2_object, v2f_t *ix2_object_position, bb2f_t *ix2_object_aabb, void *object)
{
	baby_search_t	*search = cb_context;
	entity_t	*entity = object;

	switch (entity->any.type) {
	case ENTITY_TYPE_BABY:
		return IX2_SEARCH_MORE_MISS;

	case ENTITY_TYPE_ADULT:
		/* TODO: adult picks us up? */
		return IX2_SEARCH_MORE_MISS;

	case ENTITY_TYPE_VIRUS:
		if (!stage_get_active(entity->any.node))
			return IX2_SEARCH_MORE_MISS;

		/* baby gets infected, return positive hit count */
		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_TEEPEE:
	case ENTITY_TYPE_TV:
		return IX2_SEARCH_MORE_MISS;

	case ENTITY_TYPE_MASK:
		if (stage_get_active(entity->any.node))
			hat_baby(search->game, search->baby, &entity->mask);

		return IX2_SEARCH_MORE_MISS;

	default:
		assert(0);
	}
}


static ix2_search_status_t teepee_search(void *cb_context, ix2_object_t *ix2_object, v2f_t *ix2_object_position, bb2f_t *ix2_object_aabb, void *object)
{
	game_t		*game = cb_context;
	entity_t	*entity = object;

	switch (entity->any.type) {
	case ENTITY_TYPE_ADULT:
		if (stage_get_active(game->teepee->entity.node))
			more_teepee(game, game->teepee);

		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_TEEPEE: /* ignore self */
	case ENTITY_TYPE_BABY:
	case ENTITY_TYPE_MASK:
	case ENTITY_TYPE_TV:
	case ENTITY_TYPE_VIRUS: /* TODO: virus contaminates teepee? */
		return IX2_SEARCH_MORE_MISS;

	default:
		assert(0);
	}
}


static ix2_search_status_t tv_search(void *cb_context, ix2_object_t *ix2_object, v2f_t *ix2_object_position, bb2f_t *ix2_object_aabb, void *object)
{
	game_t		*game = cb_context;
	entity_t	*entity = object;

	switch (entity->any.type) {
	case ENTITY_TYPE_BABY: {
		baby_search_t	search = { .game = game, .baby = &entity->baby };
		v2f_t		delta;
		float		len;

		/* skip inactive babies, since rescues can linger inactive til respawned */
		if (!stage_get_active(entity->any.node))
			return IX2_SEARCH_MORE_MISS;

		/* skip held baby */
		if (game->adult->holding == entity)
			return IX2_SEARCH_MORE_MISS;

		/* if baby's distance from tv is within a range, inch it towards TV */
		delta = v2f_sub(&game->tv->entity.position, &entity->any.position);
		len = v2f_length(&delta);
		if (len < GAME_TV_RANGE_MIN || len > GAME_TV_RANGE_MAX)
			return IX2_SEARCH_MORE_MISS;

		/* move the baby towards the TV */
		delta = v2f_normalize(&delta);
		delta = v2f_mult_scalar(&delta, GAME_TV_ATTRACTION);
		entity->any.position = v2f_add(&entity->any.position, &delta);
		entity_update_x(game, &entity->any);

		/* check if the baby hit any viruses */
		/* XXX: note this is a nested search, see ix2_new() call. */
		if (ix2_search_by_aabb(game->ix2, NULL, NULL, &entity->any.aabb_x, baby_search, &search)) {
			/* baby hit a virus; infect it and spawn a replacement */
			infect_entity(game, entity, "baby-virus");
			game->babies_cnt--;
		}

		return IX2_SEARCH_MORE_HIT;
	}

	case ENTITY_TYPE_ADULT:
		/* XXX: should the tv affect the adult from a distance? */
		return IX2_SEARCH_MORE_MISS;

	case ENTITY_TYPE_VIRUS:
	case ENTITY_TYPE_TV:
	case ENTITY_TYPE_TEEPEE:
	case ENTITY_TYPE_MASK:
		return IX2_SEARCH_MORE_MISS;

	default:
		assert(0);
	}
}


static ix2_search_status_t mask_search(void *cb_context, ix2_object_t *ix2_object, v2f_t *ix2_object_position, bb2f_t *ix2_object_aabb, void *object)
{
	game_t		*game = cb_context;
	entity_t	*entity = object;

	switch (entity->any.type) {
	case ENTITY_TYPE_BABY:
		/* skip inactive babies, since rescues can linger inactive til respawned */
		if (!stage_get_active(entity->any.node))
			return IX2_SEARCH_MORE_MISS;

		if (stage_get_active(game->mask->entity.node))
			hat_baby(game, &entity->baby, game->mask);

		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_ADULT:
		if (stage_get_active(game->mask->entity.node))
			mask_adult(game, &entity->adult, game->mask);

		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_MASK: /* ignore self */
	case ENTITY_TYPE_TEEPEE:
	case ENTITY_TYPE_TV:
	case ENTITY_TYPE_VIRUS: /* TODO: virus contaminates mask? */
		return IX2_SEARCH_MORE_MISS;

	default:
		assert(0);
	}
}


typedef struct virus_search_t {
	game_t	*game;
	virus_t	*virus;
} virus_search_t;

static ix2_search_status_t virus_search(void *cb_context, ix2_object_t *ix2_object, v2f_t *ix2_object_position, bb2f_t *ix2_object_aabb, void *object)
{
	virus_search_t	*search = cb_context;
	entity_t	*entity = object;

	switch (entity->any.type) {
	case ENTITY_TYPE_BABY:
		/* skip inactive babies, since rescues can linger inactive til respawned */
		if (!stage_get_active(entity->any.node))
			return IX2_SEARCH_MORE_MISS;

		/* virus hit a baby; infect it and spawn a replacement */
		infect_entity(search->game, entity, "baby-virus");
		search->game->babies_cnt--;

		return IX2_SEARCH_MORE_HIT;

	case ENTITY_TYPE_ADULT:
		/* convert adult into inanimate virus (off the viruses array) */
		if (entity->adult.masked) {
			reset_virus(search->virus);

			if (!--entity->adult.masked)
				(void) adult_node_new(&(stage_conf_t){ .stage = search->game->adult->entity.node, .replace = 1, .name = "adult-masked", .active = 1, .alpha = 1.f }, &search->game->sars->projection_x, &search->game->adult->entity.model_x);

			flash_entity(search->game, &entity->any, 4);

			return IX2_SEARCH_STOP_MISS;
		}

		(void) virus_node_new(&(stage_conf_t){ .stage = entity->any.node, .replace = 1, .name = "adult-virus", .active = 1, .alpha = 1.f }, &search->game->sars->projection_x, &entity->any.model_x);
		sfx_play(sfx.adult_infected);
		search->game->state = GAME_STATE_OVER;
		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_MASK: /* TODO: virus contaminates mask? */
	case ENTITY_TYPE_TEEPEE:
	case ENTITY_TYPE_TV:
	case ENTITY_TYPE_VIRUS:
		return IX2_SEARCH_MORE_MISS;

	default:
		assert(0);
	}
}


/* animate the viruses:
 * - anything newly infected becomes an inanimate virus (change their node)
 *   and the virus respawns somewhere
 * - if the newly infected thing is the adult, the game ends
 */
static void update_entities(play_t *play, game_t *game)
{
	virus_search_t	search = { .game = game };

	assert(play);
	assert(game);

	if (randf() > .95f && !stage_get_active(game->tv->entity.node)) {
		/* sometimes turn on the TV at a random location, we
		 * get stuck to it */
		play_ticks_reset(play, GAME_TV_TIMER);
		game->tv->entity.position.x = randf();
		game->tv->entity.position.y = randf();
		entity_update_x(game, &game->tv->entity);
		stage_set_active(game->tv->entity.node, 1);
	}

	if (randf() > .98f && !stage_get_active(game->mask->entity.node)) {
		/* sometimes activate a mask powerup */
		game->mask->entity.position.x = randf();
		game->mask->entity.position.y = -1.2f;
		entity_update_x(game, &game->mask->entity);
		stage_set_active(game->mask->entity.node, 1);
	}

	if (randf() > .45f && !stage_get_active(game->teepee->entity.node)) {
		/* sometimes activate a teepee "powerup" */
		static struct {
			unsigned	qty;
			float		chance;
		} quantities[] = {
			{ .qty =  4, .chance = .25 },
			{ .qty =  6, .chance = .125 },
			{ .qty =  9, .chance = .06 },
			{ .qty = 12, .chance = .04 },
			{ .qty = 18, .chance = .03 },
			{ .qty = 24, .chance = .02 },
			{ .qty = 30, .chance = .01 },
			{ .qty = 36, .chance = .001 },
		};

		game->teepee->quantity = 1;
		for (unsigned i = 0; i < NELEMS(quantities); i++) {
			if (randf() * .5f + .5f <= quantities[i].chance)
				game->teepee->quantity = quantities[i].qty;
		}

		bonus_node_new(&(stage_conf_t){.parent = game->game_node, .active = 1, .alpha = 1.f, .name = "teepee-bonus", .layer = 6},
			game->teepee->quantity,
			&game->sars->projection_x,
			&game->teepee->entity.position,
			.03f/* FIXME magic number alert: bonus scale */,
			&game->teepee->bonus_release,
			&game->teepee->bonus_release_position);

		game->teepee->entity.position.x = randf();
		game->teepee->entity.position.y = -1.2f;
		entity_update_x(game, &game->teepee->entity);
		stage_set_active(game->teepee->entity.node, 1);
	}

	if (stage_get_active(game->mask->entity.node)) { /* if the mask is on, move it and possibly retire it */
		game->mask->entity.position.y += GAME_MASK_SPEED;
		entity_update_x(game, &game->mask->entity);

		/* did it hit something? */
		if (!ix2_search_by_aabb(game->ix2, NULL,  NULL, &game->mask->entity.aabb_x, mask_search, game)) {
			/* No?, is it off-screen? */
			if (game->mask->entity.position.y > 1.2f)
				stage_set_active(game->mask->entity.node, 0);
		}
	}

	if (stage_get_active(game->teepee->entity.node)) { /* if the teepee is on, move it and possibly retire it */
		game->teepee->entity.position.y += GAME_MASK_SPEED;
		entity_update_x(game, &game->teepee->entity);

		/* did it hit something? */
		if (!ix2_search_by_aabb(game->ix2, NULL,  NULL, &game->teepee->entity.aabb_x, teepee_search, game)) {
			/* No?, is it off-screen? */
			if (game->teepee->entity.position.y > 1.2f) {
				stage_set_active(game->teepee->entity.node, 0);
				/* release the bonus immediately */
				*(game->teepee->bonus_release) = 1;
			}
		}
	}

	if (stage_get_active(game->tv->entity.node)) { /* if the TV is on, move nearby babies towards it */
		bb2f_t	range_aabb = { .min = { -GAME_TV_RANGE_MAX, -GAME_TV_RANGE_MAX }, .max = { GAME_TV_RANGE_MAX, GAME_TV_RANGE_MAX } };

		ix2_search_by_aabb(game->ix2, &game->tv->entity.position, NULL, &range_aabb, tv_search, game);
	}

	for (int i = 0; i < NELEMS(game->viruses); i++) {
		virus_t	*virus = game->viruses[i];

		/* are they off-screen? */
		if (virus->entity.position.y > 1.2f) {

			if (stage_get_active(virus->entity.node)) {
				/* active and off-screen gets randomize and inactivated */
				reset_virus(virus);
			} else {
				/* inactive and off-screen gets activated and moved to the
				 * top */
				stage_set_active(virus->entity.node, 1);
				virus->entity.position.y = -1.2f;
			}
		}

		virus->entity.position.y += GAME_VIRUS_SPEED;
		entity_update_x(game, &virus->entity);

		if (stage_get_active(virus->entity.node)) {
			search.virus = virus;

			/* search ix2 for collisions */
			if (ix2_search_by_aabb(game->ix2, NULL, NULL, &virus->entity.aabb_x, virus_search, &search))
				reset_virus(virus);

			/* propagate any new infections */
			while (game->new_infections) {
				entity_t	*infection = game->new_infections;

				game->new_infections = infection->virus.new_infections_next;

				search.virus = &infection->virus;
				(void) ix2_search_by_aabb(game->ix2, NULL, NULL, &infection->virus.entity.aabb_x, virus_search, &search);
			}
		}
	}
}


static ix2_search_status_t adult_search(void *cb_context, ix2_object_t *ix2_object, v2f_t *ix2_object_position, bb2f_t *ix2_object_aabb, void *object)
{
	game_t		*game = cb_context;
	entity_t	*entity = object;

	switch (entity->any.type) {
	case ENTITY_TYPE_BABY:
		if (!game->adult->holding) {
			sfx_play(sfx.baby_held);
			game->adult->holding = entity;
		}

		/* we should probably keep looking because there could be a virus too,
		 * but fuck it, these types of bugs are fun in silly games.
		 */
		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_ADULT:
		/* ignore ourselves */
		return IX2_SEARCH_MORE_MISS;

	case ENTITY_TYPE_VIRUS:
		if (!stage_get_active(entity->any.node))
			return IX2_SEARCH_MORE_MISS;

		if (game->adult->masked) {
			reset_virus(&entity->virus);

			if (!--game->adult->masked)
				(void) adult_node_new(&(stage_conf_t){ .stage = game->adult->entity.node, .replace = 1, .name = "adult-masked", .active = 1, .alpha = 1.f }, &game->sars->projection_x, &game->adult->entity.model_x);
			flash_entity(game, &game->adult->entity, 4);
			reset_virus(&entity->virus);

			return IX2_SEARCH_MORE_MISS;
		}

		/* convert adult into inanimate virus (off the viruses array) */
		(void) virus_node_new(&(stage_conf_t){ .stage = game->adult->entity.node, .replace = 1, .name = "adult-virus", .active = 1, .alpha = 1.f }, &game->sars->projection_x, &game->adult->entity.model_x);
		sfx_play(sfx.adult_infected);

		if (game->adult->holding) {
			(void) virus_node_new(&(stage_conf_t){ .stage = game->adult->holding->any.node, .replace = 1, .name = "baby-virus", .active = 1, .alpha = 1.f }, &game->sars->projection_x, &game->adult->holding->any.model_x);
			sfx_play(sfx.baby_infected);
		}
		game->state = GAME_STATE_OVER;
		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_TV:
		if (!stage_get_active(entity->any.node))
			return IX2_SEARCH_MORE_MISS;

		game->adult->captivated = 1;
		sfx_play(sfx.adult_captivated);
		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_MASK:
		if (stage_get_active(entity->any.node))
			mask_adult(game, game->adult, &entity->mask);

		return IX2_SEARCH_MORE_MISS;

	case ENTITY_TYPE_TEEPEE:
		if (stage_get_active(entity->any.node))
			more_teepee(game, &entity->teepee);

		return IX2_SEARCH_MORE_HIT;

	default:
		assert(0);
	}
}


static void game_move_adult(game_t *game, v2f_t *dir)
{
	assert(game);
	assert(dir);

	if (game->adult->captivated)
		return;

	game->adult->entity.position.x += dir->x;
	game->adult->entity.position.y += dir->y;

	/* prevent the player from going too far off the reservation */
	if (game->adult->entity.position.x > 1.1f)
		game->adult->entity.position.x = 1.1f;

	if (game->adult->entity.position.x < -1.1f)
		game->adult->entity.position.x = -1.1f;

	if (game->adult->entity.position.y > 1.1f)
		game->adult->entity.position.y = 1.1f;

	if (game->adult->entity.position.y < -1.1f)
		game->adult->entity.position.y = -1.1f;

	entity_update_x(game, &game->adult->entity);

	if (game->adult->holding) {
		game->adult->holding->any.position = game->adult->entity.position;
		entity_update_x(game, &game->adult->holding->any);

		if (game->adult->entity.position.x > 1.05f ||
		    game->adult->entity.position.x < -1.05f ||
		    game->adult->entity.position.y > 1.05f ||
		    game->adult->entity.position.y < -1.05f) {

			/* rescued baby */
			sfx_play(sfx.baby_rescued);

			/* make the rescued baby available for respawn reuse */
			game->adult->holding->any.flashes_remaining = 0;
			stage_set_active(game->adult->holding->any.node, 0);
			game->adult->holding->baby.rescues_next = game->rescues_head;
			game->rescues_head = &game->adult->holding->baby;
			game->babies_cnt--;

			game->adult->holding = NULL;
			game->adult->rescues++;
		}
	}

	/* search ix2 for collisions */
	ix2_search_by_aabb(game->ix2, NULL, NULL, &game->adult->entity.aabb_x, adult_search, game);
}


static void reset_game(game_t *game)
{
	ix2_reset(game->ix2);
	stage_free(game->game_node);

	game->pad = pad_free(game->pad);

	game->game_node = stage_new(&(stage_conf_t){ .parent = game->stage, .name = "game", .active = 1, .alpha = 1.f }, NULL, NULL);

	game->babies_node = stage_new(&(stage_conf_t){ .parent = game->game_node, .name = "babies", .layer = 4, .alpha = 1.f }, NULL, NULL);
	game->viruses_node = stage_new(&(stage_conf_t){ .parent = game->game_node, .name = "viruses", .layer = 5, .alpha = 1.f }, NULL, NULL);
	game->score_node = stage_new(&(stage_conf_t){ .parent = game->game_node, .name = "score", .layer = 9, .alpha = 1 }, NULL, NULL);

	game->pad = pad_new(sizeof(entity_t) * 32, PAD_FLAGS_ZERO);

	game->teepee_cnt = 0;
	game->teepee_head = NULL;
	game->flashers_on_head = game->flashers_off_head = NULL;
	game->rescues_head = NULL;
	game->tv = tv_new(game, game->game_node);
	game->teepee = teepee_new(game, game->game_node);
	game->mask = mask_new(game, game->game_node);
	game->adult = adult_new(game, game->game_node);
	for (int i = 0; i < NELEMS(game->viruses); i++)
		game->viruses[i] = virus_new(game, game->viruses_node);

	game->babies_cnt = GAME_NUM_BABIES;
	for (int i = 0; i < game->babies_cnt; i++)
		(void) baby_new(game, game->babies_node, NULL);

	stage_set_active(game->adult->entity.node, 1);
	stage_set_active(game->babies_node, 1);
	stage_set_active(game->viruses_node, 1);

	game->state = GAME_STATE_PLAYING;
}


static void show_score(game_t *game)
{
	unsigned	score = game->adult->rescues * 420;

	for (unsigned i = 1000000000, pos = 0; i > 0; score %= i, i /= 10, pos++) {
		unsigned	v = score / i;

		digit_node_new(&(stage_conf_t){ .parent = game->score_node, .name = "score-digit", .active = 1, .alpha = 1.f }, v, &game->sars->projection_x, &game->score_digits_x[pos]);
	}

	stage_set_active(game->score_node, 1);
}


static void * game_init(play_t *play, int argc, char *argv[], unsigned flags)
{
	sars_t	*sars = play_context(play, SARS_CONTEXT_SARS);
	game_t	*game;

	assert(sars);

	game = calloc(1, sizeof(game_t));
	fatal_if(!game, "Unable to allocate game_t");

	game->sars = sars;
	game->stage = sars->stage;
	game->plasma_node = plasma_node_new(&(stage_conf_t){ .parent = sars->stage, .name = "plasma", .alpha = 1 }, &sars->projection_x);

	game->ix2 = ix2_new(NULL, 4, 4, 2 /* support two simultaneous searches: tv_search->baby_search */);

	/* setup transformation matrices for the score digits, this is really fast and nasty hack because
	 * I am completely delerious and ready to fall asleep.
	 */
	for (int i = 0; i < NELEMS(game->score_digits_x); i++) {
		game->score_digits_x[i] = m4f_translate(NULL, &(v3f_t){ 1.f / NELEMS(game->score_digits_x) * i - .5f, 0.f, 0.f });
		game->score_digits_x[i] = m4f_scale(&game->score_digits_x[i], &GAME_DIGITS_SCALE);
	}

	sfx_init();

	return game;
}


static void game_enter(play_t *play, void *context)
{
	game_t	*game = context;

	assert(game);

	play_music_set(play, PLAY_MUSIC_FLAG_LOOP|PLAY_MUSIC_FLAG_IDEMPOTENT, "assets/game.ogg");
	play_ticks_reset(play, GAME_ENTITIES_TIMER);
	stage_set_active(game->plasma_node, 1);
	reset_game(game);
}


static void game_update(play_t *play, void *context)
{
	sars_t	*sars = play_context(play, SARS_CONTEXT_SARS);
	game_t	*game = context;

	assert(game);
	assert(sars);

	switch (game->state) {
	case GAME_STATE_PLAYING: {
		if (play_ticks_elapsed(play, GAME_ENTITIES_TIMER, GAME_ENTITIES_DELAY_MS))
			update_entities(play, game);

		if (play_ticks_elapsed(play, GAME_KBD_TIMER, GAME_KBD_DELAY_MS)) {
			const Uint8	*key_state = SDL_GetKeyboardState(NULL);
			v2f_t		dir = {}, *move = NULL;

			/* TODO: acceleration curve for movement?  it'd enable more precise
			 * negotiating of obstacles, but that's not really worthwhile until there's
			 * pixel-precision collision detection...
			 */
			if (key_state[SDL_SCANCODE_LEFT] || key_state[SDL_SCANCODE_A]) {
				dir.x += -GAME_ADULT_SPEED;
				move = &dir;
			}

			if (key_state[SDL_SCANCODE_RIGHT] || key_state[SDL_SCANCODE_D]) {
				dir.x += GAME_ADULT_SPEED;
				move = &dir;
			}

			if (key_state[SDL_SCANCODE_UP] || key_state[SDL_SCANCODE_W]) {
				dir.y += GAME_ADULT_SPEED;
				move = &dir;
			}

			if (key_state[SDL_SCANCODE_DOWN] || key_state[SDL_SCANCODE_S]) {
				dir.y += -GAME_ADULT_SPEED;
				move = &dir;
			}

			if (game->touch.active) {
				dir = v2f_sub(&game->touch.position, &game->adult->entity.position);
				move = &dir;
			}

			if (move) {
				float	distance;

				distance = v2f_length(move);
				if (distance) {
					*move = v2f_normalize(move);
					*move = v2f_mult_scalar(move, distance < GAME_ADULT_SPEED ? distance : GAME_ADULT_SPEED);
				}

				game_move_adult(game, move);
			}
		}

		if (play_ticks_elapsed(play, GAME_TV_TIMER, GAME_TV_DELAY_MS)) {
			stage_set_active(game->tv->entity.node, 0);
			game->adult->captivated = 0;
		}

		if (play_ticks_elapsed(play, GAME_FLASHERS_TIMER, GAME_FLASHERS_DELAY_MS)) {
			entity_any_t	*new_off = NULL, *new_on = NULL;

			if (game->flashers_on_head) {
				for (entity_any_t *e = game->flashers_on_head, *e_next; e; e = e_next) {
					e_next = e->flashers_next;

					if (!e->flashes_remaining) {
						e->flashing = 0;
						e->flashers_next = NULL;
						continue;
					}

					if (e->node)
						stage_set_alpha(e->node, .25f);

					e->flashes_remaining--;
					e->flashers_next = new_off;
					new_off = e;
				}
			}

			if (game->flashers_off_head) {
				for (entity_any_t *e = game->flashers_off_head; e; e = e->flashers_next) {
					if (e->node)
						stage_set_alpha(e->node, 1.f);
				}

				new_on = game->flashers_off_head;
			}

			game->flashers_off_head = new_off;
			game->flashers_on_head = new_on;
		}

		if (play_ticks_elapsed(play, GAME_NEWBABIES_TIMER, GAME_NEWBABIES_DELAY_MS)) {
			for (unsigned n = GAME_NUM_BABIES - game->babies_cnt; n > 0; n--) {
				baby_search_t	search = { .game = game, .baby = game->rescues_head };

				if (search.baby)
					game->rescues_head = search.baby->rescues_next;

				search.baby = baby_new(game, game->babies_node, search.baby);

				/* check if the new baby is immediately infected */
				if (ix2_search_by_aabb(game->ix2, NULL, NULL, &search.baby->entity.aabb_x, baby_search, &search))
					infect_entity(game, (entity_t *)search.baby, "baby-virus");
				else
					game->babies_cnt++;
			}
		}

		break;
	}

	/* winner game over states */
	case GAME_STATE_OVER_WINNING:
		show_score(game);
		play_ticks_reset(play, GAME_OVER_TIMER);
		game->state = GAME_STATE_OVER_WINNING_DELAY;
		break;

	case GAME_STATE_OVER_WINNING_DELAY: {
		float		t = (float)(play_ticks(play, GAME_OVER_TIMER) * 1.f / (float)GAME_OVER_DELAY_MS);
		teepee_icon_t	*tp = game->teepee_head;

		if (t > 1.f) {
			game->state = GAME_STATE_OVER_WINNING_WAITING;
			break;
		}

		/* explode the hoarded TP */
		for (size_t i = 0; tp != NULL; tp = tp->next, i++) {
			tp->entity.model_x = m4f_translate(NULL,
							&(v3f_t){
								.x = (((i % 16) * 0.0625f) * 2.f - .9375f) * (1.f + t * 32.f),
								.y = ((.9687f - ((i / 16) * 0.0625f)) * 1.9375f + -.9375f) * (1.f + t * 32.f),
								.z = 0.f
							});
			tp->entity.model_x = m4f_scale(&tp->entity.model_x, &tp->entity.scale);
		}
		break;
	}

	case GAME_STATE_OVER_WINNING_WAITING: {
		teepee_icon_t	*tp = game->teepee_head;
		float		t = (float)(play_ticks(play, GAME_OVER_TIMER) % 6283) * .005f;
		float		r = sinf(t);

		/* just do nothing while animating the teepee icons, waiting for a keypress of some kind */
		for (size_t i = 0; tp != NULL; tp = tp->next, i++) {
			tp->entity.model_x = m4f_translate(NULL,
							&(v3f_t){
								.x = ((i % 16) * 0.0625f) * 2.f - .9375f,
								.y = ((1.f - fmod((i / 16 * 0.0625f) + (float)(play_ticks(play, GAME_OVER_TIMER) % 10000) * .0001f, 1.f))) * 3.f - 1.5f,
								.z = 0.f
							});
			tp->entity.model_x = m4f_scale(&tp->entity.model_x, &tp->entity.scale);
			tp->entity.model_x = m4f_rotate(&tp->entity.model_x, &(v3f_t){ .x = 0.f, .y = 0.f, .z = 1.f }, r);
		}
		break;
	}

	/* loser game over states */
	case GAME_STATE_OVER:
		show_score(game);
		play_ticks_reset(play, GAME_OVER_TIMER);
		game->state = GAME_STATE_OVER_DELAY;
		break;

	case GAME_STATE_OVER_DELAY:
		if (!play_ticks_elapsed(play, GAME_OVER_TIMER, GAME_OVER_DELAY_MS))
			break;

		/* maybe throw something on-screen? */
		game->state = GAME_STATE_OVER_WAITING;
		break;

	case GAME_STATE_OVER_WAITING:
		/* just do nothing and wait for a keypress of some kind */
		break;

	default:
		assert(0);
	}

	/* always dirty the stage in the game context */
	stage_dirty(sars->stage);
}


static void game_dispatch(play_t *play, void *context, SDL_Event *event)
{
	game_t	*game = context;

	/* global handlers */
	sars_dispatch(play, context, event);

	switch (event->type) {
	case SDL_KEYDOWN:
		if (event->key.keysym.sym == SDLK_ESCAPE)
			exit(0);

		if (game->state == GAME_STATE_OVER_WAITING ||
		    game->state == GAME_STATE_OVER_WINNING_WAITING) {
			reset_game(game);
			break;
		}

		break;

	case SDL_FINGERDOWN:
		if (game->state == GAME_STATE_OVER_WAITING ||
		    game->state == GAME_STATE_OVER_WINNING_WAITING)
			reset_game(game);
		/* fallthrough */
	case SDL_FINGERMOTION:
		if ((game->touch.active &&
		     game->touch.touch_id == event->tfinger.touchId &&
		     game->touch.finger_id == event->tfinger.fingerId) ||
		    !game->touch.active) {
			game->touch.active = 1;
			game->touch.touch_id = event->tfinger.touchId;
			game->touch.finger_id = event->tfinger.fingerId;

			sars_ndc_to_bpc(game->sars,
					(event->tfinger.x + -.5f),
					-(event->tfinger.y + -.5f),
					&game->touch.position.x,
					&game->touch.position.y);

			/* XXX: note the scaling is larger than -1..+1 so the babies can be saved fullscreen,
			 * this also has the effect of putting the adult a bit offset from the finger so you
			 * can actually see what you're doing more of the time (not near the center though).
			 */
			game->touch.position.x *= 2.4f;
			game->touch.position.y *= 2.4f;
		}
		break;

	case SDL_FINGERUP:
		if (game->touch.active &&
		    game->touch.touch_id == event->tfinger.touchId &&
		    game->touch.finger_id == event->tfinger.fingerId)
			game->touch.active = 0;
		break;

	default:
		break;
	}
}


const play_ops_t	game_ops = {
	.init = game_init,
	.update = game_update,
	.render = sars_render,
	.dispatch = game_dispatch,
	.enter = game_enter,
};
