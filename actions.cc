/**	-*-mode: Fundamental; tab-width: 8; -*-
 **
 **	Actions.cc - Action controllers for actors.
 **
 **	Written: 4/20/2000 - JSF
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

#include "actions.h"
#include "actors.h"

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action.
 */

int Walking_actor_action::handle_event
	(
	Actor *actor
	)
	{
	return actor->walk();
	}

/*
 *	Delete.
 */
Path_walking_actor_action::~Path_walking_actor_action
	(
	)
	{
	delete [] path; 
	}

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action.
 */

int Path_walking_actor_action::handle_event
	(
	Actor *actor
	)
	{
	return actor->step(path[index++]);
	}

/*
 *	Delete.
 */
Sequence_actor_action::~Sequence_actor_action
	(
	)
	{
	for (int i = 0; actions[i]; i++)
		delete actions[i];
	delete actions;
	}

/*
 *	Handle a time event.
 *
 *	Output:	0 if done with this action.
 */

int Sequence_actor_action::handle_event
	(
	Actor *actor
	)
	{
	if (!actions[index])		// Done?
		return (0);
					// Do current action.
	int delay = actions[index]->handle_event(actor);
	if (!delay)
		{
		index++;		// That one's done now.
		delay = 100;		// 1/10 second.
		}
	return (delay);
	}



