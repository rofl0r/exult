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

#include "Midi.h"

#include <unistd.h>
#include <csignal>
#include <iostream>
#include "../fnames.h"
#include "../files/U7file.h"
#include "xmidi.h"

#include "Configuration.h"
extern	Configuration	*config;


void    MyMidiPlayer::start_track(int num,bool repeat,int bank)
{
  #if DEBUG
        cout << "Audio subsystem request: Music track # " << num << endl;
  #endif
	U7object	track(midi_bank[bank].c_str(),num);

	if (!midi_device)
	        return;

#ifdef WIN32
	//stop track before writing to temp. file
	midi_device->stop_track();
#endif
	
	char		*buffer;
	size_t		size;
	
	if(!track.retrieve(&buffer, size))
	        return;

	XMIDI		midfile((unsigned char *)buffer, size);
	
	if(!midfile.retrieve(0, MIDITMPFILE))
	        return;
	
	midi_device->start_track(MIDITMPFILE,repeat);
}

void    MyMidiPlayer::start_track(const char *fname,int num,bool repeat)
{
  #if DEBUG
        cout << "Audio subsystem request: Music track # " << num << " in file "<< fname << endl;
  #endif

	if (!midi_device || !fname)
	        return;

#ifdef WIN32
	//stop track before writing to temp. file
	midi_device->stop_track();
#endif
	
	XMIDI		midfile(fname);
	
	if(!midfile.retrieve(num, MIDITMPFILE))
	        return;
	
	midi_device->start_track(MIDITMPFILE,repeat);
}

void	MyMidiPlayer::start_music(int num,bool repeat,int bank)
{
	if(!midi_device)
		return;
	if(current_track==num&&midi_device->is_playing())
		return;	// Already playing it
	current_track=num;
	start_track(num,repeat,bank);
}

void	MyMidiPlayer::start_music(const char *fname,int num,bool repeat)
{
	if(!midi_device || !fname)
		return;
	current_track=-1;
	start_track(fname,num,repeat);
}

bool	MyMidiPlayer::add_midi_bank(const char *bankname)
{
	string	bank(bankname);
	midi_bank.push_back(bank);
	return true;
}


#if HAVE_TIMIDITY_BIN
  #include "midi_drivers/Timidity_binary.h"
#endif
#if HAVE_LIBKMIDI
  #include "midi_drivers/KMIDI.h"
#endif
#ifndef MACOS
  #include "midi_drivers/forked_player.h"
#endif
#ifdef WIN32
  #include "midi_drivers/win_MCI.h"
#endif
#ifdef BEOS
  #include "midi_drivers/be_midi.h"
#endif
#ifdef MACOS
  #include "midi_drivers/mac_midi.h"
#endif

#define	TRY_MIDI_DRIVER(CLASS_NAME)	\
	if(no_device) {	\
		try {	\
			midi_device=new CLASS_NAME();	\
			no_device=false;	\
			cerr << midi_device->copyright() << endl;	\
		} catch(...) {	\
			no_device=true;	\
		}	\
	}

MyMidiPlayer::MyMidiPlayer()	: current_track(-1),midi_device(0)
{
	bool	no_device=true;

	add_midi_bank(MAINMUS);
	add_midi_bank(INTROMUS);

	// instrument_patches=AccessTableFile(XMIDI_MT);
	string	s;
	config->value("config/audio/midi/enabled",s,"---");
	if(s=="---")
		{
		cout << "Config does not specify MIDI. Assuming yes" << endl;
		s="yes";
		}
	if(s=="no")
		{
		cout << "Config says no midi. MIDI disabled" << endl;
		no_device=false;
		}
	config->set("config/audio/midi/enabled",s,true);

#ifdef WIN32
	TRY_MIDI_DRIVER(Windows_MCI)
#endif
#ifdef BEOS
	TRY_MIDI_DRIVER(Be_midi)
#endif
#if HAVE_TIMIDITY_BIN
	TRY_MIDI_DRIVER(Timidity_binary)
#endif
#if HAVE_LIBKMIDI
	TRY_MIDI_DRIVER(KMIDI)
#endif
#ifdef XWIN
	TRY_MIDI_DRIVER(forked_player)
#endif
#ifdef MACOS
	TRY_MIDI_DRIVER(Mac_QT_midi)
#endif

	if(no_device)
		{
		midi_device=0;
		cerr << "Unable to create a music device. No music will be played" << endl;
		}
}

MyMidiPlayer::~MyMidiPlayer()
{
	if(midi_device&&midi_device->is_playing())
		midi_device->stop_track();
}
