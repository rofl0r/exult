/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Actors.cc - Game actors.
 **
 **	Written: 11/3/98 - JSF
 **/

/*
Copyright (C) 1998  Jeffrey S. Freedman

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <iostream.h>			/* Debugging. */
#include <stdlib.h>
#include <string.h>
#include "gamewin.h"
#include "imagewin.h"
#include "usecode.h"
#include "actions.h"
#include "ready.h"

Frames_sequence *Actor::frames[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/*
 *	Create character.
 */

Actor::Actor
	(
	char *nm, 
	int shapenum, 
	int num,			// NPC # from npc.dat.
	int uc				// Usecode #.
	) : Sprite(shapenum), npc_num(num), party_id(-1),
	    usecode(uc), flags(0), action(0), usecode_dir(0), two_handed(0)
	{
	if (!frames[(int) north])
		set_default_frames();
	name = nm == 0 ? 0 : strdup(nm);
	for (int i = 0; i < sizeof(properties)/sizeof(properties[0]); i++)
		properties[i] = 0;
	for (int i = 0; i < sizeof(spots)/sizeof(spots[0]); i++)
		spots[i] = 0;
	}

/*
 *	Delete.
 */

Actor::~Actor
	(
	)
	{
	delete name;
	delete action;
	}

/*
 *	Set default set of frames.
 */

void Actor::set_default_frames
	(
	)
	{
					// Set up actor's frame lists.
					// These are rough guesses.
	static unsigned char 	north_frames[3] = {0, 1, 2},
				south_frames[3] = {16, 17, 18},
				east_frames[3] = {48, 49, 50},
				west_frames[3] = {32, 33, 34};
	frames[(int) north] = new Frames_sequence(3, north_frames);
	frames[(int) south] = new Frames_sequence(3, south_frames);
	frames[(int) east] = new Frames_sequence(3, east_frames);
	frames[(int) west] = new Frames_sequence(3, west_frames);
	}

/*
 *	Set new action.
 */

void Actor::set_action
	(
	Actor_action *newact
	)
	{
	delete action;
					// Clear us from queue.
	Game_window::get_game_window()->get_tqueue()->remove(this);
	action = newact;
	}

/*
 *	Walk towards a given tile.
 */

void Actor::walk_to_tile
	(
	int tx, int ty, int tz,		// Tile.  (Tz is the lift.)
	int speed,			// Time between frames (msecs).
	int delay			// Delay before starting (msecs).
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int liftpixels = tz*4;
	if (!is_walking())
		set_action(new Walking_actor_action());
	start(((unsigned long) tx + 1)*tilesize - liftpixels,
	      ((unsigned long) ty + 1)*tilesize - liftpixels, speed, delay);
	}

/*
 *	Walk to destination point.
 */

void Actor::walk_to_point
	(
					// Point in world:
	unsigned long destx, unsigned long desty,
	int speed			// Delay between frames.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (!is_walking())
		set_action(new Walking_actor_action());
	start(destx, desty, speed, 0);
	}

/*
 *	Find the best spot where an item may be readied.
 *
 *	Output:	Index, or -1 if none found.
 */

int Actor::find_best_spot
	(
	Game_object *obj
	)
	{
	Shape_info& info = 
		Game_window::get_game_window()->get_shapes().get_info(
							obj->get_shapenum());
	if (info.get_shape_class() == Shape_info::container)
		return !spots[back] ? back : free_hand();
	Ready_type type = (Ready_type) info.get_ready_type();
	switch (type)
		{
	case spell:
	case other_spell:
	case one_handed_weapon:
	case tongs:
		return free_hand();
	case neck_armor:
		return !spots[neck] ? neck : free_hand();
	case torso_armor:
		return !spots[torso] ? torso : free_hand();
	case ring:
		return !spots[lfinger] ? lfinger : 
			(!spots[rfinger] ? rfinger : free_hand());
	case ammunition:		// ++++++++Check U7.
		return free_hand();
	case head_armor:
		return !spots[head] ? head : free_hand();
	case leg_armor:
		return !spots[legs] ? legs : free_hand();
	case foot_armor:
		return !spots[feet] ? feet : free_hand();
	case two_handed_weapon:
		return (!spots[lhand] && !spots[rhand]) ? lrhand : -1;
	case gloves:
		return !spots[arms] ? arms : free_hand();
					// ++++++What about belt?
	case other:
	default:
		return free_hand();
		}
	}

/*
 *	Render.
 */

void Actor::paint
	(
	Game_window *gwin
	)
	{
	if (!(flags & (1L << dont_render)))
		Sprite::paint(gwin);
	}

/*
 *	Run usecode when double-clicked.
 */

void Actor::activate
	(
	Usecode_machine *umachine
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// In gump mode?  Or Avatar?
	if (!npc_num)			// Avatar?
		gwin->show_gump(this, ACTOR_FIRST_GUMP);// ++++58 if female.
	else if (gwin->get_mode() == Game_window::gump)
		{			// Show companions' pictures.
					// +++++Check for companions.
		if (npc_num <= 10)
			gwin->show_gump(this, ACTOR_FIRST_GUMP + 1 + npc_num);
		}
	else if (usecode == -1)
		umachine->call_usecode(get_shapenum(), this,
				Usecode_machine::double_click);
	else
		umachine->call_usecode(usecode, this, 
					Usecode_machine::double_click);
	}

/*
 *	Get name.
 */

char *Actor::get_name
	(
	)
	{
	return name ? name : Game_object::get_name();
	}

/*
 *	Set flag.
 */

void Actor::set_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		flags |= ((unsigned long) 1 << flag);
	}

