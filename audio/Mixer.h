//-*-Mode: C++;-*-
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

#ifndef _Mixer_h_
#define _Mixer_h_

#if __GNUG__ >= 2
#  pragma interface
#endif

#if !AUTOCONFIGURED
#include "../autoconfig.h"
#endif

#include <list>
#include "SDL_mapping.h"
#include <SDL_audio.h>
#include "Flex.h"

//---- Mixer -----------------------------------------------------------

class Mixer 
{
public:
    Mixer();
	Mixer(unsigned int, unsigned int, unsigned char);
    Mixer(size_t ringsize,size_t bufferlength);
    ~Mixer();

	struct	MixBuffer
		{
		Uint8 *buffer;
		Uint8 num_samples;
		size_t	length;
		MixBuffer(size_t size,Uint8 silence) : buffer(new Uint8[size]),num_samples(0),length(0) { memset(buffer,silence,size); };
		};
	size_t	buffer_length;
	list<MixBuffer>	buffers;
	size_t ring_size;
	void	advance(void);
	Uint8	silence;
	void fill_audio_func(void *, Uint8 *, int);
	void play(Uint8 *, unsigned int);
};


#endif
