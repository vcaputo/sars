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
#include "baby-node.h"
#include "bb2f.h"
#include "bb3f.h"
#include "digit-node.h"
#include "glad.h"
#include "m4f.h"
#include "m4f-3dx.h"
#include "m4f-bbx.h"
#include "macros.h"
#include "plasma-node.h"
#include "sars.h"
#include "sfx.h"
#include "tv-node.h"
#include "v2f.h"
#include "virus-node.h"

#define	GAME_NUM_VIRUSES	8
#define GAME_NUM_BABIES		10

#define GAME_VIRUS_SPEED	.01f
#define GAME_ADULT_SPEED	.04f

#define GAME_VIRUS_DELAY_MS	20
#define GAME_VIRUS_TIMER	PLAY_TICKS_TIMER1

#define GAME_TV_DELAY_MS	3000
#define GAME_TV_TIMER		PLAY_TICKS_TIMER3

#define GAME_KBD_DELAY_MS	20
#define GAME_KBD_TIMER		PLAY_TICKS_TIMER4

#define GAME_OVER_DELAY_MS	1000
#define GAME_OVER_TIMER		PLAY_TICKS_TIMER2

#define GAME_BABY_SCALE		(v3f_t){.05f, .05f, .05f}
#define GAME_ADULT_SCALE	(v3f_t){.07f, .07f, .07f}
#define GAME_TV_SCALE		(v3f_t){.15f, .15f, .15f}
#define GAME_VIRUS_SCALE	(v3f_t){.05f, .05f, .05f}
#define GAME_DIGITS_SCALE	(v3f_t){.05f, .05f, .05f}

/* every entity just starts with a unit cube AABB and is transformed with a matrix into its position,
 * so here's a convenient aabb to feed into those transformations as needed.
 */
static const bb3f_t	any_aabb = { .min = { -1.f -1.f, -1.f}, .max = { 1.f, 1.f, 1.f } };

typedef enum game_state_t {
	GAME_STATE_PLAYING,
	GAME_STATE_OVER,
	GAME_STATE_OVER_DELAY,
	GAME_STATE_OVER_WAITING
} game_state_t;

typedef enum entity_type_t {
	ENTITY_TYPE_BABY,
	ENTITY_TYPE_ADULT,
	ENTITY_TYPE_VIRUS,
	ENTITY_TYPE_TV,
} entity_type_t;

typedef union entity_t entity_t;

typedef struct entity_any_t {
	entity_type_t	type;
	stage_t		*node;
	ix2_object_t	*ix2_object;
	v2f_t		position;
	v3f_t		scale;
	m4f_t		model_x;
	bb2f_t		aabb_x;
} entity_any_t;

typedef struct baby_t {
	entity_any_t	entity;
} baby_t;

typedef struct virus_t {
	entity_any_t	entity;
} virus_t;

typedef struct adult_t {
	entity_any_t	entity;
	unsigned	rescues;
	unsigned	frozen:1;
	entity_t	*holding;
} adult_t;

typedef struct tv_t {
	entity_any_t	entity;
} tv_t;

union entity_t {
	entity_any_t	any;
	baby_t		baby;
	virus_t		virus;
	adult_t		adult;
	tv_t		tv;
};


typedef struct game_t {
	game_state_t	state;

	stage_t		*stage;
	stage_t		*game_node;
	stage_t		*babies_node;
	stage_t		*viruses_node;
	stage_t		*plasma_node;
	stage_t		*score_node;
	ix2_t		*ix2;
	pad_t		*pad;

	adult_t		*adult;
	tv_t		*tv;
	virus_t		*viruses[GAME_NUM_VIRUSES];
	m4f_t		score_digits_x[10];
} game_t;


static inline float randf(void)
{
	return 2.f / RAND_MAX * rand() - 1.f;
}


/* update the entity's transformation and position in the index */
static void entity_update_x(game_t *game, entity_any_t *entity)
{

	entity->model_x = m4f_translate(NULL, &(v3f_t){entity->position.x, entity->position.y, 0.f});
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
	fatal_if(!adult, "unale to allocate adult_t");

	adult->entity.type = ENTITY_TYPE_ADULT;
	adult->entity.node = adult_node_new(&(stage_conf_t){ .parent = parent, .name = "adult", .layer = 3, .alpha = 1.f }, &adult->entity.model_x);
	adult->entity.scale = GAME_ADULT_SCALE;
	entity_update_x(game, &adult->entity);

	return adult;
}