/*
 *	Clear flag.
 */

void Actor::clear_flag
	(
	int flag
	)
	{
	if (flag >= 0 && flag < 32)
		flags &= ~((unsigned long) 1 << flag);
	}

/*
 *	Get flag.
 */

int Actor::get_flag
	(
	int flag
	)
	{
	return (flag >= 0 && flag < 32) ? (flags & ((unsigned long) 1 << flag))
			!= 0 : 0;
	}

/*
 *	Remove an object.
 */

void Actor::remove
	(
	Game_object *obj
	)
	{
	Container_game_object::remove(obj);
	int index = Actor::find_readied(obj);	// Remove from spot.
	if (index >= 0)
		{
		spots[index] = 0;
		if (index == rhand || index == lhand)
			two_handed = 0;
		}
	}

/*
 *	Add an object.
 *
 *	Output:	1, meaning object is completely contained in this,
 *		0 if not enough space.
 */

int Actor::add
	(
	Game_object *obj,
	int dont_check			// 1 to skip volume check.
	)
	{
	int index = find_best_spot(obj);// Where should it go?
	if (index < 0)			// No free spot?  Look for a bag.
		{
		if (spots[back] && spots[back]->drop(obj))
			return (1);
		if (spots[lhand] && spots[lhand]->drop(obj))
			return (1);
		if (spots[rhand] && spots[rhand]->drop(obj))
			return (1);
		return (0);
		}
					// Add to ourself.
	if (!Container_game_object::add(obj, dont_check))
		return (0);
	if (index == lrhand)		// Two-handed?
		{
		two_handed = 1;
		index = rhand;
		}
	spots[index] = obj;		// Store in correct spot.
	return (1);
	}

/*
 *	Add to given spot.
 *
 *	Output:	1 if successful, else 0.
 */

int Actor::add_readied
	(
	Game_object *obj,
	int index			// Spot #.
	)
	{
	if (index < 0 || index >= sizeof(spots)/sizeof(spots[0]))
		return (0);		// Out of range.
	if (spots[index])		// Already something there?
					// Try to drop into it.
		return (spots[index]->drop(obj));
					// Get best place it should go.
	int best_index = find_best_spot(obj);
	if (best_index == -1)		// No place?
		return (0);
	if (index == best_index || (!two_handed &&
			(index == lhand || index == rhand)))
		{			// Okay.
		if (!Container_game_object::add(obj))
			return (0);	// No room, or too heavy.
		spots[index] = obj;
		if (best_index == lrhand)
			two_handed = 1;	// Must be a two-handed weapon.
		return (1);
		}
	return (0);
	}

/*
 *	Find index of spot where an object is readied.
 *
 *	Output:	Index, or -1 if not found.
 */

int Actor::find_readied
	(
	Game_object *obj
	)
	{
	for (int i = 0; i < sizeof(spots)/sizeof(spots[0]); i++)
		if (spots[i] == obj)
			return (i);
	return (-1);
	}

/*
 *	Handle a time event (for animation).
 */

void Main_actor::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	if (action)			// Doing anything?
		{			// Do what we should.
		int delay = action->handle_event(this);
		if (delay)		// Keep going with same action.
			gwin->get_tqueue()->add(
					curtime + delay, this, udata);
		else
			set_action(0);
		}
					// Not doing an animation?
	if (!gwin->get_usecode()->in_usecode())
		get_followers();	// Get party to follow.
	}

