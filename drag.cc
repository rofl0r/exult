/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Drag.cc - Dragging objects in Game_window.
 **
 **	Written: 3/2/2000 - JSF
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

#include <iostream.h>	/* Debugging */
#include "gamewin.h"
#include "gumps.h"

/*
 *	Begin a possible drag when the mouse button is depressed.
 *
 *	Output:	1 if object selected for dragging, else 0.
 */

int Game_window::start_dragging
	(
	int x, int y			// Position in window.
	)
	{
	dragging = 0;
	dragging_gump = 0;
	dragging_mousex = x;
	dragging_mousey = y;
	dragging_rect = Rectangle(0, 0, 0, 0);
	Game_object *found[100];	// See what was clicked on.
	int cnt;
					// First see if it's a gump.
	dragging_gump = find_gump(x, y);
	if (dragging_gump)
		{
		cnt = dragging_gump->find_objects(this, x, y, found);
		if (cnt)
			{
			dragging = found[cnt - 1];
			dragging_gump->get_shape_location(dragging,
					dragging_paintx, dragging_painty);
			}
		else
			{		// Dragging whole gump.
			dragging_paintx = dragging_gump->get_x();
			dragging_painty = dragging_gump->get_y();
cout << "(x,y) rel. to gump is (" << (x-dragging_paintx) << ", " <<
		(y-dragging_painty) << ")\n";
			}
		}
	else if (!dragging)		// Not found in gump?
		{
		cnt = find_objects(x, y, found);
		if (!cnt)
			return (0);
		dragging = found[cnt - 1];
					// Don't want to move walls.
		if (!dragging->is_dragable())	
			{
			dragging = 0;
			return (0);
			}
					// Get coord. where painted.
		get_shape_location(dragging, dragging_paintx, dragging_painty);
		}
	return (1);
	}

/*
 *	Mouse was moved while dragging.
 */

void Game_window::drag
	(
	int x, int y			// Mouse pos. in window.
	)
	{
	if (!dragging && !dragging_gump)
		return;
	if (dragging_rect.w == 0)
		{			// First motion.
					// Store original pos. on screen.
		dragging_rect = dragging_gump ?
			(dragging ? dragging_gump->get_shape_rect(dragging)
				  : get_gump_rect(dragging_gump))
			: get_shape_rect(dragging);
					// Pad more if dragging gump.
		int pad = dragging ? 8 : 12;
		dragging_rect.x -= pad;	// Make a little bigger.
		dragging_rect.y -= pad;
		dragging_rect.w += 2*pad;
		dragging_rect.h += 2*pad;
					// Remove from actual position.
		if (dragging_gump)
			if (dragging)
				dragging_gump->remove(dragging);
			else
				dragging_gump->remove_from_chain(
							open_gumps);
		else
			get_objects(dragging->get_cx(), 
				dragging->get_cy())->remove(dragging);
		}
	win->set_clip(0, 0, get_width(), get_height());
	Rectangle rect = clip_to_win(dragging_rect);
	paint(rect);			// Paint over last place shown.
	int deltax = x - dragging_mousex, deltay = y - dragging_mousey;
	dragging_mousex = x;
	dragging_mousey = y;
					// Shift to new position.
	dragging_rect.shift(deltax, deltay);
	dragging_paintx += deltax;
	dragging_painty += deltay;
	if (dragging)
		paint_shape(win, dragging_paintx, dragging_painty, 
			dragging->get_shapenum(), dragging->get_framenum());
	else				// Dragging whole gump.
		{
		dragging_gump->set_pos(dragging_paintx, dragging_painty);
		dragging_gump->paint(this);
		}
	win->clear_clip();
	}

/*
 *	Mouse was released, so drop object.
 */

void Game_window::drop_dragged
	(
	int x, int y			// Mouse pos.
	)
	{
	if (!dragging && !dragging_gump)
		return;
	drop(x, y);			// Drop it.
	dragging = 0;
	dragging_gump = 0;
	paint();
	}

/*
 *	Drop at given position.
 */

void Game_window::drop
	(
	int x, int y			// Mouse position.
	)
	{
	drag(x, y);			// Get object to mouse pos.
	if (!dragging)			// Only dragging a gump?
		{
		dragging_gump->append_to_chain(open_gumps);
		return;
		}
					// First see if it's a gump.
	Gump_object *on_gump = find_gump(x, y);
	if (on_gump)
		{
		on_gump->add(dragging);
		return;
		}
	Game_object *found[100];	// Was it dropped on something?
	int cnt = find_objects(x, y, found);
	for (int i = cnt - 1; i >= 0; i--)
		if (found[i]->drop(dragging))
			return;
					// Find where to drop it.
	int max_lift = main_actor->get_lift() + 4;
	int lift;
	for (lift = dragging->get_lift(); lift < max_lift; lift++)
		if (drop_at_lift(lift))
			break;
	if (lift == max_lift)		// Couldn't drop?  Put it back.
		{
		if (dragging_gump)
			dragging_gump->add(dragging);
		else
			get_objects(dragging->get_cx(), 
					dragging->get_cy())->add(dragging);
		}
	}

/*
 *	Try to drop at a given lift.
 *
 *	Output:	1 if successful.
 */

int Game_window::drop_at_lift
	(
	int at_lift
	)
	{
					// Take lift into account, round.
	int x = dragging_paintx + at_lift*4 + tilesize/2;
	int y = dragging_painty + at_lift*4 + tilesize/2;
	int cx = chunkx + x/chunksize;
	int cy = chunky + y/chunksize;
	int tx = (x/tilesize)%tiles_per_chunk - 1;
	int ty = (y/tilesize)%tiles_per_chunk - 1;
	Chunk_object_list *chunk = get_objects(cx, cy);
	chunk->setup_cache();		// Be sure cache is set up.
	int lift;			// Can we put it here?
	if (!chunk->is_blocked(dragging->get_lift(), tx, ty, lift))
		{
		dragging->set_lift(lift);
		dragging->set_shape_pos(tx, ty);
		chunk->add(dragging);
		return (1);
		}
	return (0);
	}