static baby_t * baby_new(game_t *game, stage_t *parent)
{
	baby_t	*baby;

	baby = pad_get(game->pad, sizeof(entity_t));
	fatal_if(!baby, "unale to allocate baby_t");

	baby->entity.type = ENTITY_TYPE_BABY;
	baby->entity.node = baby_node_new(&(stage_conf_t){ .parent = parent, .name = "baby", .active = 1, .alpha = 1.f }, &baby->entity.model_x);
	baby->entity.scale = GAME_BABY_SCALE;
	baby->entity.position.x = randf();
	baby->entity.position.y = randf();
	entity_update_x(game, &baby->entity);

	return baby;
}


static tv_t * tv_new(game_t *game, stage_t *parent)
{
	tv_t	*tv;

	tv = pad_get(game->pad, sizeof(entity_t));
	fatal_if(!tv, "unale to allocate tv_t");

	tv->entity.type = ENTITY_TYPE_TV;
	tv->entity.node = tv_node_new(&(stage_conf_t){ .parent = parent, .name = "tv", .layer = 1, .alpha = 1.f }, &tv->entity.model_x);
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
	fatal_if(!virus, "unale to allocate virus_t");

	virus->entity.type = ENTITY_TYPE_VIRUS;
	virus->entity.node = virus_node_new(&(stage_conf_t){ .parent = parent, .name = "virus", .alpha = 1.f }, &virus->entity.model_x);
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
		/* convert baby into inanimate virus (off the viruses array) */
		(void) virus_node_new(&(stage_conf_t){ .parent = entity->any.node, .adopt = 1, .name = "baby-virus", .active = 1, .alpha = 1.f }, &entity->any.model_x);
		sfx_play(sfx.baby_infected);
		entity->any.type = ENTITY_TYPE_VIRUS;

		/* add new baby */
		(void) baby_new(search->game, search->game->babies_node);

		/* reset virus */
		reset_virus(search->virus);

		/* stop searching */
		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_ADULT:
		/* convert adult into inanimate virus (off the viruses array) */
		(void) virus_node_new(&(stage_conf_t){ .parent = entity->any.node, .adopt = 1, .name = "adult-virus", .active = 1, .alpha = 1.f }, &entity->any.model_x);
		sfx_play(sfx.adult_infected);
		search->game->state = GAME_STATE_OVER;
		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_VIRUS:
	case ENTITY_TYPE_TV:
		return IX2_SEARCH_MORE_MISS;

	default:
		assert(0);
	}

	/* XXX: I'm not really caring about the return value for now */
}


/* animate the viruses:
 * - anything newly infected becomes an inanimate virus (change their node)
 *   and the virus respawns somewhere
 * - if the newly infected thing is the adult, the game ends
 */
static void update_viruses(play_t *play, game_t *game)
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
			ix2_search_by_aabb(game->ix2, NULL, NULL, &virus->entity.aabb_x, virus_search, &search);
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

		/* convert adult into inanimate adult (of the adultes array) */
		stage_free(game->adult->entity.node);
		game->adult->entity.node = virus_node_new(&(stage_conf_t){ .parent = game->viruses_node, .name = "adult-virus", .active = 1, .alpha = 1.f }, &game->adult->entity.model_x);
		sfx_play(sfx.adult_infected);

		if (game->adult->holding) {
			stage_free(game->adult->holding->any.node);
			game->adult->holding->any.node = virus_node_new(&(stage_conf_t){ .parent = game->viruses_node, .name = "baby-virus", .active = 1, .alpha = 1.f }, &game->adult->holding->any.model_x);
			sfx_play(sfx.baby_infected);
		}
		game->state = GAME_STATE_OVER;
		return IX2_SEARCH_STOP_HIT;

	case ENTITY_TYPE_TV:
		if (!stage_get_active(entity->any.node))
			return IX2_SEARCH_MORE_MISS;

		game->adult->frozen = 1;
		return IX2_SEARCH_STOP_HIT;

	default:
		assert(0);
	}

	/* XXX: I'm not really caring about the return value for now */
}


