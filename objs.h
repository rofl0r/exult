/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objs.h - Game objects.
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

#ifndef INCL_OBJS
#define INCL_OBJS	1

#if (defined(DOS) || defined(XWIN))

#else

#define Rectangle RECTX

#endif

#include "lists.h"
#include "usecode.h"
#include "tqueue.h"

class Vga_file;
class Game_window;
class Npc_actor;

/*
 *	Sizes:
 */
const int tilesize = 8;			// A tile (shape) is 8x8 pixels.
const int tiles_per_chunk = 16;		// A chunk is 16x16 tiles.
const int chunksize = 16 * 8;		// A chunk has 16 8x8 shapes.
const int num_chunks = 12*16;		// Total # of chunks in each dir.

/*
 *	A shape ID contains a shape # and a frame # within the shape encoded
 *	as a 2-byte quantity.
 */
class ShapeID
	{
	unsigned char low, high;
public:
	ShapeID(unsigned char l, unsigned char h) : low(l), high(h)
		{  }
	ShapeID(unsigned char *& data)	// Read from buffer & incr. ptr.
		{
		low = *data++;
		high = *data++;
		}
					// Create "end-of-list"/invalid entry.
	ShapeID() : low(0xff), high(0xff)
		{  }
	int is_invalid()		// End-of-list or invalid?
		{ return (low == 0xff && high == 0xff); }
	int is_eol()
		{ return is_invalid(); }
	int get_shapenum()
		{ return low + 256*(high&0x3); }
	int get_framenum()
		{ return (high >> 2) & 0x1f; }
					// Set to given shape.
	void set_shape(int shapenum, int framenum)
		{
		low = shapenum & 0xff;
		high = ((shapenum >> 8) & 3) | (framenum << 2);
		}
	ShapeID(int shapenum, int framenum)
		{ set_shape(shapenum, framenum); }
	void set_shape(int shapenum)	// Set shape, but keep old frame #.
		{
		low = shapenum & 0xff;
		high = ((shapenum >> 8) & 3) | (high & ~3);
		}
	void set_frame(int framenum)	// Set to new frame.
		{ high = (high & 3) | (framenum << 2); }
	};

/*
 *	A game object is a shape from shapes.vga along with info. about its
 *	position within its chunk.
 */