/*
 *	Get the party to follow.
 */

void Main_actor::get_followers
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Usecode_machine *uc = gwin->get_usecode();
	int cnt = uc->get_party_count();
	for (int i = 0; i < cnt; i++)
		{
		Npc_actor *npc = (Npc_actor *) gwin->get_npc(
						uc->get_party_member(i));
		if (npc)
			npc->follow(this);
		}
	}

/*
 *	Walk in current direction.
 *
 *	Output:	Delay for next frame, or 0 to stop.
 *		Dormant is set if off screen.
 */

int Main_actor::walk
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int cx, cy, sx, sy;		// Get chunk, shape within chunk.
	int frame;
	if (next_frame(cx, cy, sx, sy, frame))
		{
		Chunk_object_list *nlist = gwin->get_objects(cx, cy);
		int new_lift;		// Might climb/descend.
					// Just assume height==3.
		if (nlist->is_blocked(3, get_lift(), sx, sy, new_lift))
			{
			stop();
			return (0);
			}
					// Check for scrolling.
		int chunkx = gwin->get_chunkx(), chunky = gwin->get_chunky();
					// At left?
		if (cx - chunkx <= 0 && sx < 6)
			gwin->view_left();
					// At right?
		else if ((cx - chunkx)*16 + sx >= gwin->get_width()/8 - 4)
			gwin->view_right();
					// At top?
		if (cy - chunky <= 0 && sy < 6)
			gwin->view_up();
					// At bottom?
		else if ((cy - chunky)*16 + sy >= gwin->get_height()/8 - 4)
			gwin->view_down();
					// Get old rectangle.
		Rectangle oldrect = gwin->get_shape_rect(this);
					// Get old chunk.
		Chunk_object_list *olist = gwin->get_objects(
						get_cx(), get_cy());
					// Move it.
		move(olist, cx, cy, nlist, sx, sy, frame, new_lift);
					// Near an egg?
		nlist->activate_eggs(sx, sy);
		int inside;		// See if moved inside/outside.
					// In a new chunk?
		if (olist != nlist)
			{
			switched_chunks(olist, nlist);
			if (gwin->check_main_actor_inside())
				gwin->paint();
			}
		else
			gwin->repaint_sprite(this, oldrect);
		if (at_destination())
			{
			stop();
			return (0);
			}
		return (frame_time);	// Add back to queue for next time.
		}
	return (0);			// Done.
	}

/*
 *	Step onto an adjacent tile.
 *	+++++++++++This should be used by walk.
 *
 *	Output:	Delay for next frame, or 0 to stop.
 *		Dormant is set if off screen.
 */

int Main_actor::step
	(
	Tile_coord t,			// Tile to step onto.
	int frame			// New frame #.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Get chunk.
	int cx = t.tx/tiles_per_chunk, cy = t.ty/tiles_per_chunk;
					// Get rel. tile coords.
	int tx = t.tx%tiles_per_chunk, ty = t.ty%tiles_per_chunk;
	Chunk_object_list *nlist = gwin->get_objects(cx, cy);
	int new_lift;			// Might climb/descend.
					// Just assume height==3.
	if (nlist->is_blocked(3, get_lift(), tx, ty, new_lift))
		{
		stop();
		return (0);
		}
					// Check for scrolling.
	int chunkx = gwin->get_chunkx(), chunky = gwin->get_chunky();
					// At left?
	if (cx - chunkx <= 0 && tx < 6)
		gwin->view_left();
					// At right?
	else if ((cx - chunkx)*16 + tx >= gwin->get_width()/8 - 4)
		gwin->view_right();
					// At top?
	if (cy - chunky <= 0 && ty < 6)
		gwin->view_up();
					// At bottom?
	else if ((cy - chunky)*16 + ty >= gwin->get_height()/8 - 4)
		gwin->view_down();
					// Get old rectangle.
	Rectangle oldrect = gwin->get_shape_rect(this);
					// Get old chunk.
	Chunk_object_list *olist = gwin->get_objects(get_cx(), get_cy());
					// Move it.
	move(olist, cx, cy, nlist, tx, ty, frame, new_lift);
					// Near an egg?
	nlist->activate_eggs(tx, ty);
	int inside;			// See if moved inside/outside.
					// In a new chunk?
	if (olist != nlist)
		{
		switched_chunks(olist, nlist);
		if (gwin->check_main_actor_inside())
			gwin->paint();
		}
	else
		gwin->repaint_sprite(this, oldrect);
	if (at_destination())
		{
		stop();
		return (0);
		}
	return (frame_time);		// Add back to queue for next time.
	}