static void game_move_adult(game_t *game, v2f_t *dir)
{
	assert(game);
	assert(dir);

	if (game->adult->frozen)
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

			game->adult->holding->any.position.x = randf();
			game->adult->holding->any.position.y = randf();
			entity_update_x(game, &game->adult->holding->any);

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

	if (game->pad) /* XXX FIXME: this is a stupidty in libpad */
		pad_free(game->pad);

	game->game_node = stage_new(&(stage_conf_t){ .parent = game->stage, .name = "game", .active = 1, .alpha = 1.f }, NULL, NULL);

	game->babies_node = stage_new(&(stage_conf_t){ .parent = game->game_node, .name = "babies", .layer = 4, .alpha = 1.f }, NULL, NULL);
	game->viruses_node = stage_new(&(stage_conf_t){ .parent = game->game_node, .name = "viruses", .layer = 5, .alpha = 1.f }, NULL, NULL);
	game->score_node = stage_new(&(stage_conf_t){ .parent = game->game_node, .name = "score", .layer = 7, .alpha = 1 }, NULL, NULL);

	game->pad = pad_new(sizeof(entity_t) * 32, PAD_FLAGS_ZERO);

	game->tv = tv_new(game, game->game_node);
	game->adult = adult_new(game, game->game_node);
	for (int i = 0; i < NELEMS(game->viruses); i++)
		game->viruses[i] = virus_new(game, game->viruses_node);

	for (int i = 0; i < GAME_NUM_BABIES; i++)
		(void) baby_new(game, game->babies_node);

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

		digit_node_new(&(stage_conf_t){ .parent = game->score_node, .name = "score-digit", .active = 1, .alpha = 1.f }, v, &game->score_digits_x[pos]);
	}

	stage_set_active(game->score_node, 1);
}


static void * game_init(play_t *play, int argc, char *argv[])
{
	sars_t	*sars = play_context(play, SARS_CONTEXT_SARS);
	game_t	*game;

	assert(sars);

	game = calloc(1, sizeof(game_t));
	fatal_if(!game, "Unable to allocate game_t");

	game->stage = sars->stage;
	game->plasma_node = plasma_node_new(&(stage_conf_t){ .parent = sars->stage, .name = "plasma", .alpha = 1 });

	game->ix2 = ix2_new(NULL, 4, 4, 1 /* increase for nested searches */);

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
	play_ticks_reset(play, GAME_VIRUS_TIMER);
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
		if (play_ticks_elapsed(play, GAME_VIRUS_TIMER, GAME_VIRUS_DELAY_MS))
			update_viruses(play, game);

		if (play_ticks_elapsed(play, GAME_KBD_TIMER, GAME_KBD_DELAY_MS)) {
			const Uint8	*key_state = SDL_GetKeyboardState(NULL);
			v2f_t		dir = {}, *move = NULL;

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

			if (move)
				game_move_adult(game, move);
		}

		if (play_ticks_elapsed(play, GAME_TV_TIMER, GAME_TV_DELAY_MS)) {
			stage_set_active(game->tv->entity.node, 0);
			game->adult->frozen = 0;
		}

		break;
	}

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

	/* just always dirty the stage in the game context */
	stage_dirty(sars->stage);
}


static void game_dispatch(play_t *play, void *context, SDL_Event *event)
{
	game_t	*game = context;

	/* global handlers */
	sars_dispatch(play, context, event);

	/* anything more to do here? */
	switch (event->type) {
	case SDL_KEYDOWN:
		if (event->key.keysym.sym == SDLK_ESCAPE)
			exit(0);

		if (game->state == GAME_STATE_OVER_WAITING) {
			reset_game(game);
			break;
		}

		break;

	default:
		break;
	}
}


const play_ops_t	game_ops = {
	.init = game_init,
//	.shutdown = game_shutdown,
	.update = game_update,
	.render = sars_render,
	.dispatch = game_dispatch,
	.enter = game_enter,
};
