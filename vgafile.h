/**	-*-mode: Fundamental; tab-width: 8; -*-
**
 **	Handle access to one of the xxx.vga files.
 **
 **	Written: 4/29/99 - JSF
 **/

#ifndef INCL_VGAFILE
#define INCL_VGAFILE	1

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

#include <fstream.h>
#include "fnames.h"

class Shape;

/*
 *	A shape from "shapes.vga":
 */
class Shape_frame
	{
	unsigned char rle;		// 1 if run-length encoded.
	unsigned char *data;		// The actual data.
	short xleft;			// Extent to left of origin.
	short xright;			// Extent to right.
	short yabove;			// Extent above origin.
	short ybelow;			// Extent below origin.
	Shape_frame *reflect();		// Create new frame, reflected.
					// Create RLE data & store in frame.
	void create_rle(unsigned char *pixels, int w, int h);
					// Create from RLE entry.
	void get_rle_shape(ifstream& shapes, long filepos, long len);
public:
	friend class Game_window;
	friend class Shape;
	Shape_frame() : data(0)
		{  }
					// Read in shape/frame.
	unsigned char read(ifstream& shapes, unsigned long shapeoff,
					unsigned long shapelen, int frnum);
	int has_point(int x, int y);	// Is a point within the shape?
	int get_width()			// Get dimensions.
		{ return xleft + xright; }
	int get_height()
		{ return yabove + ybelow; }
	int get_xleft()
		{ return xleft; }
	int get_xright()
		{ return xright; }
	int get_yabove()
		{ return yabove; }
	int get_ybelow()
		{ return ybelow; }
	~Shape_frame()
		{ delete data; }
	};

/*
 *	A shape from a .vga file consists of one of more frames.
 */
class Shape
	{
protected:
	Shape_frame **frames;		// List of ->'s to frames.
	unsigned char num_frames;	// # of frames.
					// Create reflected frame.
	Shape_frame *reflect(ifstream& shapes, int shnum, int frnum);
					// Read in shape/frame.
	Shape_frame *read(ifstream& shapes, int shnum, int frnum);
					// Store shape that was read.
	Shape_frame *store_frame(Shape_frame *frame, int framenum);
public:
	friend class Vga_file;
	Shape() : frames(0), num_frames(0)
		{  }
	Shape_frame *get(ifstream& shapes, int shnum, int frnum)
		{ 
		return (frames && frnum < num_frames && frames[frnum]) ? 
			frames[frnum] : read(shapes, shnum, frnum); 
		}
	};

/*
 *	A shape file just has one shape with multiple frames.  They're all
 *	read in during construction.
 */
class Shape_file : private Shape
	{
public:
	Shape_file(char *nm);
	int get_num_frames()
		{ return num_frames; }
	Shape_frame *get_frame(int framenum)
		{ return framenum < num_frames ? frames[framenum] : 0; }
	};

/*
 *	A class for accessing any .vga file:
 */
class Vga_file
	{
	ifstream file;			// For reading.
protected:
	int num_shapes;			// Total # of shapes.
	Shape *shapes;			// List of ->'s to shapes' lists
public:
	Vga_file(char *nm);
	int get_num_shapes()
		{ return num_shapes; }
	int is_good()
		{ return (num_shapes != 0); }
					// Get shape.
	Shape_frame *get_shape(int shapenum, int framenum = 0)
		{
		return (shapes[shapenum].get(file, shapenum, framenum));
		}
					// Get # frames for a shape.
	int get_num_frames(int shapenum)
		{
		get_shape(shapenum, 0);	// Force it into memory.
		return shapes[shapenum].num_frames;
		}
	};

/*
 *	This class contains information only about shapes from "shapes.vga".
 */
class Shape_info
	{
	unsigned char tfa[3];		// From "tfa.dat".+++++Keep for
					//   debugging, for now.
					// 3D dimensions in tiles:
	unsigned char xtiles, ytiles, ztiles;
	unsigned char weight, volume;	// From "wgtvol.dat".
	unsigned char shpdims[2];	// From "shpdims.dat".
	void set_tfa_data()		// Set fields from tfa.
		{
		xtiles = 1 + (tfa[2]&7);
		ytiles = 1 + ((tfa[2]>>3)&7);
		ztiles = (tfa[0] >> 5);
		}
public:
	friend class Shapes_vga_file;	// Class that reads in data.
	Shape_info() : xtiles(0), ytiles(0), ztiles(0), weight(0), volume(0)
		{ tfa[0] = tfa[1] = tfa[2] = shpdims[0] = shpdims[1] = 0; }
	int get_weight()		// Get weight, volume.
		{ return weight; }
	int get_volume()
		{ return volume; }
	int get_3d_height()		// Height (in lifts?).
		{ return ztiles; }
	int get_3d_xtiles()		// Dimension in tiles - X.
		{ return xtiles; }
	int get_3d_ytiles()		// Dimension in tiles - Y.
		{ return ytiles; }
	unsigned char get_tfa(int i)	// For debugging:
		{ return tfa[i]; }
	int is_animated()
		{ return (tfa[0] & (1<<2)) != 0; }
	int is_light_source()
		{ return (tfa[2] & (1<<6)) != 0; }
	int is_transparent()		// ??
		{ return (tfa[1] & (1<<7)) != 0; }
	int has_translucency()
		{ return (tfa[2] & (1<<7)) != 0; }
	int is_solid()			// Guessing.  Means can't walk through.
		{ return (tfa[0] & (1<<3)) != 0; }
	int is_xobstacle()		// Obstacle in x-dir.???
		{ return (shpdims[1] & 1) != 0; }
	int is_yobstacle()		// Obstacle in y-dir.???
		{ return (shpdims[0] & 1) != 0; }
	/*
	 *	TFA[1][b0-b6] seems to indicate object types:
	 *	+++++++Just guessing for now.
	enum Shape_class {
		bed = 2,		// Also water trough.
		quantity = 3,		// Can have more than 1:  coins, arrs.
		shutters = 4,		// Also mirrors.
		wearable = 5,		// Includes wieldable weapons, food,
					//   table, curtain??
		container = 6,		// Includes NPC's.
		egg = 7,
		window = 14,
		door = 34
	*/
	};

/*
 *	The "shapes.vga" file:
 */
class Shapes_vga_file : public Vga_file
	{
	Shape_info *info;		// Extra info. about each shape.
	Shape_info zinfo;		// A fake one (all 0's).
public:
	Shapes_vga_file() : Vga_file(SHAPES_VGA)
		{ info = new Shape_info[num_shapes]; }
	int read_info();		// Read additional data files.
	Shape_info& get_info(int shapenum)
		{ return info[shapenum]; }
	};

#endif
