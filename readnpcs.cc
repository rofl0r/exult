/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Readnpcs.cc - Read in NPC's from npc.dat & schedule.dat.  Also writes
 **		npc.dat back out.
 **
 **	Written: 5/13/99 - JSF
 **/

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

#include <string.h>
#include "gamewin.h"
#include "utils.h"
#include "fnames.h"

/*
 *	Read in the NPC's, plus the monster info.
 */

void Game_window::read_npcs
	(
	)
	{
	ifstream nfile;
	u7open(nfile, NPC_DAT);
	num_npcs1 = Read2(nfile);		// Get counts.
	int cnt2 = Read2(nfile);
	num_npcs = num_npcs1 + cnt2;
	npcs = new Actor *[num_npcs];
	int i;
	for (i = 0; i < num_npcs; i++)
		{
					// Get chunk/tile coords.
		unsigned locx = Read1(nfile);
		unsigned locy = Read1(nfile);
					// Get shape, frame #.
		unsigned char shape[2];
		nfile.read(shape, 2);
		int iflag1 = Read2(nfile);// Inventory flag.
					// Superchunk #.
		int schunk = Read1(nfile);
		Read1(nfile);		// Skip next byte.
					// Get usecode function #.
		int usefun = Read2(nfile);
					// Lift is high 4 bits.
		int lift = usefun >> 12;
		usefun &= 0xfff;
		if (i >= num_npcs1)	// Type2?
			usefun = -1;	// Let's try this.
					// Guessing:  !!
		int food_level = Read1(nfile);
		nfile.seekg(3, ios::cur);// Skip 3 bytes.
					// Another inventory flag.
		int iflag2 = Read2(nfile);
					// Skip next 2.
		nfile.seekg(2, ios::cur);
					// Get char. atts.
		int strength = Read1(nfile);
		int dexterity = Read1(nfile);
		int intelligence = Read1(nfile);
		int combat = Read1(nfile);
		int schedtype = Read1(nfile);
		nfile.seekg(4, ios::cur); //??
		int mana = Read1(nfile);// ??
		nfile.seekg(4, ios::cur);
		int exp = Read2(nfile);	// Could this be 4 bytes?
		nfile.seekg(2, ios::cur);
		int train = Read1(nfile);
					// Get name.
		nfile.seekg(0x40, ios::cur);
		char namebuf[17];
		nfile.read(namebuf, 16);
		namebuf[16] = 0;	// Be sure it's 0-delimited.
		int namelen = strlen(namebuf);
					// Get abs. chunk. coords. of schunk.
		int scy = 16*(schunk/12);
		int scx = 16*(schunk%12);
		int cx = locx >> 4;	// Get chunk indices within schunk.
		int cy = locy >> 4;
					// Get shape #'s.
		int shapex = locx & 0xf;
		int shapey = locy & 0xf;
		Actor *actor;
		Npc_actor *npc_actor = 0;
		if (i == 0)		// Main character?
			{
			actor = main_actor =
				new Main_actor(namebuf, 
				shape[0] + 256*(shape[1]&0x3), i, usefun);
					// Skipping intro?
			if (usecode->get_global_flag(
					Usecode_machine::did_first_scene))
				main_actor->clear_flag(Actor::dont_render);
			else
				main_actor->set_flag(Actor::dont_render);
			}
		else			// Create NPC.
			actor = npc_actor = new Npc_actor(namebuf, 
				shape[0] + 256*(shape[1]&0x3), i, usefun);
		npcs[i] = actor;	// Store in list.
		Chunk_object_list *olist = get_objects(scx + cx, scy + cy);
#if 0
		actor->set_initial_location(scx + cx, scy + cy, olist,
				shapex, shapey, (shape[1]>>2) & 0x1f, lift);
#endif
		actor->move(0, scx + cx, scy + cy, olist,
				shapex, shapey, (shape[1]>>2) & 0x1f, lift);
					// Put in chunk's NPC list.
		if (npc_actor)
			{
			npc_actor->switched_chunks(0, olist);
			npc_actor->set_schedule_type(schedtype);
			}
		else if (main_actor)
			main_actor->switched_chunks(0, olist);
					// Set attributes.
		actor->set_property((int) Actor::strength, strength);
		actor->set_property((int) Actor::dexterity, dexterity);
		actor->set_property((int) Actor::intelligence, intelligence);
					// Hits = strength, I think.
		actor->set_property((int) Actor::health, strength);
		actor->set_property((int) Actor::combat, combat);
		actor->set_property((int) Actor::mana, mana);
		actor->set_property((int) Actor::magic, mana);
		actor->set_property((int) Actor::exp, exp);
		actor->set_property((int) Actor::training, train);
		actor->set_property((int) Actor::food_level, food_level);
#if 0
cout << i << " Creating " << namebuf << ", shape = " << 
	actor->get_shapenum() <<
	", frame = " << actor->get_framenum() << ", usecode = " <<
				usefun << '\n';
cout << "Chunk coords are (" << scx + cx << ", " << scy + cy << "), lift is "
	<< lift << '\n';
#endif
		if (iflag1 && iflag2)	// Inventory?  Read (but ignore++++).
			read_ireg_objects(nfile, scx, scy, actor);
		}
	read_schedules();		// Now get their schedules.
	ifstream mfile;			// Now get monsters.
	u7open(mfile, MONSTERS);
	num_monsters = Read1(mfile);	// Get count.
					// Create list, and read it in.
	monster_info = new Monster_info[num_monsters];
	unsigned char monster[25];
	for (i = 0; i < num_monsters; i++)
		{
		int shape = Read2(mfile);
		mfile.read(monster, 23);// Get the rest.
		monster_info[i].set(shape, monster[0], monster[1],
				monster[2], monster[3], monster[4]);
		}
	}