/*
 *	Setup cache after a change in chunks.
 */

void Main_actor::switched_chunks
	(
	Chunk_object_list *olist,	// Old chunk, or null.
	Chunk_object_list *nlist	// New chunk.
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	int newcx = nlist->get_cx(), newcy = nlist->get_cy();
	int xfrom, xto, yfrom, yto;	// Get range of chunks.
	if (!olist)			// No old?  Use all 9.
		{
		xfrom = newcx > 0 ? newcx - 1 : newcx;
		xto = newcx < num_chunks - 1 ? newcx + 1 : newcx;
		yfrom = newcy > 0 ? newcy - 1 : newcy;
		yto = newcy < num_chunks - 1 ? newcy + 1 : newcy;
		}
	else
		{
		int oldcx = olist->get_cx(), oldcy = olist->get_cy();
		if (newcx == oldcx + 1)
			{
			xfrom = newcx;
			xto = newcx < num_chunks - 1 ? newcx + 1 : newcx;
			}
		else if (newcx == oldcx - 1)
			{
			xfrom = newcx > 0 ? newcx - 1 : newcx;
			xto = newcx;
			}
		else
			{
			xfrom = newcx > 0 ? newcx - 1 : newcx;
			xto = newcx < num_chunks - 1 ? newcx + 1 : newcx;
			}
		if (newcy == oldcy + 1)
			{
			yfrom = newcy;
			yto = newcy < num_chunks - 1 ? newcy + 1 : newcy;
			}
		else if (newcy == oldcy - 1)
			{
			yfrom = newcy > 0 ? newcy - 1 : newcy;
			yto = newcy;
			}
		else
			{
			yfrom = newcy > 0 ? newcy - 1 : newcy;
			yto = newcy < num_chunks - 1 ? newcy + 1 : newcy;
			}
		}
	for (int y = yfrom; y <= yto; y++)
		for (int x = xfrom; x <= xto; x++)
			gwin->get_objects(x, y)->setup_cache();
	}

/*
 *	Create a horizontal pace schedule.
 */

Pace_schedule *Pace_schedule::create_horiz
	(
	Npc_actor *n
	)
	{
	int tx, ty, tz;			// Get his position.
	n->get_abs_tile(tx, ty, tz);
	return (new Pace_schedule(n, Tile_coord(tx - 4, ty, tz),
					Tile_coord(tx + 4, ty, tz)));
	}

/*
 *	Create a vertical pace schedule.
 */

Pace_schedule *Pace_schedule::create_vert
	(
	Npc_actor *n
	)
	{
	int tx, ty, tz;			// Get his position.
	n->get_abs_tile(tx, ty, tz);
	return (new Pace_schedule(n, Tile_coord(tx, ty - 4, tz),
					Tile_coord(tx, ty + 4, tz)));
	}

/*
 *	Schedule change for pacing:
 */

void Pace_schedule::now_what
	(
	)
	{
	which = !which;			// Flip direction.
					// Wait .75 sec. before moving.
	npc->walk_to_tile(which ? p1 : p0, 250, 750);
	}

/*
 *	Schedule change for patroling:
 */

void Patrol_schedule::now_what
	(
	)
	{
	const int PATH_SHAPE = 607;
	pathnum++;			// Find next path.
					// Already know its location?
	Game_object *path = (Game_object *) paths.get(pathnum);
	if (!path)			// No, so look around.
		{
		Game_window *gwin = Game_window::get_game_window();
		Vector nearby;
		int cnt = npc->find_nearby(nearby, PATH_SHAPE, -359, -359);
		for (int i = 0; i < cnt; i++)
			{
			Game_object *obj = (Game_object *) nearby.get(i);
			int framenum = obj->get_framenum();
					// Cache it.
			paths.put(framenum, obj);
			if (framenum == pathnum)
				{	// Found it.
				path = obj;
				break;
				}
			}
		if (!path)		// Turn back if at end.
			{
			pathnum = 0;
			path = (Game_object *) paths.get(pathnum);
			}
		if (!path)
			{
			cout << "Couldn't find patrol path " << pathnum
				<< " for obj. shape " << npc->get_shapenum()
								<< '\n';
					// Wiggle a bit.
			Tile_coord pos = npc->get_abs_tile_coord();
			Tile_coord delta = Tile_coord(rand()%3 - 1,
					rand()%3 - 1, 0);
			npc->walk_to_tile(pos + delta, 250, 500);
			int pathcnt = paths.get_cnt();
			pathnum = rand()%(pathcnt < 4 ? 4 : pathcnt);
			return;
			}
		}
					//++++Testing.  Works for passion play.
	npc->set_lift(path->get_lift());
					// Delay up to 2 secs.
	npc->walk_to_tile(path->get_abs_tile_coord(), 250, rand()%2000);
	}

