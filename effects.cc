/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Effects.cc - Special effects.
 **
 **	Written: 5/25/2000 - JSF
 **/

/*
Copyright (C) 2000  Jeffrey S. Freedman

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

#include "gamewin.h"
#include "effects.h"
#include "Zombie.h"

/*
 *	Create an animation from the 'sprites.vga' file.
 */

Sprites_effect::Sprites_effect
	(
	int num,			// Index.
	int px, int py			// Screen location.
	) : sprite_num(num), frame_num(0), tx(px), ty(py)
	{
	Game_window *gwin = Game_window::get_game_window();
	frames = gwin->get_sprite_num_frames(num);
					// Start immediately.
	gwin->get_tqueue()->add(SDL_GetTicks(), this, 0L);
	}

/*
 *	Animation.
 */

void Sprites_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	const int delay = 50;		// Delay between frames.
	Game_window *gwin = Game_window::get_game_window();
	if (frame_num == frames)	// At end?
		{			// Remove & delete this.
		gwin->remove_effect(this);
		gwin->paint();
		return;
		}
	Sprites_effect::paint(gwin);	// Render.
	gwin->set_painted();
	frame_num++;			// Next frame.
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Render.
 */

void Sprites_effect::paint
	(
	Game_window *gwin
	)
	{
	gwin->paint_sprite((tx - gwin->get_scrolltx())*tilesize,
		(ty - gwin->get_scrollty())*tilesize, sprite_num, frame_num);
	}

/*
 *	Create a projectile animation.
 */

Projectile_effect::Projectile_effect
	(
	Game_object *from,		// Start here.
	Game_object *to,		// End here, then run usecode on it.
	int ufun,			// Usecode function to run.
	int shnum			// Shape # in 'shapes.vga'.
	) : usefun(ufun), shape_num(shnum), frame_num(0)
	{
	Game_window *gwin = Game_window::get_game_window();
	frames = gwin->get_shape_num_frames(shnum);
					// Get starting position.
	pos = from->get_abs_tile_coord();
	path = new Zombie();		// Create simple pathfinder.
					// Find path.  Should never fail.
	path->NewPath(pos, to->get_abs_tile_coord(), 0);
					// Start immediately.
	gwin->get_tqueue()->add(SDL_GetTicks(), this, 0L);
	}

/*
 *	Delete.
 */

Projectile_effect::~Projectile_effect
	(
	)
	{
	delete path;
	}

/*
 *	Animation.
 */

void Projectile_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	const int delay = 100;		// Delay between frames.
	Game_window *gwin = Game_window::get_game_window();
	if (frame_num == frames)	// Last frame?
		frame_num = 0;
	if (!path->GetNextStep(pos))	// Get next spot.
		{			// Done? 
		pos.tx = -1;		// Signal we're done.
//++++++++++++ Run usecode.
		gwin->remove_effect(this);
		gwin->paint();
		return;
		}
#if 1
	gwin->paint();		// ++++Experiment.
#else	/* +++++Doesn't clean up prev. */
	Projectile_effect::paint(gwin);	// Render.
	gwin->set_painted();
#endif
	frame_num++;			// Next frame.
					// Add back to queue for next time.
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Render.
 */

void Projectile_effect::paint
	(
	Game_window *gwin
	)
	{
	if (pos.tx == -1)
		return;			// Already at destination.
	int liftpix = pos.tz*tilesize/2;
	gwin->paint_shape((pos.tx - gwin->get_scrolltx())*tilesize - liftpix,
		(pos.ty - gwin->get_scrollty())*tilesize - liftpix, 
		shape_num, frame_num);
	}

/*
 *	Create a text object.
 */

Text_effect::Text_effect
	(
	const char *m, 			// A copy is made.
	int t_x, int t_y, 		// Abs. tile coords.
	int w, int h
	) : msg(strdup(m)), tx(t_x), ty(t_y), width(w), height(h)
	{
	}

/*
 *	Remove from screen.
 */

void Text_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata			// Ignored.
	)
	{
	Game_window *gwin = (Game_window *) udata;
					// Repaint slightly bigger rectangle.
	Rectangle rect((tx - gwin->get_scrolltx() - 1)*tilesize,
		       (ty - gwin->get_scrollty() - 1)*tilesize,
			width + 2*tilesize, height + 2*tilesize);
					// Intersect with screen.
	rect = gwin->clip_to_win(rect);
	gwin->remove_effect(this);	// Remove & delete this.
	if (rect.w > 0 && rect.h > 0)	// Watch for negatives.
		gwin->paint(rect.x, rect.y, rect.w, rect.h);

	}