/*
 *	Write NPC (and monster) data back out.
 *
 *	Output:	0 if error, already reported.
 */

int Game_window::write_npcs
	(
	)
	{
	ofstream nfile;
	if (!U7open(nfile, NPC_DAT))
		{			// +++++Better error???
		cerr << "Exult:  Error opening '" << NPC_DAT <<
				"' for writing\n";
		return (0);
		}
	Write2(nfile, num_npcs1);	// Start with counts.
	Write2(nfile, num_npcs - num_npcs1);
	int i;
	for (i = 0; i < num_npcs; i++)
		npcs[i]->write(nfile);
	//++++++++++++Don't forget monsters.
	nfile.flush();
	int result = nfile.good();
	if (!result)			// ++++Better error system needed??
		cerr << "Exult:  Error writing '" << NPC_DAT << "'\n";
	return (result);
	}

/*
 *	Read NPC schedules.
 */

void Game_window::read_schedules
	(
	)
	{
	ifstream sfile;
	u7open(sfile, SCHEDULE_DAT);
	int num_npcs = Read4(sfile);	// # of NPC's, not include Avatar.
	short *offsets = new short[num_npcs];
	int i;				// Read offsets with list of scheds.
	for (i = 0; i < num_npcs; i++)
		offsets[i] = Read2(sfile);
	for (i = 0; i < num_npcs - 1; i++)	// Do each NPC, except Avatar.
		{
					// Avatar isn't included here.
		Npc_actor *npc = (Npc_actor *) npcs[i + 1];
		int cnt = offsets[i + 1] - offsets[i];
#if 0
		cout << "Schedule for " << npc->get_name() << ":\n";
#endif
					// Read schedules into this array.
		Schedule_change *schedules = new Schedule_change[cnt];
		for (int j = 0; j < cnt; j++)
			{
			unsigned char ent[4];
			sfile.read(ent, 4);
			schedules[j].set(ent);
#if 0
			cout << "    " << sched->get_type() << 
				", time = " << sched->get_time() << '\n';
#endif
			}
					// Store in NPC.
		npc->set_schedules(schedules, cnt);
#if 0	/* ++++++++ Now done in gameclk.cc. */
		// Start their initial schedule
		npc->update_schedule(this,2);
		// Set their starting location
		npc->move(npc->initial_location.cx, npc->initial_location.cy, npc->initial_location.chunk,
				npc->initial_location.sx, npc->initial_location.sy, npc->initial_location.frame, npc->initial_location.lift);
#endif
		}
	delete [] offsets;		// Done with this.
	}


