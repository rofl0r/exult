/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef GAME_H
#define GAME_H

#include <hash_map>
#include <vector>

class Game_window;
class Image_window8;

struct eqstr
{
	bool operator()(const char* s1, const char* s2) const {
		return strcmp(s1, s2) == 0;
	}
};

struct str_int_pair
{
	char *str;
	int  num;
};

class Game
	{
private:
	hash_map<const char*, int, hash<const char*>, eqstr> shapes;
	hash_map<const char*, str_int_pair, hash<const char*>, eqstr> resources;
protected:
	int topx, topy, centerx, centery;
public:
	Game_window *gwin;
	Image_window8 *win;
	Palette pal;

	Game();
	virtual ~Game();
	
	static Game *create_game(const char *identity);
	static Game *get_game();
	static Exult_Game get_game_type();
	
	virtual void play_intro() =0;
	virtual void end_game(bool success) =0;
	virtual void top_menu(Vga_file &shapes) =0;
	virtual void show_quotes() =0;
	virtual void show_credits() =0;
	virtual bool new_game(Vga_file &shapes) =0;
	virtual int  get_start_tile_x() =0;
	virtual int  get_start_tile_y() =0;
	virtual const char * get_extra_shape_file() =0;
	
	void clear_screen();
	void refresh_screen();
	void play_flic(const char *archive, int index);
	void play_audio(const char *archive, int index);
	void play_midi(int track);
	bool wait_delay(int ms);
	void add_shape(const char *name, int shapenum);
	int get_shape(const char *name);
	void add_resource(const char *name, const char *str, int num);
	str_int_pair get_resource(const char *name);
	void show_text_line(int x, int y, const char *s);
	vector<char *> *load_text(const char *archive, int index);
	void destroy_text(vector<char *> *text);
	void show_menu();
	};

class BG_Game: public Game
	{
public:
	BG_Game();
	~BG_Game();
	
	virtual void play_intro();
	virtual void end_game(bool success);
	virtual void top_menu(Vga_file &shapes);
	virtual void show_quotes();
	virtual void show_credits();
	virtual bool new_game(Vga_file &shapes);
	virtual int  get_start_tile_x()
		{ return (64*tiles_per_chunk); }
	virtual int  get_start_tile_y()
		{ return (136*tiles_per_chunk); }
	virtual const char * get_extra_shape_file() 
		{ return "static/endshape.flx"; }
	};

class SI_Game: public Game
	{
public:
	SI_Game();
	~SI_Game();
	
	virtual void play_intro();
	virtual void end_game(bool success);
	virtual void top_menu(Vga_file &shapes);
	virtual void show_quotes();
	virtual void show_credits();
	virtual bool new_game(Vga_file &shapes);
	virtual int  get_start_tile_x()
		{ return (25*tiles_per_chunk); }
	virtual int  get_start_tile_y()
		{ return (155*tiles_per_chunk); }
	virtual const char * get_extra_shape_file() 
		{ return "static/paperdol.vga"; }
	};
	
#endif