class Game_object : public ShapeID
	{
	unsigned char shape_pos;	// (X,Y) of shape within chunk.
	unsigned char lift;		// Raise by 4* this number.
	short quality;			// Some sort of game attribute.
	Game_object *next;		// ->next in chunk list.
protected:
	unsigned char cx, cy;		// (Absolute) chunk coords.
public:
	friend class Chunk_object_list;
					// Create from ifix record.
	Game_object(unsigned char *ifix)
			: ShapeID(ifix[2], ifix[3]), shape_pos(ifix[0]),
			  lift(ifix[1] & 0xf), quality(0), cx(255), cy(255)
		{  }
					// Create from ireg. data.
	Game_object(unsigned char l, unsigned char h, unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: ShapeID(l, h), shape_pos((shapex << 4) + shapey), 
					lift(lft), quality(0), cx(255), cy(255)
		{  }
	Game_object() : ShapeID()	// Create fake entry.
		{  }
	int get_shape_pos_x()		// Really the tiles.
		{ return (shape_pos >> 4) & 0xf; }
	int get_shape_pos_y()
		{ return shape_pos & 0xf; }
	int get_lift()
		{ return lift; }
	int get_quality()
		{ return quality; }
	void set_quality(int q)
		{ quality = q; }
					// Set shape coord. within chunk.
	void set_shape_pos(unsigned int shapex, unsigned int shapey)
		{ shape_pos = (shapex << 4) + shapey; }
	void set_lift(int l)
		{ lift = l; }
	Game_object *get_next()
		{ return next; }
	void set_next(Game_object *obj) // Set next in list.
		{ next = obj; }
	int lt(Game_object& obj2)	// Is this less than another in pos.?
		{
		int y = get_shape_pos_y(), y2 = obj2.get_shape_pos_y();
		int l = lift, l2 = obj2.lift;
		return (l < l2 || (l == l2 &&
			(y < y2 || (y == y2 && 
				get_shape_pos_x() < obj2.get_shape_pos_x()))));
		} 
					// Return chunk coords.
	int get_cx()
		{ return cx; }
	int get_cy()
		{ return cy; }
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
	virtual int get_schedule()	// Return NPC schedule.
		{ return 11; }		// Loiter.
	virtual char *get_name();
	virtual void set_property(int prop, int val)
		{  }
	virtual int get_property(int prop)
		{ return 0; }
	virtual int is_dragable();	// Can this be dragged?
					// Drop another onto this.
	virtual int drop(Game_object *obj);
					// Set/clear/get actor flag.
	virtual void set_flag(int flag) { }
	virtual void clear_flag(int flag) { }
	virtual int get_flag(int flag) { return 0; }
	};

/*
 *	A moveable game object (from 'ireg' files):
 */
class Ireg_game_object : public Game_object
	{
public:
					// Create from ireg. data.
	Ireg_game_object(unsigned char l, unsigned char h, 
				unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: Game_object(l, h, shapex, shapey, lft)
		{  }
	Ireg_game_object()		// Create fake entry.
		{  }
	virtual int is_dragable();	// Can this be dragged?
	};

/*
 *	A container object:
 */
class Container_game_object : public Ireg_game_object
	{
	Slist objects;			// List of objects contained.
public:
	Container_game_object(unsigned char l, unsigned char h, 
				unsigned int shapex,
				unsigned int shapey, unsigned int lft = 0)
		: Ireg_game_object(l, h, shapex, shapey, lft)
		{  }
	Container_game_object() {  }
	void add(Game_object *obj)
		{ objects.append(obj); }
	void remove(Game_object *obj)
		{ objects.remove(obj); }
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
					// Drop another onto this.
	virtual int drop(Game_object *obj);
	};

/*
 *	An "egg" is a special object that activates under certain
 *	circumstances.
 */
class Egg_object : public Game_object
	{
	unsigned char type;		// One of the below types.
	unsigned char probability;	// 1-100, chance of egg activating.
	unsigned char criteria;		// Don't know this one.
	unsigned char distance;		// Distance for activation.
	unsigned char flags;		// Formed from below flags.
	unsigned short data1, data2;	// More data, depending on type.
public:
	enum Egg_types {		// Types of eggs:
		monster = 1,
		jukebox = 2,
		soundsfx = 3,
		voice = 4,
		usecode = 5,
		missile = 6,
		teleport = 7,
		weather = 8,
		path = 9,
		button = 10
		};
	enum Egg_flag_shifts {
		nocturnal = 0,
		once = 1,
		hatched = 2,
		auto_reset = 3
		};
					// Create from ireg. data.
	Egg_object(unsigned char l, unsigned char h, unsigned int shapex,
		unsigned int shapey, unsigned int lft, 
		unsigned short itype,
		unsigned char prob, short d1, short d2)
		: Game_object(l, h, shapex, shapey, lft),
			probability(prob), data1(d1), data2(d2)
		{
		type = itype&0xf;
		criteria = (itype & (7<<4)) >> 4;
		distance = (itype >> 10) & 0x1f;
		unsigned char noct = (itype >> 7) & 1;
		unsigned char do_once = (itype >> 8) & 1;
		unsigned char htch = (itype >> 9) & 1;
		unsigned char ar = (itype >> 15) & 1;
		flags = (noct << nocturnal) + (do_once << once) +
			(htch << hatched) + (ar << auto_reset);
		}
	int within_distance(int abs_tx, int abs_ty);
					// Run usecode function.
	virtual void activate(Usecode_machine *umachine);
	};

/*
 *	Data cached for a chunk to speed up processing, but which doesn't need
 *	to be saved to disk:
 */
class Chunk_cache
	{
	unsigned char setup_done;	// Already setup.
	unsigned short blocked[256];	// For each tile, a bit for each lift
					//   level if it's blocked by an obj.
					// In the process of implementing:+++++
	Egg_object **egg_objects;	// ->eggs which influence this chunk.
	short num_eggs;
	unsigned short eggs[256];	// Bit #i (0-14) set means that the
					//   tile is within egg_object[i]'s
					//   influence.  Bit 15 means it's 1 or
					//   more of 
					//   egg_objects[15-(num_eggs-1)].
	friend class Chunk_object_list;
	Chunk_cache();			// Create empty one.
	~Chunk_cache();
					// Set/unset blocked region.
	void set_blocked(int startx, int starty, int endx, int endy,
						int lift, int ztiles, int set);
					// Add/remove object.
	void update_object(Chunk_object_list *chunk,
						Game_object *obj, int add);
					// Set up with chunk's data.
	void setup(Chunk_object_list *chunk);
					// Set blocked tile's bits.
	void set_blocked_tile(int tx, int ty, int lift, int ztiles)
		{
		blocked[ty*tiles_per_chunk + tx] |= 
						((1 << ztiles) - 1) << lift;
		}
					// Clear blocked tile's bits.
	void clear_blocked_tile(int tx, int ty, int lift, int ztiles)
		{
		blocked[ty*tiles_per_chunk + tx] &= 
					~(((1 << ztiles) - 1) << lift);
		}
					// Is a spot occupied?
	int is_blocked(int lift, int tx, int ty, int& new_lift);
					// Activate eggs nearby.
	void activate_eggs(Chunk_object_list *chunk,
				int tx, int ty, unsigned short eggbits);
	void activate_eggs(Chunk_object_list *chunk, int tx, int ty)
		{
		unsigned short eggbits = eggs[ty*tiles_per_chunk + tx];
		if (eggbits)
			activate_eggs(chunk, tx, ty, eggbits);
		}
	};

/*
 *	Game objects are stored in a list for each chunk, sorted from top-to-
 *	bottom, left-to-right.
 */
class Chunk_object_list
	{
	ShapeID flats[256];		// Flat (non-RLE) shapes.  The unused
					//   are "invalid" entries.
	Game_object *objects;		// ->first in ordered list.
	Npc_actor *npcs;		// List of NPC's in this chunk.
					//   (Managed by Npc_actor class.)
	Chunk_cache *cache;		// Data for chunks near player.
			// +++++Might cache/redo this egg stuff+++++
	Egg_object **egg_objects;	// ->eggs in chunk.
	int num_eggs;
	unsigned char eggs[256];	// For each (x,y), index of egg in
					//  egg_objects, or -1 if no egg there.
	unsigned char roof;		// 1 if a roof present.
	unsigned char cx, cy;		// Absolute chunk coords. of this.
public:
	friend class Npc_actor;
	Chunk_object_list(int chunkx, int chunky);
	void add(Game_object *obj);	// Add an object.
	void add_egg(Egg_object *egg);	// Add an egg.
	void remove(Game_object *obj);	// Remove an object.
	Egg_object *find_egg(int sx, int sy) //+++++Redo to use distance+++++
		{
		unsigned char index = eggs[sy*16 + sx];
		return (index < num_eggs ? egg_objects[index] : 0);
		}
	int is_roof()			// Is there a roof?
		{ return roof; }
	int get_cx()
		{ return cx; }
	int get_cy()
		{ return cy; }
	Npc_actor *get_npcs()		// Get ->first npc in chunk.
		{ return npcs; }
					// Set/get flat shape.
	void set_flat(int shapex, int shapey, ShapeID id)
		{ flats[16*shapey + shapex] = id; }
	ShapeID get_flat(int shapex, int shapey)
		{ return flats[16*shapey + shapex]; }
	Game_object *get_first()	// Return first object.
		{ return objects; }
					// Return next object.
	Game_object *get_next(Game_object *obj)
		{ return obj->next; }
					// Get/create cache.
	Chunk_cache *need_cache()
		{ 
		return cache ? cache 
				: (cache = new Chunk_cache()); 
		}
	void setup_cache()
		{ 
		if (!cache || !cache->setup_done)
			need_cache()->setup(this);
		}
					// Set/unset blocked region.
	void set_blocked(int startx, int starty, int endx, int endy,
						int lift, int ztiles, int set)
		{ need_cache()->set_blocked(startx, starty, endx, endy,
							lift, ztiles, set); }
					// Is a spot occupied?
	int is_blocked(int lift, int tx, int ty, int& new_lift)
		{ return cache->is_blocked(lift, tx, ty, new_lift); }
	void activate_eggs(int tx, int ty)
		{ need_cache()->activate_eggs(this, tx, ty); }
	};

/*
 *	Directions:
 */
enum Direction
	{
	east = 0,
	northeast = 1,
	north = 2,
	northwest = 3,
	west = 4,
	southwest = 5,
	south = 6,
	southeast = 7
	};

/*
 *	Axes:
 */
enum Axis
	{
	xaxis = 0,
	yaxis = 1
	};

/*
 *	A sequence of frames.  Frame 0 is the resting state.
 */
class Frames_sequence
	{
	unsigned char *frames;		// Last one is 0xff.
public:
	Frames_sequence(int cnt, unsigned char *f);
	~Frames_sequence()
		{ delete frames; }
	unsigned char get_resting()	// Get resting frame.
		{ return frames[0]; }
					// Get next frame.  Call
					//   with index = 0 for first one.
	unsigned char get_next(int& index)
		{
		unsigned char frame = frames[++index];
		return frame == 0xff ? frames[index = 1] : frame;
		}
	};

/*
 *	A sprite is a game object which can change shape and move around.
 */
class Sprite : public Container_game_object, public Time_sensitive
	{
	Chunk_object_list *chunk;	// The chunk list we're on.
	int major_frame_incr;		// # of pixels to move
					//   along major axis for each frame.
	Frames_sequence *frames[8];	// A frame sequence for each dir.
	/*
	 *	Motion info.:
	 */
	unsigned long curx, cury;	// Current pos. within world.
	Axis major_axis;		// Axis along which motion is greater.
	int major_delta, minor_delta;	// For each pixel we move along major
					//   axis, we add 'minor_delta'.  When
					//   the sum >= 'major_delta', we move
					//   1 pixel along minor axis, and
					//   subtract 'major_delta' from sum.
	int sum;			// Sum of 'minor_delta''s.
	int x_dir, y_dir;		// 1 or -1 for dir. along each axis.
	Frames_sequence *frames_seq;	// ->sequence of frames to display.
	int frame_index;		// Index into frames_seq.
protected:
	int frame_time;			// Time between frames in msecs.
public:
	Sprite(int shapenum);
	int in_world()			// Do we really exist?
		{ return chunk != 0; }
	int get_worldx()		// Get x-coord. within world.
		{ return cx*chunksize + get_shape_pos_x()*8; }
	int get_worldy()		// Get y-coord. within world.
		{ return cy*chunksize + get_shape_pos_y()*8; }
					// Set a frame seq. for a direction.
	void set_frame_sequence(Direction dir, int cnt, unsigned char *seq)
		{
		delete frames[(int) dir];
		frames[(int) dir] = new Frames_sequence(cnt, seq);
		}
					// Get resting frame for given dir.
	int get_resting_frame(Direction dir)
		{
		Frames_sequence *seq = frames[(int) dir];
		return seq != 0 ? seq->get_resting() : -1;
		}
					// Move to new chunk, shape coords.
	void move(int new_cx, int new_cy, Chunk_object_list *new_chunk, 
			int new_sx, int new_sy, int new_frame, 
			int new_lift = -1)
		{
		if (chunk)		// Remove from current chunk.
			chunk->remove(this);
		chunk = new_chunk;	// Add to new one.
		set_shape_pos(new_sx, new_sy);
		if (new_frame >= 0)
			set_frame(new_frame);
		if (new_lift >= 0)
			set_lift(new_lift);
		chunk->add(this);
		}
	int is_moving()
		{ return x_dir != 0; }
	void stop();			// Stop motion.
					// Start moving.
	void start(Game_window *gwin,
			unsigned long destx, unsigned long desty, int speed);
	virtual int is_dragable();	// Can this be dragged?
					// Figure next frame location.
	virtual int next_frame(unsigned long time,
		int& new_cx, int& new_cy, int& new_sx, int& new_sy,
		int& new_frame);
					// For Time_sensitive:
	virtual void handle_event(unsigned long time, long udata);
	};

/*
 *	A rectangle:
 */
class Rectangle
	{
public:					// Let's make it all public.
	int x, y;			// Position.
	int w, h;			// Dimensions.
	Rectangle(int xin, int yin, int win, int hin)
			: x(xin), y(yin), w(win), h(hin)
		{  }
	Rectangle() { }			// An uninitialized one.
	int has_point(int px, int py)	// Is this point in it?
		{ return (px >= x && px < x + w && py >= y && py < y + h); }
	Rectangle add(Rectangle& r2)	// Add another to this one to get
		{			//  a rect. that encloses both.
		int xend = x + w, yend = y + h;
		int xend2 = r2.x + r2.w, yend2 = r2.y + r2.h;
		Rectangle r;		// Return this.
		r.x = x < r2.x ? x : r2.x;
		r.y = y < r2.y ? y : r2.y;
		r.w = (xend > xend2 ? xend : xend2) - r.x;
		r.h = (yend > yend2 ? yend : yend2) - r.y;
		return (r);
		}
					// Intersect another with this.
	Rectangle intersect(Rectangle& r2)
		{
		int xend = x + w, yend = y + h;
		int xend2 = r2.x + r2.w, yend2 = r2.y + r2.h;
		Rectangle r;		// Return this.
		r.x = x >= r2.x ? x : r2.x;
		r.y = y >= r2.y ? y : r2.y;
		r.w = (xend <= xend2 ? xend : xend2) - r.x;
		r.h = (yend <= yend2 ? yend : yend2) - r.y;
		return (r);
		}
	void shift(int deltax, int deltay)
		{
		x += deltax;
		y += deltay;
		}		
	};

/*
 *	A text object is a message that stays on the screen for just a couple
 *	of seconds.  These are all kept in a single list, and managed by
 *	Game_window.
 */
class Text_object : public Time_sensitive
	{
	Text_object *next, *prev;	// All of them are chained together.
	char *msg;			// What to print.
	int cx, cy;			// Chunk coords of upper-left corner.
	unsigned char sx, sy;		// Tile coords. within chunk.
	short width, height;		// Dimensions of rectangle.
public:
	friend class Game_window;
	Text_object(char *m, int c_x, int c_y, int s_x, int s_y, int w, int h)
		: msg(m), cx(c_x), cy(c_y), sx(s_x), sy(s_y),
		  width(w), height(h)
		{  }
					// At timeout, remove from screen.
	virtual void handle_event(unsigned long curtime, long udata);
	};

/*
 *	A gump contains an image of an open container from "gumps.vga".
 */
class Gump_object : public ShapeID
	{
	Gump_object *next;		// ->next to draw.
	Game_object *container;		// What this gump shows.
	int x, y;			// Location on screen.
	unsigned char shapenum;
public:
	Gump_object(Game_object *cont, int initx, int inity, int shnum)
		: container(cont), x(initx), y(inity), ShapeID(shnum, 0)
		{  }
	int get_x()			// Get coords.
		{ return x; }
	int get_y()
		{ return y; }
					// Append to end of chain.
	void append_to_chain(Gump_object *& chain);
					// Remove from chain.
	void remove_from_chain(Gump_object *& chain);
	Gump_object *get_next()		// (Chain ends with ->next == 0.)
		{ return next; }
	};

#endif
