/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objs.cc - Game objects.
 **
 **	Written: 10/1/98 - JSF
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

#include "egg.h"
#include "gamewin.h"
#include "actors.h"
#include "animate.h"
#include "Audio.h"
#include "usecode.h"
#include "useval.h"
#include "effects.h"
#include "game.h"

// #include <math.h>

/*
 *	Paint at given spot in world.
 */

void Egglike_game_object::paint
	(
	Game_window *gwin
	)
	{
	if(gwin->paint_eggs)
		Game_object::paint(gwin);
	}

/*
 *	Can this be clicked on?
 */

int Egglike_game_object::is_findable
	(
	Game_window *gwin
	)
	{
	return gwin->paint_eggs && Ireg_game_object::is_findable(gwin);
	}

/*
 *	Create an egg from IREG data.
 */

Egg_object::Egg_object
	(
	unsigned char l, unsigned char h, 
	unsigned int shapex, unsigned int shapey, 
	unsigned int lft, 
	unsigned short itype,
	unsigned char prob, 
	short d1, short d2
	) : Egglike_game_object(l, h, shapex, shapey, lft),
	    probability(prob), data1(d1), data2(d2),
	    area(Rectangle(0, 0, 0, 0)), monster_created(0)
	{
	type = itype&0xf;
	criteria = (itype & (7<<4)) >> 4;
	distance = (itype >> 10) & 0x1f;
	unsigned char noct = (itype >> 7) & 1;
	unsigned char do_once = (itype >> 8) & 1;
					// Cached_in eggs can be rehatched.
	unsigned char htch = criteria == cached_in ? 0 : ((itype >> 9) & 1);
	solid_area = (criteria == cached_in || criteria == something_on) ? 1 
								: 0;
	unsigned char ar = (itype >> 15) & 1;
	flags = (noct << nocturnal) + (do_once << once) +
			(htch << hatched) + (ar << auto_reset);
	if (type == usecode || type == teleport || type == path)
		set_quality(data1&0xff);
	if (type == path)		// Store paths.
		Game_window::get_game_window()->add_path_egg(this);
	}

/*
 *	Destructor:
 */

Egg_object::~Egg_object
	(
	)
	{
	if (monster_created)
		monster_created->set_creator(0);
	}

/*
 *	Set active area after being added to its chunk.
 */

void Egg_object::set_area
	(
	)
	{
	if (!probability || type == path)// No chance of normal activation?
		{
		area = Rectangle(0, 0, 0, 0);
		return;
		}
	Game_window *gwin = Game_window::get_game_window();
	int tx, ty, tz;			// Get absolute tile coords.
	get_abs_tile(tx, ty, tz);
	switch (criteria)		// Set up active area.
		{
	case cached_in:			// Make it really large.
		area = Rectangle(tx - 32, ty - 32, 64, 64);
		break;
	case avatar_footpad:
	case party_footpad:
		{
		Shape_info& info = gwin->get_info(this);
		int xtiles = info.get_3d_xtiles(), 
		    ytiles = info.get_3d_ytiles();
		area = Rectangle(tx - xtiles + 1, ty - ytiles + 1,
							xtiles, ytiles);
		break;
		}
	case avatar_far:		// Make it 1 tile bigger each dir.
		area = Rectangle(tx - distance - 1, ty - distance - 1, 
					2*distance + 3, 2*distance + 3);
		break;
	default:
		{
		int width = 2*distance;
		if (distance <= 1)	// Small?
			width++;
		area = Rectangle(tx - distance, ty - distance, 
					width, width);
		break;
		}
		}
					// Don't go outside the world.
	Rectangle world(0, 0, num_chunks*tiles_per_chunk,
						num_chunks*tiles_per_chunk);
	area = area.intersect(world);
	}

/*
 *	This is called by the monster that this egg created.
 */

void Egg_object::monster_gone
	(
	)
	{
					// In future, may want to set a time
					//   before 'hatched' is cleared.
	flags &= ~((1 << (int) hatched));
	monster_created = 0;
	}

/*
 *	Is the egg active when stepping onto a given spot, or placing an obj.
 *	on the spot?
 */

