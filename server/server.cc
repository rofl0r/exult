/**
 **	Server.cc - Server functions for Exult (NOT ExultStudio).
 **
 **	Written: 5/2/2001 - JSF
 **/

/*
Copyright (C) 2000-2001 The Exult Team

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
#  include <config.h>	// All the ifdefs aren't useful if we don't do this
#endif

/*
 *	For the time being, we'll only inflict this on X users.
 */
#ifdef USE_EXULTSTUDIO

#include <unistd.h>
#include <fcntl.h>

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <cstdio>

#ifdef _AIX
#include <strings.h>
#endif

#if HAVE_NETDB_H
#include <netdb.h>
#endif

#include <sys/un.h>

#include "server.h"
#include "servemsg.h"
#include "utils.h"
#include "egg.h"
#include "actors.h"
#include "gamewin.h"
#include "gamemap.h"
#include "chunkter.h"
#include "cheat.h"

using std::cout;
using std::cerr;
using std::endl;
/*
 *	Sockets, etc.
 */
extern int xfd;				// X-windows fd.
int listen_socket = -1;			// Listen here for map-editor.
int client_socket = -1;			// Socket to the map-editor.
int highest_fd = -1;			// Largest fd + 1.

#ifdef __sun__
// Solaris doesn't know PF_LOCAL
#define PF_LOCAL PF_UNIX
#endif

/*
 *	Set the 'highest_fd' value to 1 + <largest fd>.
 */
inline void Set_highest_fd
	(
	)
	{
	highest_fd = xfd;		// Figure highest to listen to.
	if (listen_socket > highest_fd)
		highest_fd = listen_socket;
	if (client_socket > highest_fd)
		highest_fd = client_socket;
	highest_fd++;			// Select wants 1+highest.
	}

/*
 *	Initialize server for map-editing.  Call this AFTER xfd has been set.
 */

void Server_init
	(
	)
	{
	// Get location of socket file.
	std::string servename = get_system_path("<GAMEDAT>/exultserver");
	// Make sure it isn't there.
	unlink(servename.c_str());
#if HAVE_GETADDRINFOX
	// Don't use the old deprecated network API
	int r;
	struct addrinfo hints,*ai;

	memset(&hints,0,sizeof(hints));
	hints.ai_flags=AI_PASSIVE;
	r=getaddrinfo(	0,
			servename.c_str(),
			&hints,
			&ai);
	if(r!=0)
		{
		cerr << "getaddrinfo(): "<<gai_strerror(r) << endl;
		return;
		}

	listen_socket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
#else
	// Deprecated
	listen_socket = socket(PF_LOCAL, SOCK_STREAM, 0);
#endif
	if (listen_socket < 0)
		perror("Failed to open map-editor socket");
	else 
		{
#if HAVE_GETADDRINFOX
		if(bind(listen_socket,ai->ai_addr,ai->ai_addrlen) == -1 ||
			listen(listen_socket,1) == -1)
#else
		struct sockaddr_un addr;
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, servename.c_str());
		if (bind(listen_socket, (struct sockaddr *) &addr, 
		      sizeof(addr.sun_family) + strlen(addr.sun_path)) == -1 ||
		    listen(listen_socket, 1) == -1)
#endif
			{
			perror("Bind or listen on socket failed");
			close(listen_socket);
			listen_socket = -1;
			}
		else			// Set to be non-blocking.
			{
			cout << "Listening on socket " << listen_socket
								<< endl;
			fcntl(listen_socket, F_SETFL, 
				fcntl(listen_socket, F_GETFL) | O_NONBLOCK);
			}
#if HAVE_GETADDRINFOX
		freeaddrinfo(ai);
#endif
		}
	Set_highest_fd();
	}

/*
 *	Close the server.
 */

void Server_close
	(
	)
	{
	// unlink socket file+++++++
	}

/*
 *	A message from a client is available, so handle it.
 */