/*
 *	Schedule change for 'talk':
 */

void Talk_schedule::now_what
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Usecode_machine *umachine = gwin->get_usecode();
	switch (phase)
		{
	case 0:				// Start by approaching Avatar.
		npc->follow(gwin->get_main_actor());
		phase++;
		return;
	case 1:				// Wait a second.
	case 2:
		{
		int dx = 1 - 2*(rand()%2);
		npc->walk_to_tile(npc->get_abs_tile_coord() +
			Tile_coord(dx, -dx, 0), 200, 300);
					// Wait til conversation is over.
		if (gwin->get_num_faces_on_screen() == 0)
			phase++;
		return;
		}
	case 3:				// Talk.
		npc->activate(gwin->get_usecode());
		gwin->set_mode(Game_window::normal);
		gwin->paint();
		phase++;
		return;
	default:
		break;
		}
	}

/*
 *	Create a loiter schedule.
 */

Loiter_schedule::Loiter_schedule
	(
	Npc_actor *n
	) : Schedule(n), center(n->get_abs_tile_coord())
	{
	}

/*
 *	Schedule change for 'loiter':
 */

void Loiter_schedule::now_what
	(
	)
	{
	const int dist = 12;		// Distance in tiles to roam.
	int newx = center.tx - dist + rand()%(2*dist);
	int newy = center.ty - dist + rand()%(2*dist);
					// Wait a bit.
	npc->walk_to_tile(newx, newy, center.tz, 350, rand()%2000);
	}

/*
 *	Set a schedule.
 */

void Schedule_change::set
	(
	unsigned char *entry		// 4 bytes read from schedule.dat.
	)
	{
	time = entry[0]&7;
	type = entry[0]>>3;
	x = entry[1];
	y = entry[2];
	superchunk = entry[3];
	}

/*
 *	Create NPC.
 */

Npc_actor::Npc_actor
	(
	char *nm, 			// Name.  A copy is made.
	int shapenum, 
	int fshape, 
	int uc
	) : Actor(nm, shapenum, fshape, uc), next(0), nearby(0),
		schedule_type((int) Schedule::loiter), num_schedules(0), 
		schedules(0), schedule(0), dormant(1), alignment(0)
	{
	}

/*
 *	Kill an actor.
 */

Npc_actor::~Npc_actor
	(
	)
	{
	delete schedule;
	delete [] schedules;
	}

/*
 *	Update schedule at a 3-hour time change.
 */

void Npc_actor::update_schedule
	(
	Game_window *gwin,
	int hour3			// 0=midnight, 1=3am, etc.
	)
	{
	if (Npc_actor::get_party_id() >= 0)
		return;			// Skip if a party member.
	for (int i = 0; i < num_schedules; i++)
		if (schedules[i].get_time() == hour3)
			{		// Found entry.
					// Store old chunk list.
			Chunk_object_list *olist = gwin->get_objects(
							get_cx(), get_cy());
			int new_cx, new_cy, tx, ty;
			schedules[i].get_pos(new_cx, new_cy, tx, ty);
#if DEBUG
cout << "Npc " << get_name() << " has new schedule " << schedule << '\n';
#endif
			Chunk_object_list *nlist = 
					gwin->get_objects(new_cx, new_cy);
					// Move it.
			move(olist, new_cx, new_cy, nlist, tx, ty, -1);
			if (nlist != olist)
				switched_chunks(olist, nlist);
			set_schedule_type(schedules[i].get_type());
			return;
			}
	}

/*
 *	Set new schedule.
 */

