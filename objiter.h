/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Objiter.h - Game objects iterator.
 **
 **	Written: 7/29/2000 - JSF
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

#ifndef INCL_OBJITER
#define INCL_OBJITER	1

/*
 *	Iterate through list of objects.
 */
class Object_iterator
	{
	Game_object *first;
	Game_object *cur;		// Next to return.
public:
	void reset()
		{ cur = first; }
	Object_iterator(Game_object *f) : first(f)
		{ reset(); }
	Object_iterator(Chunk_object_list *chunk) : first(chunk->objects)
		{ reset(); }
	Game_object *get_next()
		{
		if (!cur)
			return 0;
		Game_object *ret = cur;
		cur = cur->get_next();
		if (cur == first)
			cur = 0;
		return ret;
		}
	};

/*
 *	Iterate backwards through list of objects.
 */
class Object_iterator_backwards
	{
	Game_object *first;
	Game_object *cur;		// Return prev. to this.
public:
	void reset()
		{ cur = first; }
	Object_iterator_backwards(Chunk_object_list *chunk) 
		: first(chunk->objects)
		{ reset(); }
	Game_object *get_next()
		{
		if (!cur)
			return 0;
		cur = cur->get_prev();
		Game_object *ret = cur;
		if (cur == first)
			cur = 0;
		return ret;
		}
	};

#endif
