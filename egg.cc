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
#include "Audio.h"
#include "usecode.h"

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
	) : Game_object(l, h, shapex, shapey, lft),
	    probability(prob), data1(d1), data2(d2)
	{
	type = itype&0xf;
	criteria = (itype & (7<<4)) >> 4;
	distance = ((itype >> 10) - 1) & 0x1f;
	distance++;			// I think this is right.
	unsigned char noct = (itype >> 7) & 1;
	unsigned char do_once = (itype >> 8) & 1;
	unsigned char htch = (itype >> 9) & 1;
	unsigned char ar = (itype >> 15) & 1;
	flags = (noct << nocturnal) + (do_once << once) +
			(htch << hatched) + (ar << auto_reset);
	if (type == usecode || type == teleport || type == path)
		set_quality(data1&0xff);
	}

static	inline int	distance_between_points(int ax,int ay,int az,int bx,int by,int bz)
{
	int	dx(abs(ax-bx)),dy(abs(ay-by)),dz(abs(az-bz));
	return	(int)sqrt(dx*dx+dy*dy+dz*dz);
}

static	inline int	distance_between_points(int ax,int ay,int bx,int by)
{
	int	dx(abs(ax-bx)),dy(abs(ay-by));
	return	(int)sqrt(dx*dx+dy*dy);
}


/*
 *	Is a given tile within this egg's influence?
 */

int Egg_object::within_distance
	(
	int abs_tx, int abs_ty		// Tile coords. within entire world.
	) const
	{
	int egg_tx = ((int) cx)*tiles_per_chunk + get_tx();
	int egg_ty = ((int) cy)*tiles_per_chunk + get_ty();
	if (criteria == avatar_footpad)	// Must step on it?
		{
		Game_window *gwin = Game_window::get_game_window();
		Shape_info& info = gwin->get_info(this);
		return (abs_tx <= egg_tx && 
		        abs_tx > egg_tx - info.get_3d_xtiles() &&
		        abs_ty <= egg_ty &&
			abs_ty > egg_ty - info.get_3d_ytiles());
		}
	return distance_between_points(abs_tx,abs_ty,egg_tx,egg_ty)<=distance;
	}

/*
 *	Paint at given spot in world.
 */

void Egg_object::paint
	(
	Game_window *gwin
	)
	{
	if(gwin->paint_eggs)
		Game_object::paint(gwin);
	}

/*
 *	Run usecode when double-clicked or when activated by proximity.
 */

void Egg_object::activate
	(
	Usecode_machine *umachine
	)
	{
#if DEBUG
cout << "Egg type is " << (int) type << ", prob = " << (int) probability <<
		", distance = " << (int) distance << ", crit = " <<
		(int) criteria << ", once = " <<
	((flags & (1<<(int)once) != 0)) << ", hatched = " <<
	((flags & (1<<(int)hatched) != 0)) <<
	", areset = " <<
	((flags & (1<<(int)auto_reset) != 0)) << ", data1 = " << data1
		<< ", data2 = " << data2 << '\n';
#endif
	int roll = 1 + rand()%100;
	if (roll > probability)
		return;			// Out of luck.
	flags |= (1 << (int) hatched);	// Flag it as done.
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
		case monster:
			{
			Monster_info *inf = gwin->get_monster_info(data2&1023);
			if (inf)
				{
				Npc_actor *monster = inf->create(get_cx(),
					get_cy(), get_tx(), get_ty(),
								get_lift());
				monster->set_alignment(data1&3);
				gwin->add_dirty(monster);
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
		default:
			cout << "Egg not actioned" << endl;
                }

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