void Npc_actor::set_schedule_type
	(
	int new_schedule_type
	)
	{
	stop();				// Stop moving.
	schedule_type = new_schedule_type;
	delete schedule;		// Done with the old.
	schedule = 0;
	switch ((Schedule::Schedule_types) schedule_type)
		{
	case Schedule::horiz_pace:
		schedule = Pace_schedule::create_horiz(this);
		break;
	case Schedule::vert_pace:
		schedule = Pace_schedule::create_vert(this);
		break;
	case Schedule::talk:
		schedule = new Talk_schedule(this);
		break;
	case Schedule::loiter:
	case Schedule::hound:		// For now.
	case Schedule::graze:
		schedule = new Loiter_schedule(this);
		break;
	case Schedule::patrol:
		schedule = new Patrol_schedule(this);
		break;
		}
	if (schedule)			// Try to start it.
		{
		dormant = 0;
		schedule->now_what();
		}
	}

Patrol_schedule::~Patrol_schedule()
{}

/*
 *	Render.
 */

void Npc_actor::paint
	(
	Game_window *gwin
	)
	{
	Actor::paint(gwin);		// Draw on screen.
	if (dormant && schedule)	// Resume schedule.
		{
		dormant = 0;		// But clear out old entries first.??
		gwin->get_tqueue()->remove(this);
		schedule->now_what();	// Ask scheduler what to do.
		}
	}

/*
 *	Handle a time event (for animation).
 */

void Npc_actor::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	if (!action)			// Not doing anything?
		dormant = 1;
	else
		{			// Do what we should.
		int delay = action->handle_event(this);
		if (delay)		// Keep going with same action.
			Game_window::get_game_window()->get_tqueue()->add(
					curtime + delay, this, udata);
		else
			{
			set_action(0);
			if (!dormant && schedule)
				schedule->now_what();
			}
		}
	}

/*
 *	Walk in current direction.
 *
 *	Output:	Delay for next frame, or 0 to stop.
 *		Dormant is set if off screen.
 */

int Npc_actor::walk
	(
	)
	{
					// Store old chunk.
	int old_cx = get_cx(), old_cy = get_cy();
	Game_window *gwin = Game_window::get_game_window();
	int cx, cy, sx, sy;		// Get chunk, shape within chunk.
	int frame;
	if (next_frame(cx, cy, sx, sy, frame))
		{
					// Get ->new chunk.
		Chunk_object_list *nlist = gwin->get_objects(cx, cy);
		nlist->setup_cache();	// Setup cache if necessary.
		int new_lift;		// Might climb/descend.
					// Just assume height==3.
		if (nlist->is_blocked(3, get_lift(), sx, sy, new_lift))
			{
			stop();
			return (0);	// Done.
			}
		gwin->add_dirty(this);	// Set to repaint old area.
					// Get old chunk.
		Chunk_object_list *olist = gwin->get_objects(old_cx, old_cy);
					// Move it.
		move(olist, cx, cy, nlist, sx, sy, frame, new_lift);
		if (olist != nlist)	// In new chunk?
			switched_chunks(olist, nlist);
		if (at_destination())
			{
			stop();
			return (0);
			}
		if (!gwin->add_dirty(this))
			{		// No longer on screen.
			stop();
			dormant = 1;
			return (0);
			}
		return (frame_time);	// Add back to queue for next time.
		}
	dormant = 1;			// Not moving.
	return (0);
	}

/*
 *	Want one value to approach another.
 */

inline int Approach
	(
	int from,
	int to,
	int dist			// Desired distance.
	)
	{
	if (from <= to)			// Going forwards?
		return (to - from <= dist ? from : to - dist);
	else				// Going backwards.
		return (from - to <= dist ? from : to + dist);
	}

/*
 *	Follow the leader.
 */

void Npc_actor::follow
	(
	Actor *leader
	)
	{
					// How close to aim for.
	int dist = 2 + Npc_actor::get_party_id()/3;
	Tile_coord goal = leader->get_abs_tile_coord();
	Tile_coord pos = get_abs_tile_coord();
	int goaldist = goal.distance(pos);	// Tiles to goal.
	if (goaldist < dist)		// Already close enough?
		return;
					// Figure where to aim.
	int newtx = Approach(pos.tx, goal.tx, dist);
	int newty = Approach(pos.ty, goal.ty, dist);
	newtx += 1 - rand()%3;		// Jiggle a bit.
	newty += 1 - rand()%3;
					// Get his speed.
	int speed = leader->get_frame_time();
	if (!speed)			// Not moving?
		speed = 125;
	speed += 10 - rand()%50;	// Let's try varying it a bit.
#if 1
	if (goaldist > 32 &&		// Getting kind of far away?
	    get_party_id() >= 0)	// And a member of the party.
		{			// Teleport.
		int pixels = goaldist*tilesize;
		Game_window *gwin = Game_window::get_game_window();
		if (pixels > gwin->get_width() + 16)
			{
			Chunk_object_list *oldchunk = gwin->get_objects(
				get_cx(), get_cy());
			move(newtx, newty, goal.tz);
			Chunk_object_list *newchunk = gwin->get_objects(
				get_cx(), get_cy());
			if (oldchunk != newchunk)
				switched_chunks(oldchunk, newchunk);
			Rectangle box = gwin->get_shape_rect(this);
			gwin->add_text("Thou shan't lose me so easily!", 
							box.x, box.y);
			gwin->paint();
			return;
			}
		}
#endif
	walk_to_tile(newtx, newty, goal.tz, speed);
	}