int Egg_object::is_active
	(
	Game_object *obj,		// Object placed (or Actor).
	int tx, int ty, int tz,		// Tile stepped onto.
	int from_tx, int from_ty	// Tile stepped from.
	)
	{
	if (flags & (1 << (int) hatched) && //+++++Testing
	    criteria != cached_in)
		return (0);		// For now... Already hatched.
	Game_window *gwin = Game_window::get_game_window();
	if (flags & (1 << (int) nocturnal))
		{			// Nocturnal.
		int hour = gwin->get_hour();
		if (!(hour >= 9 || hour <= 5))
			return (0);	// It's not night.
		}
	Egg_criteria cri = (Egg_criteria) get_criteria();
					// Watch for cached_in eggs which have
					//   been reset.
	if (cri == cached_in && !solid_area)
		cri = avatar_near;
	switch (cri)
		{
	case cached_in:			// Anywhere in square.
		return area.has_point(tx, ty);
	case party_near:
		return (obj->get_party_id() >= 0 || 
					obj == gwin->get_main_actor()) &&
			area.has_point(tx, ty) && tz == get_lift() &&
					!area.has_point(from_tx, from_ty);
	case avatar_near:		// New tile is in, old is out.
		return obj == gwin->get_main_actor() && tz == get_lift() &&
			area.has_point(tx, ty) &&
					!area.has_point(from_tx, from_ty);
	case avatar_far:		// New tile is outside, old is inside.
		{
		if (obj != gwin->get_main_actor() || !area.has_point(tx, ty))
			return (0);
		Rectangle inside(area.x + 1, area.y + 1, 
						area.w - 2, area.h - 2);
		return inside.has_point(from_tx, from_ty) &&
			!inside.has_point(tx, ty);
		}
	case avatar_footpad:
		return obj == gwin->get_main_actor() && tz == get_lift() &&
						area.has_point(tx, ty);
	case party_footpad:
		return area.has_point(tx, ty) && tz == get_lift() &&
			(obj->get_party_id() >= 0 || 
						obj == gwin->get_main_actor());
	case something_on:
		return obj != gwin->get_main_actor() && 
			tz >= get_lift() && tz - get_lift() <= 3 &&
			area.has_point(tx, ty) && obj->get_npc_num() <= 0;
	default:
		return 0;
		}
	}

static	inline int	distance_between_points(int ax,int ay,int az,int bx,int by,int bz)
{
	int	dx(abs(ax-bx)),dy(abs(ay-by)),dz(abs(az-bz));
	return	(int)sqrt(float(dx*dx+dy*dy+dz*dz));	// the cast to float is required to prevent ambiguity!
}

static	inline int	distance_between_points(int ax,int ay,int bx,int by)
{
	int	dx(abs(ax-bx)),dy(abs(ay-by));
	return	(int)sqrt(float(dx*dx+dy*dy));			// the cast to float is required to prevent ambiguity!
}


/*
 *	Run usecode when double-clicked.
 */

void Egg_object::activate
	(
	Usecode_machine *umachine,
	int /* event */
	)
	{
	activate(umachine, 0, 0);
	}

/*
 *	Run usecode when double-clicked or when activated by proximity.
 */

void Egg_object::activate
	(
	Usecode_machine *umachine,
	Game_object *obj,		// Object (actor) that came near it.
	int must			// If 1, skip dice roll.
	)
	{
#if DEBUG
cout << "Egg type is " << (int) type << ", prob = " << (int) probability <<
		", distance = " << (int) distance << ", crit = " <<
		(int) criteria << ", once = " <<
	((flags & (1<<(int)once)) != 0) << ", hatched = " <<
	((flags & (1<<(int)hatched)) != 0) <<
	", areset = " <<
	((flags & (1<<(int)auto_reset)) != 0) << ", data1 = " << data1
		<< ", data2 = " << data2 << '\n';
#endif
	int roll = must ? 0 : 1 + rand()%100;
	if (roll > probability)
		return;			// Out of luck.
	if (monster_created)
		{
		if (!must)
			return;
		monster_created->set_creator(0);
		}
	if ((flags & (1 << (int) auto_reset)) == 0)
					// Flag it as done if not auto-reset.
		flags |= (1 << (int) hatched);
	Game_window *gwin = Game_window::get_game_window();
	switch(type)
		{
		case jukebox:
#if DEBUG
			cout << "Audio parameters might be: " << (data1&0xff) << " and " << ((data1>>8)&0x01) << endl;
#endif
			audio->start_music((data1)&0xff,(data1>>8)&0x01);
			break;
		case voice:
			audio->start_speech((data1)&0xff);
			break;
		case monster:		// +++++Not just for monsters!
			{
			int shnum = data2&1023;
			int frnum = data2>>10;
			Monster_info *inf = gwin->get_monster_info(shnum);
			if (inf)
				{
				Monster_actor *monster = inf->create(get_cx(),
					get_cy(), get_tx(), get_ty(),
								get_lift());
				monster->set_alignment(data1&3);
				monster->set_creator(this);
				monster_created = monster;
				gwin->add_dirty(monster);
				gwin->add_nearby_npc(monster);
				}
			else		// Create item.
				{
				Shape_info& info = gwin->get_info(shnum);
				Game_object *nobj = info.is_animated() ?
					new Animated_ireg_object(
						shnum, frnum, get_tx(),
						get_ty(), get_lift())
					: new Ireg_game_object(
						shnum, frnum, get_tx(),
						get_ty(), get_lift());
				gwin->get_objects(get_cx(), get_cy())->add(
									nobj);
				gwin->add_dirty(nobj);
				nobj->set_flag(okay_to_take);
				}
			break;
			}
		case usecode:
			{		// Data2 is the usecode function.
			Game_window::Game_mode savemode = gwin->get_mode();
			umachine->call_usecode(data2, this,
					Usecode_machine::egg_proximity);
			if (gwin->get_mode() == Game_window::conversation)
				{
				gwin->set_mode(savemode);
				gwin->set_all_dirty();
				}
			break;
			}
		case teleport:
			{
			Tile_coord pos;	// Get position to jump to.
			if (get_quality() == 255)
				{	// Jump to coords.
				int schunk = data1 >> 8;
				pos = Tile_coord((schunk%12)*tiles_per_schunk +
								(data2&0xff), 
					(schunk/12)*tiles_per_schunk +
								(data2>>8), 0);
				}
			else
				{
				Egg_object *path =
					gwin->get_path_egg(get_quality());
				if (!path)
					break;
				pos = path->get_abs_tile_coord();
				}
			cout << "Should teleport to (" << pos.tx << ", " <<
					pos.ty << ')' << endl;
			if (obj && (obj == gwin->get_main_actor() ||
					obj->get_party_id() >= 0))
					// Teleport everyone!!!
				gwin->teleport_party(pos);
					// Can keep doing it.
			flags &= ~((1 << (int) hatched));
			break;
			}
		case weather:
			{
			int len = data1>>8;
			switch (data1&0xff)
				{
			case 0:		// Back to normal.
				gwin->remove_weather_effects();
				break;
			case 2:		// Storm.
				gwin->add_effect(new Storm_effect(len));
				break;
			case 6:		// Clouds.
				gwin->add_effect(new Clouds_effect(len));
				break;
			default:
				break;
				}
			break;
			}
		default:
			cout << "Egg not actioned" << endl;
                }
	if (flags & (1 << (int) once))
		remove_this();		// All done, so go away.
	else if (criteria == cached_in && solid_area && (type != usecode || Game::get_game_type() == SERPENT_ISLE))
		{			// Replace solid area with outline.
		Chunk_object_list *chk = gwin->get_objects(get_cx(), get_cy());
		chk->remove_egg(this);	// Remove from chunk.
		solid_area = 0;		// Clear flag.
		chk->add_egg(this);	// Add back.
		}
	}

