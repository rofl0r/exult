/*
Copyright (C) 2000 The Exult Team

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "SDL_events.h"
#include "game.h"
#include "gamewin.h"
#include "Gump_button.h"
#include "gump_utils.h"
#include "misc_buttons.h"
#include "Slider_gump.h"


using std::cout;
using std::endl;

/*
 *	Statics:
 */

short Slider_gump::leftbtnx = 31;
short Slider_gump::rightbtnx = 103;
short Slider_gump::btny = 14;
short Slider_gump::diamondy = 6;
short Slider_gump::xmin = 35;	// Min., max. positions of diamond.
short Slider_gump::xmax = 93;

/*
 *	One of the two arrow button on the slider:
 */
class Slider_button : public Gump_button
{
public:
	Slider_button(Gump *par, int px, int py, int shapenum)
		: Gump_button(par, shapenum, px, py)
		{  }
					// What to do when 'clicked':
	virtual void activate(Game_window *gwin);
};

/*
 *	Handle click on a slider's arrow.
 */

void Slider_button::activate
	(
	Game_window *gwin
	)
{
	((Slider_gump *) parent)->clicked_arrow(this);
}

/*
 *	Set slider value.
 */

void Slider_gump::set_val
	(
	int newval
	)
{
	val = newval;
	static int xdist = xmax - xmin;
	if(max_val-min_val==0)
	{
		val=0;
		diamondx=xmin;
	}
	else
		diamondx = xmin + ((val - min_val)*xdist)/(max_val - min_val);
}

/*
 *	Create a slider.
 */

Slider_gump::Slider_gump
	(
	int mival, int mxval,		// Value range.
	int step,			// Amt. to change by.
	int defval			// Default value.
	) : Modal_gump(0, game->get_shape("gumps/slider")),
	    min_val(mival), max_val(mxval), step_val(step),
	    val(defval), dragging(0), prev_dragx(0)
{
#ifdef DEBUG
	cout << "Slider:  " << min_val << " to " << max_val << " by " << step << endl;
#endif
	left_arrow = new Slider_button(this, leftbtnx, btny, 
				       game->get_shape("gumps/slider_left"));
	right_arrow = new Slider_button(this, rightbtnx, btny, 
					game->get_shape("gumps/slider_right"));
					// Init. to middle value.
	if (defval < min_val)
	  defval = min_val;
	else if (defval > max_val)
	  defval = max_val;
	set_val(defval);
}

/*
 *	Delete slider.
 */

Slider_gump::~Slider_gump
	(
	)
{
	delete left_arrow;
	delete right_arrow;
}

/*
 *	An arrow on the slider was clicked.
 */

void Slider_gump::clicked_arrow
	(
	Slider_button *arrow	// What was clicked.
	)
{
	int newval = val;
	if (arrow == left_arrow)
	{
		newval -= step_val;
		if (newval < min_val)
			newval = min_val;
	}
	else if (arrow == right_arrow)
	{
		newval += step_val;
		if (newval > max_val)
			newval = max_val;
	}
	set_val(newval);
	Game_window *gwin = Game_window::get_game_window();
	paint(gwin);
	gwin->set_painted();
}

/*
 *	Paint on screen.
 */

void Slider_gump::paint
	(
	Game_window *gwin
	)
{
	const int textx = 128, texty = 7;
					// Paint the gump itself.
	gwin->paint_gump(x, y, get_shapenum(), get_framenum());
					// Paint red "checkmark".
	paint_button(gwin, check_button);
					// Paint buttons.
	paint_button(gwin, left_arrow);
	paint_button(gwin, right_arrow);
					// Paint slider diamond.
	gwin->paint_gump(x + diamondx, y + diamondy, game->get_shape("gumps/slider_diamond"), 0);
					// Print value.
  	Paint_num(gwin, val, x + textx, y + texty);
	gwin->set_painted();
}

/*
 *	Handle mouse-down events.
 */

void Slider_gump::mouse_down
	(
	int mx, int my			// Position in window.
	)
{
	dragging = 0;
	Game_window *gwin = Game_window::get_game_window();
	Gump_button *btn = Gump::on_button(gwin, mx, my);
	if (btn)
		pushed = btn;
	else if (left_arrow->on_button(gwin, mx, my))
		pushed = left_arrow;
	else if (right_arrow->on_button(gwin, mx, my))
		pushed = right_arrow;
	else
		pushed = 0;
	if (pushed)
	{
		pushed->push(gwin);
		return;
	}
					// See if on diamond.
	Shape_frame *diamond = gwin->get_gump_shape(game->get_shape("gumps/slider_diamond"), 0);
	if (diamond->has_point(mx - (x + diamondx), my - (y + diamondy)))
	{			// Start to drag it.
		dragging = 1;
		prev_dragx = mx;
	} else {
		if(my-get_y()<diamondy || my-get_y()>diamondy+diamond->get_height())
			return;
		diamondx = mx-get_x();
		if(diamondx<xmin)
			diamondx = xmin;
		if(diamondx>xmax)
			diamondx = xmax;
		static int xdist = xmax - xmin;
		int delta = (diamondx - xmin)*(max_val - min_val)/xdist;
					// Round down to nearest step.
		delta -= delta%step_val;
		int newval = min_val + delta;
		if (newval != val)		// Set value.
			val = newval;
		paint(Game_window::get_game_window());
	}
}

/*
 *	Handle mouse-up events.
 */

void Slider_gump::mouse_up
	(
	int mx, int my			// Position in window.
	)
{
	Game_window *gwin = Game_window::get_game_window();
	if (dragging)			// Done dragging?
	{
		set_val(val);		// Set diamond in correct pos.
		paint(gwin);
		gwin->set_painted();
		dragging = 0;
	}
	if (!pushed)
		return;
	pushed->unpush(gwin);
	if (pushed->on_button(gwin, mx, my))
		pushed->activate(gwin);
	pushed = 0;
}

/*
 *	Mouse was dragged with left button down.
 */

void Slider_gump::mouse_drag
	(
	int mx, int my			// Where mouse is.
	)
{
	if (!dragging)
		return;
	diamondx += mx - prev_dragx;
	prev_dragx = mx;
	if (diamondx < xmin)		// Stop at ends.
		diamondx = xmin;
	else if (diamondx > xmax)
		diamondx = xmax;
	static int xdist = xmax - xmin;
	int delta = (diamondx - xmin)*(max_val - min_val)/xdist;
					// Round down to nearest step.
	delta -= delta%step_val;
	int newval = min_val + delta;
	if (newval != val)		// Set value.
		val = newval;
	paint(Game_window::get_game_window());
}

/*
 *	Handle ASCII character typed.
 */

void Slider_gump::key_down
	(
	int chr
	)
{
	switch(chr) {
	case SDLK_RETURN:
		done = 1;
		break;
	case SDLK_LEFT:
		clicked_arrow(left_arrow);
		break;
	case SDLK_RIGHT:
		clicked_arrow(right_arrow);
		break;
	} 
}