static void Handle_client_message
	(
	int& fd				// Socket to client.  May be closed.
	)
	{
	unsigned char data[Exult_server::maxlength];
	Exult_server::Msg_type id;
	int datalen = Exult_server::Receive_data(fd, id, data, sizeof(data));
	if (datalen < 0)
		return;
	unsigned char *ptr = &data[0];
	Game_window *gwin = Game_window::get_game_window();
	switch (id)
		{
	case Exult_server::obj:
		Game_object::update_from_studio(&data[0], datalen);
		break;
	case Exult_server::egg:
		Egg_object::update_from_studio(&data[0], datalen);
		break;
	case Exult_server::npc:
		Actor::update_from_studio(&data[0], datalen);
		break;
	case Exult_server::num_npcs:
		{
		unsigned char data[16];
		unsigned char *ptr = &data[0];
		Write2(ptr, gwin->get_num_npcs());
		Send_data(client_socket, Exult_server::num_npcs, data,
							ptr - data);
		break;
		}
	case Exult_server::write_map:
		gwin->write_map();	// Send feedback?+++++
		break;
	case Exult_server::read_map:
		gwin->read_map();
		break;
	case Exult_server::map_editing_mode:
		{
		int onoff = Read2(ptr);
		if ((onoff != 0) != cheat.in_map_editor())
			cheat.toggle_map_editor();
		break;
		}
	case Exult_server::tile_grid:
		{
		int onoff = Read2(ptr);
		if ((onoff != 0) != cheat.show_tile_grid())
			cheat.toggle_tile_grid();
		break;
		}
	case Exult_server::edit_lift:
		{
		int lift = Read2(ptr);
		cheat.set_edit_lift(lift);
		break;
		}
	case Exult_server::reload_usecode:
		gwin->reload_usecode();
		break;
	case Exult_server::locate_terrain:
		{
		int tnum = Read2(ptr);
		int cx = (short) Read2(ptr);
		int cy = (short) Read2(ptr);
		bool up = *ptr++ ? true : false;
		bool okay = gwin->get_map()->locate_terrain(tnum, cx, cy, up);
		ptr = &data[2];		// Set back reply.
		Write2(ptr, cx);
		Write2(ptr, cy);
		ptr++;			// Skip 'up' flag.
		*ptr++ = okay ? 1 : 0;
		Send_data(client_socket, Exult_server::locate_terrain, data,
							ptr - data);
		break;
		}
	case Exult_server::swap_terrain:
		{
		int tnum = Read2(ptr);
		bool okay = gwin->get_map()->swap_terrains(tnum);
		*ptr++ = okay ? 1 : 0;
		Send_data(client_socket, Exult_server::swap_terrain, data,
							ptr - data);
		break;
		}
	case Exult_server::insert_terrain:
		{
		int tnum = (short) Read2(ptr);
		bool dup = *ptr++ ? true : false;
		bool okay = gwin->get_map()->insert_terrain(tnum, dup);
		*ptr++ = okay ? 1 : 0;
		Send_data(client_socket, Exult_server::insert_terrain, data,
							ptr - data);
		break;
		}
	case Exult_server::send_terrain:
		{			// Send back #, total, 512-bytes data.
		int tnum = (short) Read2(ptr);
		Write2(ptr, gwin->get_map()->get_num_chunk_terrains());
		Chunk_terrain *ter = gwin->get_map()->get_terrain(tnum);
		ter->write_flats(ptr);	// Serialize it.
		ptr += 512;		// I just happen to know the length...
		Send_data(client_socket, Exult_server::send_terrain, data,
							ptr - data);
		break;
		}
		}
	}

/*
 *	Delay for a fraction of a second, or until there's data available.
 *	If a server request comes, it's handled here.
 */

void Server_delay
	(
	)
	{
	extern int xfd;
	fd_set rfds;
	struct timeval timer;
	timer.tv_sec = 0;
	timer.tv_usec = 50000;		// Try 1/50 second.
	FD_ZERO(&rfds);
	FD_SET(xfd, &rfds);
	if (listen_socket >= 0)
		FD_SET(listen_socket, &rfds);
	if (client_socket >= 0)
		FD_SET(client_socket, &rfds);
					// Wait for timeout or event.
	if (select(highest_fd, &rfds, 0, 0, &timer) > 0)
		{			// Something's come in.
		if (listen_socket >= 0 && FD_ISSET(listen_socket, &rfds))
			{		// New client connection.
					// For now, just one at a time.
			if (client_socket >= 0)
				close(client_socket);
			client_socket = accept(listen_socket, 0, 0);
			cout << "Accept returned client_socket = " <<
							client_socket << endl;
					// Non-blocking.
			fcntl(client_socket, F_SETFL, 
				fcntl(client_socket, F_GETFL) | O_NONBLOCK);
			Set_highest_fd();
			}
		if (client_socket >= 0 && FD_ISSET(client_socket, &rfds))
			{
			Handle_client_message(client_socket);
					// Client gone?
			if (client_socket == -1)
				Set_highest_fd();
			}
		}
	}

#endif