/*
 *	Render.
 */

void Text_effect::paint
	(
	Game_window *gwin
	)
	{
	const char *ptr = msg;
	if (*ptr == '@')
		ptr++;
	int len = strlen(ptr);
	if (ptr[len - 1] == '@')
		len--;
	gwin->paint_text(0, ptr, len, (tx - gwin->get_scrolltx())*tilesize,
				(ty - gwin->get_scrollty())*tilesize);
	}

/*
 *	Init. a weather effect.
 */

Weather_effect::Weather_effect
	(
	int duration			// Length in game minutes.
	)
	{
	stop_time = SDL_GetTicks() + 1000*((duration*60)/time_factor);
	}

/*
 *	Move raindrop.
 */

inline void Raindrop::next
	(
	Image_window8 *iwin,		// Where to draw.
	Xform_palette xform,		// Transform array.
	int w, int h			// Dims. of window.
	)
	{
	if (x >= 0)			// Not the first time?  Restore pix.
		iwin->put_pixel8(oldpix, x, y);
					// Time to restart?
	if (x < 0 || x == w - 1 || y == h - 3)
		{			
		int r = rand();
					// Have a few fall faster.
		yperx = (r%4) ? 1 : 2;
		int vert = (7*h)/8;	// Top of vert. area.
		int horiz = (7*w)/8;	// Left part of horiz. area.
		int start = r%(vert + horiz);
		if (start < vert)	// Start on left edge.
			{ x = 0; y = h - start; }
		else
			{ x = start - vert; y = 0; };
		}
	else				// Next spot.
		{
		x++;
		y += yperx;
		}
	oldpix = iwin->get_pixel8(x, y);	// Get pixel.
	iwin->put_pixel8(xform[oldpix], x, y);
	}

/*
 *	Rain.
 */

void Rain_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Image_window8 *win = gwin->get_win();
	int w = win->get_width(), h = win->get_height();
					// Get transform table.
	Xform_palette xform = gwin->get_xform(10);	//++++Experiment.
	const int num_drops = sizeof(drops)/sizeof(drops[0]);
					// Move drops.
	for (int i = 0; i < num_drops; i++)
		drops[i].next(win, xform, w, h);
	gwin->set_painted();
	if (curtime < stop_time)	// Keep going?
		gwin->get_tqueue()->add(curtime + 100, this, udata);
	else
		delete this;
	}

/*
 *	Lightning.
 */

void Lightning_effect::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	gwin->set_painted();
	int r = rand();			// Get a random #.
	int delay = 100;		// Delay for next time.
	if (save_brightness > 0)	// Just turned white?  Restore.
		{
		gwin->set_palette(-1, save_brightness);
		if (curtime >= stop_time)
			{		// Time to stop.
			delete this;
			return;
			}
		if (r%4 == 0)		// Occassionally flash again.
			delay = (1 + r%3)*100;
		else			// Otherwise, wait several secs.
			delay = (5000 + r%4);
		}
	else				// Time to flash.
		{
		save_brightness = gwin->get_brightness();
		gwin->set_palette(-1, 200);
		delay = (1 + r%3)*50;
		}
	gwin->get_tqueue()->add(curtime + delay, this, udata);
	}

/*
 *	Shake the screen.
 */

void Earthquake::handle_event
	(
	unsigned long curtime,		// Current time of day.
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Image_window *win = gwin->get_win();
	int w = win->get_width(), h = win->get_height();
	int sx = 0, sy = 0;
	int dx = rand()%9 - 4;
	int dy = rand()%9 - 4;
	if (dx > 0)
		w -= dx;
	else
		{
		w += dx;
		sx -= dx;
		dx = 0;
		}
	if (dy > 0)
		h -= dy;
	else
		{
		h += dy;
		sy -= dy;
		dy = 0;
		}
	win->copy(sx, sy, w, h, dx, dy);
	gwin->set_painted();
	gwin->show();
					// Shake back.
	win->copy(dx, dy, w, h, sx, sy);
	if (++i < len)			// More to do?  Put back in queue.
		gwin->get_tqueue()->add(curtime + 100, this, udata);
	else
		delete this;
	}