/*
 *	Remove an object from the world.
 *	The object is deleted.
 */

void Egg_object::remove_this
	(
	int nodel			// 1 to not delete.
	)
	{
	Chunk_object_list *chunk = 
			Game_window::get_game_window()->get_objects_safely(
								cx, cy);
	if (chunk)
		chunk->remove_egg(this);
	if (!nodel)
		delete this;
	}

/*
 *	Write out.
 */

void Egg_object::write_ireg
	(
	ostream& out
	)
	{
	unsigned char buf[13];		// 13-byte entry + length-byte.
	buf[0] = 12;
	unsigned char *ptr = &buf[1];	// To avoid confusion about offsets.
	write_common_ireg(ptr);		// Fill in bytes 1-4.
	ptr += 4;
	unsigned short tword = type&0xf;// Set up 'type' word.
	tword |= ((criteria&7)<<4);
	tword |= (((flags>>nocturnal)&1)<<7);
	tword |= (((flags>>once)&1)<<8);
	tword |= (((flags>>hatched)&1)<<9);
	tword |= ((distance&0x1f)<<10);
	tword |= (((flags>>auto_reset)&1)<<15);
	Write2(ptr, tword);
	*ptr++ = probability;
	Write2(ptr, data1);
	*ptr++ = (get_lift()&15)<<4;	// Low bits?++++++
	Write2(ptr, data2);
	out.write((char*)buf, sizeof(buf));
	}

/*
 *	Create from IREG data.
 */
Animated_egg_object::Animated_egg_object
	(
	unsigned char l, unsigned char h, 
	unsigned int shapex,
	unsigned int shapey, unsigned int lft, 
	unsigned short itype,
	unsigned char prob, short d1, short d2
	) : Egg_object(l, h, shapex, shapey, lft, itype, prob, d1, d2)
	{ 
	animator = new Frame_animator(this, 1); 
	}

/*
 *	Delete.
 */
Animated_egg_object::~Animated_egg_object()
	{ 
	delete animator; 
	}

/*
 *	Render.
 */

void Animated_egg_object::paint
	(
	Game_window *gwin
	)
	{
	Game_object::paint(gwin);	// Always paint these.
	animator->want_animation();	// Be sure animation is on.
	}

/*
 *	Run usecode when double-clicked or when activated by proximity.
 */

void Animated_egg_object::activate
	(
	Usecode_machine *umachine
	)
	{
	Egg_object::activate(umachine);
	flags &= ~(1 << (int) hatched);	// Moongate:  reset always.
	}

