/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Npctime.cc - Timed-even handlers for NPC's.
 **
 **	Written: 8/4/00 - JSF
 **/

#include "npctime.h"
#include "gamewin.h"
#include "actors.h"
#include "items.h"

/*
 *	Base class for keeping track of things like poison, protection, hunger.
 */
class Npc_timer : public Time_sensitive
	{
protected:
	Npc_timer_list *list;		// Where NPC stores ->this.
	unsigned long get_minute();	// Get game minutes.
public:
	Npc_timer(Npc_timer_list *l);
	virtual ~Npc_timer();
	};

/*
 *	Handle starvation.
 */
class Npc_hunger_timer : public Npc_timer
	{
	unsigned long last_time;	// Last game minute when penalized.
public:
	Npc_hunger_timer(Npc_timer_list *l) : Npc_timer(l)
		{ last_time = get_minute(); }
	virtual ~Npc_hunger_timer();
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Handle poison.
 */
class Npc_poison_timer : public Npc_timer
	{
	unsigned long end_time;		// Time when it wears off.
	unsigned long last_time;	// Last game minute when penalized.
public:
	Npc_poison_timer(Npc_timer_list *l);
	virtual ~Npc_poison_timer();
					// Handle events:
	void handle_event(unsigned long curtime, long udata);
	};

/*
 *	Delete list.
 */

Npc_timer_list::~Npc_timer_list
	(
	)
	{
	delete hunger;
	delete poison;
	}

/*
 *	Start hunger (if not already there).
 */

void Npc_timer_list::start_hunger
	(
	)
	{
	if (!hunger)			// Not already there?
		hunger = new Npc_hunger_timer(this);
	}

/*
 *	End hunger.
 */

void Npc_timer_list::end_hunger
	(
	)
	{
	if (hunger)
		{
		delete hunger;
		hunger = 0;
		}
	}

/*
 *	Start poison.
 */

void Npc_timer_list::start_poison
	(
	)
	{
	if (poison)			// Remove old one.
		delete poison;
	poison = new Npc_poison_timer(this);
	}

/*
 *	End poison.
 */

void Npc_timer_list::end_poison
	(
	)
	{
	if (poison)
		{
		delete poison;
		poison = 0;
		}
	}

/*
 *	Get game minute from start.
 */

unsigned long Npc_timer::get_minute
	(
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	return 60*gwin->get_total_hours() + gwin->get_minute();
	}

/*
 *	Start timer.
 */

Npc_timer::Npc_timer
	(
	Npc_timer_list *l
	) : list(l)
	{
	Game_window *gwin = Game_window::get_game_window();
					// Start in 10 secs.
	gwin->get_tqueue()->add(SDL_GetTicks() + 10000, this, 0L);
	}

/*
 *	Be sure we're no longer in the time queue.
 */

Npc_timer::~Npc_timer
	(
	)
	{
	if (in_queue())
		{
		Time_queue *tq = Game_window::get_game_window()->get_tqueue();
		tq->remove(this);
		}
	}

/*
 *	Done with hunger timer.
 */

Npc_hunger_timer::~Npc_hunger_timer
	(
	)
	{
	list->hunger = 0;
	}

/*
 *	Time to penalize for hunger.
 */

void Npc_hunger_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	Actor *npc = list->npc;
					// No longer a party member?
	if ((npc != gwin->get_main_actor() && npc->get_party_id() < 0) ||
					//   or no longer hungry?
	    npc->get_property((int) Actor::food_level) >= 0)
		{
		delete this;
		return;
		}
	unsigned long minute = get_minute();
					// Once/hour.
	if (minute >= last_time + 60)
		{
		int health = npc->get_property((int) Actor::health);
		health -= rand()%3;
		npc->set_property((int) Actor::health, health);
		if (rand()%4)
			npc->say(first_starving, first_starving + 2);
		last_time = minute;
		}
	gwin->get_tqueue()->add(curtime + 30000, this, 0L);
	}

/*
 *	Done with poison timer.
 */

Npc_poison_timer::~Npc_poison_timer
	(
	)
	{
	list->poison = 0;
	}

/*
 *	Initialize poison timer.
 */

Npc_poison_timer::Npc_poison_timer
	(
	Npc_timer_list *l
	) : Npc_timer(l)
	{
	last_time = get_minute();	// Lasts 1-2 hours.
	end_time = last_time + 60 + rand()%60;
	}

/*
 *	Time to penalize for poison, or see if it's worn off.
 */

void Npc_poison_timer::handle_event
	(
	unsigned long curtime, 
	long udata
	)
	{
	Game_window *gwin = Game_window::get_game_window();
	unsigned long minute = get_minute();
	Actor *npc = list->npc;
	if (minute >= end_time ||	// Long enough?  Or cured?
	    npc->get_flag(Actor::poisoned) == 0)
		{
		npc->clear_flag(Actor::poisoned);
		delete this;
		return;
		}
					// Once per 20 minutes.
	if (minute >= last_time + 20)
		{
		int health = npc->get_property((int) Actor::health);
		health -= rand()%3;
		npc->set_property((int) Actor::health, health);
		if (rand()%4)
			npc->say(first_ouch, last_ouch);
		last_time = minute;
		}
					// Check again in 20 secs.
	gwin->get_tqueue()->add(curtime + 20000, this, 0L);
	}