/*
 *	Update chunks' npc lists after this has moved.
 */

void Npc_actor::switched_chunks
	(
	Chunk_object_list *olist,	// Old chunk, or null.
	Chunk_object_list *nlist	// New chunk, or null.
	)
	{
	Npc_actor *each;
	if (olist)			// Remove from old list.
		{
		if (this == olist->npcs)
			olist->npcs = next;
		else
			{
			Npc_actor *each, *prev = olist->npcs;
			while ((each = prev->next) != 0)
				if (each == this)
					{
					prev->next = next;
					break;
					}
				else
					prev = each;
			}
		}
	if (nlist)			// Add to new list.
		{
		next = nlist->npcs;
		nlist->npcs = this;
		}
	}

/*
 *	See if it's blocked when trying to move to a new tile.
 *	And don't allow climbing/descending, at least for now.
 *
 *	Output: 1 if so, else 0.
 */

int Monster_actor::is_blocked
	(
	int destx, int desty		// Square we want to move to.
	)
	{
	int tx, ty, lift;		// Get current position.
	get_abs_tile(tx, ty, lift);
	Game_window *gwin = Game_window::get_game_window();
	Shape_info& info = gwin->get_shapes().get_info(get_shapenum());
					// Get dim. in tiles.
	int xtiles = info.get_3d_xtiles(), ytiles = info.get_3d_ytiles();
	int height = info.get_3d_height();
	int vertx0, vertx1;		// Get x-coords. of vert. block
					//   to right/left.
	int horizx0, horizx1;		// Get x-coords of horiz. block
					//   above/below.
	int verty0, verty1;		// Get y-coords of horiz. block
					//   above/below.
	int horizy0, horizy1;		// Get y-coords of vert. block
					//   to right/left.
	horizx0 = destx + 1 - xtiles;
	horizx1 = destx;
	if (destx >= tx)		// Moving right?
		{
		vertx0 = tx + 1;	// Start to right of hot spot.
		vertx1 = destx;		// End at dest.
		}
	else				// Moving left?
		{
		vertx0 = destx + 1 - xtiles;
		vertx1 = tx - xtiles;
		}
	verty0 = desty + 1 - ytiles;
	verty1 = desty;
	if (desty >= ty)		// Moving down?
		{
		horizy0 = ty + 1;	// Start below hot spot.
		horizy1 = desty;	// End at dest.
		verty1--;		// Includes bottom of vert. area.
		}
	else				// Moving up?
		{
		horizy0 = desty + 1 - ytiles;
		horizy1 = ty - ytiles;
		verty0++;		// Includes top of vert. area.
		}
	int x, y;			// Go through horiz. part.
	for (y = horizy0; y <= horizy1; y++)
		{			// Get y chunk, tile-in-chunk.
		int cy = y/tiles_per_chunk, rty = y%tiles_per_chunk;
		for (x = horizx0; x <= horizx1; x++)
			{
			int new_lift;
			Chunk_object_list *olist = gwin->get_objects(
					x/tiles_per_chunk, cy);
			if (olist->is_blocked(height, lift, x%tiles_per_chunk,
							rty, new_lift) ||
			    new_lift != lift)
				return (1);
			}
		}
					// Do vert. block.
	for (x = vertx0; x <= vertx1; x++)
		{			// Get x chunk, tile-in-chunk.
		int cx = x/tiles_per_chunk, rtx = x%tiles_per_chunk;
		for (y = verty0; y <= verty1; y++)
			{
			int new_lift;
			Chunk_object_list *olist = gwin->get_objects(
					cx, y/tiles_per_chunk);
			if (olist->is_blocked(height, lift, rtx,
					y%tiles_per_chunk, new_lift) ||
			    new_lift != lift)
				return (1);
			}
		}
	return (0);			// All clear.
	}

