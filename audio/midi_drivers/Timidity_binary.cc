/*
Copyright (C) 2000  Dancer A.L Vesperman

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


#if (__GNUG__ >= 2) && (!defined WIN32)
#  pragma implementation
#endif

#ifdef XWIN

#include "Midi.h"

#include <unistd.h>
#include <csignal>
#include "fnames.h"
#include "Audio.h"

#include "Configuration.h"
extern	Configuration	*config;


const	Uint32	Timidity_binary_magic=0x345300;

// #undef HAVE_TIMIDITY_BIN	// Damn. Can't do this while SDL has the audio device.
// New strategy - Tell timidity to output to stdout. Capture that via a pipe, and
// introduce it back up to the mixing layer.
#if HAVE_TIMIDITY_BIN
#include "Timidity_binary.h"

static  void    playTBmidifile(const char *name)
{
	const char	*args[]= {
			"timidity",
			"-Oru8S",	// Raw. Unsigned. 8-bit. Stereo
			"-id",
			"-T 175",	// Tempo. Faster than normal to make it
					// sound right
			"-o-",
			name,
			0 };
        // execlp("timidity","-Or","-id","-o-",name,0);
	execvp("timidity",(char *const *)args);
	exit(0);	// Just in case
}

template<class T>
T max(T x,T y)
	{
	return (x>y)?x:y;
	}



static	int	sub_process(void *p)
{
	Timidity_binary *ptr=static_cast<Timidity_binary *>(p);
	ptr->player();
	return 0;
}

void	Timidity_binary::player(void)
{
	Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic);
	ProducerConsumerBuf *audiostream=Audio::get_ptr()->Create_Audio_Stream();
	audiostream->id=Timidity_binary_magic;
	string	s="timidity -Oru8S -id -T 175 -o- "+filename;
	data=popen(s.c_str(),"r");
	if(!data)
		return;
	while(!feof(data))
		{
		char	buf[1024];
		size_t	x=fread(buf,1,sizeof(buf),data);
		if(x==0)
			break;
		audiostream->produce(buf,x);
		}
	audiostream->end_production();
	pclose(data);
	audiostream=0;
	my_thread=0;	// Race?
}



Timidity_binary::Timidity_binary() : my_thread(0),data(0),filename()
	{
		// Figure out if the binary is where we expect it to be.
		
	}

Timidity_binary::~Timidity_binary()
	{
	// Stop any current player
	stop_track();
	}

void	Timidity_binary::stop_track(void)
	{
	if(my_thread)
		{
		Audio::get_ptr()->Destroy_Audio_Stream(Timidity_binary_magic);
		my_thread=0;
		}
	}

bool	Timidity_binary::is_playing(void)
{
	if(Audio::get_ptr()->is_playing(Timidity_binary_magic))
		return true;
	return false;
}

void	Timidity_binary::start_track(const char *name,bool repeat)
{
#if DEBUG
	cerr << "Starting midi sequence with Timidity_binary" << endl;
#endif
        if(my_thread)
                {
#if DEBUG
	cerr << "Stopping any running track" << endl;
#endif
		stop_track();
                }
#if DEBUG
	cerr << "Starting to play " << name << endl;
#endif
	filename=name;
	my_thread=SDL_CreateThread(sub_process,this);
}

const	char *Timidity_binary::copyright(void)
{
	return "Internal cheapass forked timidity synthesiser";
}

#endif // HAVE_TIMIDITY_BIN

#endif // XWIN
