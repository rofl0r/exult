/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Usecode.h - Interpreter for usecode.
 **
 **	Written: 8/12/99 - JSF
 **/

#ifndef INCL_USECODE
#define INCL_USECODE	1

/*
Copyright (C) 1999  Jeffrey S. Freedman

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

#ifdef MACOS
#include <iostream>
using namespace std;

#else
class istream;
class ostream;
#endif
class Game_window;
class Game_object;
class Text_gump;
class Vector;
class Deleted_objects;
class Actor;

#include "tiles.h"
#include <vector>	// STL container
#include <deque>	// STL container
#include <string>	// STL string

#define	USECODE_INTRINSIC(NAME)	Usecode_value	Usecode_machine::UI_ ## NAME ## (int event,int intrinsic,int num_parms,Usecode_value parms[12])
#define	USECODE_INTRINSIC_DECL(NAME)	Usecode_value	UI_ ## NAME ## (int event,int intrinsic,int num_parms,Usecode_value parms[12])

/*
 *	A value that we store can be an integer, string, or array.
 */
class Usecode_value
	{
public:
	enum Val_type			// The types:
		{
		int_type = 0,
		string_type = 1,	// Allocated string.
		array_type = 2,
		end_of_array_type = 3	// Marks end of array.
		};
private:
	unsigned char type;		// Type stored here.
	union
		{
		long intval;
		const char *str;
		Usecode_value *array;
		} value;
					// Count array elements.
	static int count_array(const Usecode_value& val);
public:
	inline Usecode_value() : type((unsigned char) int_type)
		{ value.intval = 0; }
	inline Usecode_value(int ival) : type((unsigned char) int_type)
		{ value.intval = ival; }
	Usecode_value(const char *s);
					// Create array with 1st element.
	Usecode_value(int size, Usecode_value *elem0) 
			: type((unsigned char) array_type)
		{
		value.array = new Usecode_value[size + 1];
		value.array[size].type = (unsigned char) end_of_array_type;
		if (elem0)
			value.array[0] = *elem0;
		}
	~Usecode_value()
		{
		if (type == (unsigned char) array_type)
			delete [] value.array;
		else if (type == (unsigned char) string_type)
			delete value.str;
		}
	Usecode_value &operator=(const Usecode_value& v2);
					// Copy ctor.
	inline Usecode_value(const Usecode_value& v2) : type((unsigned char) int_type)
		{ *this = v2; }
					// Comparator.
	int operator==(const Usecode_value& v2);
	inline Val_type get_type() const
		{ return (Val_type) type; }
	int get_array_size() const		// Get size of array.
		{ return type == (int) array_type ? count_array(*this) : 0; }
	bool is_array() const
		{ return type == (int) array_type; }
	bool is_int() const
		{ return type == (int) int_type; }
	long get_int_value() const	// Get integer value.
		{ return (type == (int) int_type ? value.intval : 0); }
					// Get string value.
	const char *get_str_value() const
		{ return (type == (int) string_type ? value.str : 0); }
					// Add array element. (No checking!)
	void put_elem(int i, Usecode_value& val)
		{ value.array[i] = val; }
					// Get an array element, or *this.
	inline Usecode_value& get_elem(int i)
		{
		return type == (int) array_type ? value.array[i] : *this;
		}
	inline bool is_false() const			// Represents a FALSE value?
		{
		return is_int() ? value.intval == 0
				: is_array() ? value.array[0].type == 
					(int) end_of_array_type : 0;
		}
	inline bool is_true() const
		{ return !is_false(); }

	int resize(int new_size);	// Resize array.
					// Look in array for given value.
	int find_elem(const Usecode_value& val);
					// Concat. to end of this array.
	Usecode_value& concat(Usecode_value& val2);
					// Add value(s) to an array.
	int add_values(int index, Usecode_value& val2);
	void print(ostream& out);	// Print in ASCII.
	};

/*
 *	A single function:
 */
class Usecode_function
	{
	int id;				// The function #.  (Appears to be the
					//   game item # the function gets
					//   called for.)
	int len;			// Length.
	unsigned char *code;		// The code.
	friend class Usecode_machine;
					// Create from file.
	Usecode_function(istream& file);
	inline ~Usecode_function()
		{ delete code; }
	};