/*
 *	Walk in current direction.
 *
 *	Output:	Delay for next frame, or 0 to stop.
 *		Dormant is set if off screen.
 */

int Monster_actor::walk
	(
	)
	{
					// Store old chunk.
	int old_cx = get_cx(), old_cy = get_cy();
	Game_window *gwin = Game_window::get_game_window();
	int cx, cy, sx, sy;		// Get chunk, shape within chunk.
	int frame;
	if (next_frame(cx, cy, sx, sy, frame))
		{
					// Get ->new chunk.
		Chunk_object_list *nlist = gwin->get_objects(cx, cy);
		nlist->setup_cache();	// Setup cache if necessary.
					// Blocked.
		if (is_blocked(cx*tiles_per_chunk + sx,
						cy*tiles_per_chunk + sy))
			{
			stop();
			return (0);	// Done.
			}
		gwin->add_dirty(this);	// Set to repaint old area.
					// Get old chunk.
		Chunk_object_list *olist = gwin->get_objects(old_cx, old_cy);
					// Move it.
		move(olist, cx, cy, nlist, sx, sy, frame, -1);
		if (olist != nlist)	// In new chunk?
			switched_chunks(olist, nlist);
		if (at_destination())
			{
			stop();
			return (0);
			}
		if (!gwin->add_dirty(this))
			{		// No longer on screen.
			stop();
			dormant = 1;
			return (0);
			}
		return (frame_time);	// Add back to queue for next time.
		}
	dormant = 1;			// Not moving.
	return (0);
	}

/*
 *	Create an instance of a monster.
 */

Npc_actor *Monster_info::create
	(
	int chunkx, int chunky,		// Chunk to place it in.
	int tilex, int tiley,		// Tile within chunk.
	int lift			// Lift.
	)
	{
#if 1	/* ++++This is what we need to try. */
	Monster_actor *monster = new Monster_actor(0, shapenum);
#else
	Npc_actor *monster = new Npc_actor(0, shapenum);
#endif
	monster->set_property(Actor::strength, strength);
	monster->set_property(Actor::dexterity, dexterity);
	monster->set_property(Actor::intelligence, intelligence);
	monster->set_property(Actor::combat, combat);
					// ++++Armor?
					// Place in world.
	Game_window *gwin = Game_window::get_game_window();
	Chunk_object_list *olist = gwin->get_objects(chunkx, chunky);
	monster->move(0, chunkx, chunky, olist, tilex, tiley, 0, lift);
					// Put in chunk's NPC list.
	monster->switched_chunks(0, olist);
					// ++++++For now:
	monster->set_schedule_type(Schedule::loiter);
	return (monster);
	}

#if 0
/*
 *	Figure where the sprite will be in the next frame.
 *
 *	Output:	0 if don't need to move.
 */

int Area_actor::next_frame
	(
	unsigned long time,		// Current time.
	int& new_cx, int& new_cy,	// New chunk coords. returned.
	int& new_sx, int& new_sy,	// New shape (tile) coords. returned.
	int& next_frame			// Next frame # returned.
	)
	{
					// See if we should change motion.
	if (time < next_change)
		return (Actor::next_frame(new_cx, new_cy,
						new_sx, new_sy, next_frame));
	if (is_walking())
		{
		stop();
		new_cx = get_cx();
		new_cy = get_cy();
		new_sx = get_tx();
		new_sy = get_ty();
		next_frame = get_framenum();
					// Wait 0 to 5 seconds.
		next_change.tv_sec = time.tv_sec +
			(long) (5.0*rand()/(RAND_MAX + 1.0));
		return (1);
		}
	else
		{			// Where should we aim for?
		long deltax = 64 - 
			(1 + (long) (128.0*rand()/(RAND_MAX + 1.0)));
		long deltay = 64 - 
			(1 + (long) (128.0*rand()/(RAND_MAX + 1.0)));
		long newx = get_worldx() + deltax;
		long newy = get_worldy() + deltay;
					// Move every 1/4 second.
		start(newx, newy, 250);
					// Go 1 to 4 seconds.
		next_change.tv_sec = time.tv_sec + 1 +
			(long) (4.0*rand()/(RAND_MAX + 1.0));
		return (0);
		}
	}
#endif