/*
 *	A set of answers:
 */
struct Answers
	{
	friend class Usecode_machine;
	vector<string> answers;	// What we can click on.
	Answers();
	void add_answer(const char *str);	// Add to the list.
	void add_answer(Usecode_value& val);
	void remove_answer(Usecode_value& val);
	void _remove_answer(const char *);
	void	clear(void);
	};	

/*
 *	Here's our virtual machine for running usecode.
 */
class Usecode_machine
	{
	Game_window *gwin;		// Game window.
	Vector *funs;			// I'th entry contains funs for ID's
					//    256*i + n.
	int call_depth;			// How far deep we are.
	unsigned char gflags[1024];	// Global flags.
	int party[8];			// NPC #'s of party members.
	int party_count;		// # of NPC's in party.
	Text_gump *book;		// Book/scroll being displayed.
	Game_object *caller_item;	// Item this is being called on.
	Game_object *last_created;	// Last item created with intrins. x24.
	Deleted_objects *removed;	// List of 'removed' objects.
	const char *user_choice;	// String user clicked on.
	char *String;			// The single string register.
	void append_string(const char *txt);	// Append to string.
	void show_pending_text();	// Make sure user's seen all text.
	void show_book();		// "Say" book/scroll text.
	void say_string();		// "Say" the string.
	Usecode_value *stack;		// Stack.
	Usecode_value *sp;		// Stack ptr.  Grows upwards.
	void stack_error(int under);
	inline void push(Usecode_value& val)	// Push/pop stack.
		{ *sp++ = val; }
	inline Usecode_value pop()		// +++++Watch for too many str. copies.
		{ 
		if (sp <= stack)
			stack_error(1);
		return *--sp; 
		}
	inline void pushi(long val)		// Push/pop integers.
		{
		Usecode_value v(val);
		push(v);
		}
	inline int popi()
		{
		Usecode_value val = pop();
		return (val.get_int_value());
		}
					// Push/pop strings.
	inline void pushs(char *s)
		{
		Usecode_value val(s);
		push(val);
		}
	Answers answers;		// What user can click on.
	deque< Answers > answer_stack;
					// Get ->obj. from 'itemref'.
	Game_object *get_item(Usecode_value& itemref);
					// "Safe" cast to Actor.
	Actor *as_actor(Game_object *obj);
					// Get position.
	Tile_coord get_position(Usecode_value& itemref);
	/*
	 *	Built-in usecode functions:
	 */
public:
	typedef Usecode_value (Usecode_machine::*UsecodeIntrinsicFn)(
		int event,int intrinsic,int num_parms,Usecode_value parms[12]);

	void show_npc_face(Usecode_value& arg1, Usecode_value& arg2);
	void remove_npc_face(Usecode_value& arg1);
	void set_item_shape(Usecode_value& item_arg, Usecode_value& shape_arg);
	void set_item_frame(Usecode_value& item_arg, Usecode_value& frame_arg);
	int get_item_shape(Usecode_value& item_arg);
	int get_item_frame(Usecode_value& item_arg);
	void remove_item(Game_object *obj);
	int npc_in_party(Game_object *npc);
	void add_to_party(Game_object *npc);
	void remove_from_party(Game_object *npc);
	Usecode_value get_party();
	void item_say(Usecode_value& objval, Usecode_value& strval);
	Usecode_value find_nearby(Usecode_value& objval,
		Usecode_value& shapeval, Usecode_value& qval,
							Usecode_value& mval);
	Usecode_value find_nearest(Usecode_value& objval,
			Usecode_value& shapeval, Usecode_value& unknown);
	Usecode_value find_direction(Usecode_value& from, Usecode_value& to);
	Usecode_value count_objects(Usecode_value& objval,
		Usecode_value& shapeval, Usecode_value& qualval,
						Usecode_value& frameval);
	Usecode_value get_objects(Usecode_value& objval,
		Usecode_value& shapeval, Usecode_value& qualval,
						Usecode_value& frameval);
	int remove_party_items(Usecode_value& quantval,
		Usecode_value& shapeval, Usecode_value& qualval,
			Usecode_value& frameval, Usecode_value& flagval);
	Usecode_value add_party_items(Usecode_value& quantval,
		Usecode_value& shapeval, Usecode_value& qualval,
			Usecode_value& frameval, Usecode_value& flagval);
	Usecode_value click_on_item();

	/*
         *	Embedded intrinsics
	 */

	Usecode_value	Execute_Intrinsic(UsecodeIntrinsicFn func,const char *name,int event,int intrinsic,int num_parms,Usecode_value parms[12]);
	USECODE_INTRINSIC_DECL(NOP);
	USECODE_INTRINSIC_DECL(UNKNOWN);
	USECODE_INTRINSIC_DECL(get_random);
	USECODE_INTRINSIC_DECL(execute_usecode_array);
	USECODE_INTRINSIC_DECL(delayed_execute_usecode_array);
	USECODE_INTRINSIC_DECL(show_npc_face);
	USECODE_INTRINSIC_DECL(remove_npc_face);
	USECODE_INTRINSIC_DECL(add_answer);
	USECODE_INTRINSIC_DECL(remove_answer);
	USECODE_INTRINSIC_DECL(push_answers);
	USECODE_INTRINSIC_DECL(pop_answers);
	USECODE_INTRINSIC_DECL(clear_answers);
	USECODE_INTRINSIC_DECL(select_from_menu);
	USECODE_INTRINSIC_DECL(select_from_menu2);
	USECODE_INTRINSIC_DECL(input_numeric_value);
	USECODE_INTRINSIC_DECL(set_item_shape);
	USECODE_INTRINSIC_DECL(find_nearest);
	USECODE_INTRINSIC_DECL(die_roll);
	USECODE_INTRINSIC_DECL(get_item_shape);
	USECODE_INTRINSIC_DECL(get_item_frame);
	USECODE_INTRINSIC_DECL(set_item_frame);
	USECODE_INTRINSIC_DECL(get_item_quality);
	USECODE_INTRINSIC_DECL(set_item_quality);
        USECODE_INTRINSIC_DECL(get_item_quantity);
        USECODE_INTRINSIC_DECL(set_item_quantity);
        USECODE_INTRINSIC_DECL(get_object_position);
	USECODE_INTRINSIC_DECL(get_distance);
        USECODE_INTRINSIC_DECL(find_direction);
        USECODE_INTRINSIC_DECL(get_npc_object);
        USECODE_INTRINSIC_DECL(get_schedule_type);
        USECODE_INTRINSIC_DECL(set_schedule_type);
        USECODE_INTRINSIC_DECL(add_to_party);
        USECODE_INTRINSIC_DECL(remove_from_party);
        USECODE_INTRINSIC_DECL(get_npc_prop);
        USECODE_INTRINSIC_DECL(set_npc_prop);
        USECODE_INTRINSIC_DECL(get_avatar_ref);
        USECODE_INTRINSIC_DECL(get_party_list);
        USECODE_INTRINSIC_DECL(create_new_object);
        USECODE_INTRINSIC_DECL(set_last_created);
        USECODE_INTRINSIC_DECL(update_last_created);
        USECODE_INTRINSIC_DECL(get_npc_name);
        USECODE_INTRINSIC_DECL(count_objects);
	USECODE_INTRINSIC_DECL(take_from_owner);
        USECODE_INTRINSIC_DECL(get_cont_items);
        USECODE_INTRINSIC_DECL(remove_party_items);
        USECODE_INTRINSIC_DECL(add_party_items);
        USECODE_INTRINSIC_DECL(play_music);
        USECODE_INTRINSIC_DECL(npc_nearby);
        USECODE_INTRINSIC_DECL(find_nearby_avatar);
        USECODE_INTRINSIC_DECL(is_npc);
        USECODE_INTRINSIC_DECL(display_runes);
        USECODE_INTRINSIC_DECL(click_on_item);
        USECODE_INTRINSIC_DECL(find_nearby);
        USECODE_INTRINSIC_DECL(give_last_created);
	USECODE_INTRINSIC_DECL(is_dead);
        USECODE_INTRINSIC_DECL(game_hour);
        USECODE_INTRINSIC_DECL(game_minute);
	USECODE_INTRINSIC_DECL(get_npc_number);
	USECODE_INTRINSIC_DECL(part_of_day);
	USECODE_INTRINSIC_DECL(get_alignment);
	USECODE_INTRINSIC_DECL(set_alignment);
	USECODE_INTRINSIC_DECL(move_object);
	USECODE_INTRINSIC_DECL(item_say);
	USECODE_INTRINSIC_DECL(get_lift);
	USECODE_INTRINSIC_DECL(set_lift);
	USECODE_INTRINSIC_DECL(sit_down);
	USECODE_INTRINSIC_DECL(display_map);
	USECODE_INTRINSIC_DECL(add_spell);
	USECODE_INTRINSIC_DECL(sprite_effect);
	USECODE_INTRINSIC_DECL(book_mode);
	USECODE_INTRINSIC_DECL(earthquake);
	USECODE_INTRINSIC_DECL(is_pc_female);
	USECODE_INTRINSIC_DECL(run_endgame);
	USECODE_INTRINSIC_DECL(get_array_size);
	USECODE_INTRINSIC_DECL(is_pc_inside);
	USECODE_INTRINSIC_DECL(mouse_exists);
	USECODE_INTRINSIC_DECL(get_container);
	USECODE_INTRINSIC_DECL(remove_item);
	USECODE_INTRINSIC_DECL(get_equipment_list);
	USECODE_INTRINSIC_DECL(nap_time);
	USECODE_INTRINSIC_DECL(advance_time);
	USECODE_INTRINSIC_DECL(path_run_usecode);
	USECODE_INTRINSIC_DECL(close_gumps);
	USECODE_INTRINSIC_DECL(is_not_blocked);
	USECODE_INTRINSIC_DECL(direction_from);
	USECODE_INTRINSIC_DECL(get_npc_flag);
	USECODE_INTRINSIC_DECL(set_npc_flag);
	USECODE_INTRINSIC_DECL(clear_npc_flag);
	USECODE_INTRINSIC_DECL(run_usecode);
	USECODE_INTRINSIC_DECL(fade_palette);
	USECODE_INTRINSIC_DECL(get_party_list2);




	/*
	 *	Other private methods:
	 */
					// Call instrinsic function.
	Usecode_value call_intrinsic(int event, int intrinsic, int num_parms);
	void click_to_continue();	// Wait for user to click.
	void set_book(Text_gump *b);	// Set book/scroll to display.
	const char *get_user_choice();	// Get user's choice.
	int get_user_choice_num();
					// Run the function.
	void run(Usecode_function *fun, int event);
					// Call desired function.
	int call_usecode_function(int id, int event = 0, 
						Usecode_value *parm0 = 0);
public:
	friend class Scheduled_usecode;
	Usecode_machine(istream& file, Game_window *gw);
	~Usecode_machine();
					// Possible events:
	enum Usecode_events {
		npc_proximity = 0,
		double_click = 1,
		internal_exec = 2,	// Internal call via intr. 1 or 2.
		egg_proximity = 3
		};
	enum Global_flag_names {
		did_first_scene = 0x3b,	// Went through 1st scene with Iolo.
		have_trinsic_password = 0x3d,
		found_stable_key = 0x48
		};
	int get_global_flag(int i)	// Get/set ith flag.
		{ return gflags[i]; }
	void set_global_flag(int i, int val = 1)
		{ gflags[i] = (val == 1); }
	int get_party_count()		// Get # party members.
		{ return party_count; }
	int get_party_member(int i)	// Get npc# of i'th party member.
		{ return party[i]; }
	int in_usecode()		// Currently in a usecode function?
		{ return call_depth > 0; }
					// Call desired function.
	int call_usecode(int id, Game_object *obj, Usecode_events event)
		{
					// Avoid these when already execing.
		if (call_depth && event == npc_proximity)
			return (0);
		Game_object *prev_item = caller_item;
		caller_item = obj;
		answers.clear();
		Usecode_value parm(0);	// They all seem to take 1 parm.
		int ret = call_usecode_function(id, event, &parm);
		set_book(0);
		caller_item = prev_item;
		return ret;
		}
	int write();			// Write out 'gamedat/usecode.dat'.
	int read();			// Reat in 'gamedat/usecode.dat'.
	void link_party();		// Set party's id's.
	};

#endif	/* INCL_USECODE */
