/*
 *	utils.h - Common utility routines.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2002  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <iostream>
#include <string>
#include <cstring>
#include <stdio.h>
#include <iosfwd>

#include "common_types.h"
#include "rect.h"

#ifndef HAVE_SNPRINTF
extern int snprintf(char *, size_t, const char *, /*args*/ ...);
#endif

/*
 *	Read a 1-byte value.
 */

inline uint8 Read1 (std::istream &in)
{
	return static_cast<uint8>(in.get());
}

/*
 *	Read a 2-byte value, lsb first.
 */

inline uint16 Read2 (std::istream &in)
{
	uint32 val = in.get();
	val |= (in.get()<<8);
	return static_cast<uint16>(val);
}

/*
 *	Read a 2-byte value from a buffer.
 */

inline uint16 Read2
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	return static_cast<uint16>(b0 | (b1 << 8));
	}

/*
 *	Read a 2-byte value from a file.
 */

inline uint16 Read2
	(
	std::FILE* in
	)
	{
	uint8 b0, b1;
	if (std::fread(&b0,sizeof(uint8),1,in) != 1) b0 = 0;
	if (std::fread(&b1,sizeof(uint8),1,in) != 1) b1 = 0;
	return static_cast<uint16>(b0 | (b1 << 8));
	}

/*
 *	Read a 2-byte value, hsb first.
 */

inline uint16 Read2high (std::istream &in)
{
	uint32 val = in.get()<<8;
	val |= in.get();
	return static_cast<uint16>(val);
}

/*
 *	Read a 2-byte value from a buffer.
 */

inline uint16 Read2high
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	return static_cast<uint16>((b0 << 8) | b1);
	}

/*
 *	Read a 2-byte value from a file.
 */

inline uint16 Read2high
	(
	std::FILE* in
	)
	{
	uint8 b0, b1;
	if (std::fread(&b0,sizeof(uint8),1,in) != 1) b0 = 0;
	if (std::fread(&b1,sizeof(uint8),1,in) != 1) b1 = 0;
	return static_cast<uint16>((b0 << 8) | b1);
	}

/*
 *	Read a 4-byte long value, lsb first.
 */

inline uint32 Read4 (std::istream &in)
{
	uint32 val = 0;
	val |= static_cast<uint32>(in.get());
	val |= static_cast<uint32>(in.get()<<8);
	val |= static_cast<uint32>(in.get()<<16);
	val |= static_cast<uint32>(in.get()<<24);
	return val;
}

/*
 *	Read a 4-byte value from a buffer.
 */

inline uint32 Read4
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	uint8 b2 = *in++;
	uint8 b3 = *in++;
	return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	}

/*
 *	Read a 4-byte value from a file.
 */

inline uint32 Read4
	(
	std::FILE* in
	)
	{
	uint8 b0, b1, b2, b3;
	if (std::fread(&b0,sizeof(uint8),1,in) != 1) b0 = 0;
	if (std::fread(&b1,sizeof(uint8),1,in) != 1) b1 = 0;
	if (std::fread(&b2,sizeof(uint8),1,in) != 1) b2 = 0;
	if (std::fread(&b3,sizeof(uint8),1,in) != 1) b3 = 0;
	return (b0 | (b1<<8) | (b2<<16) | (b3<<24));
	}

/*
 *	Read a 4-byte long value, hsb first.
 */
inline uint32 Read4high (std::istream &in)
{
	uint32 val = 0;
	val |= static_cast<uint32>(in.get()<<24);
	val |= static_cast<uint32>(in.get()<<16);
	val |= static_cast<uint32>(in.get()<<8);
	val |= static_cast<uint32>(in.get());
	return val;
}

/*
 *	Read a 4-byte value from a buffer.
 */
inline uint32 Read4high
	(
	uint8 *& in
	)
	{
	uint8 b0 = *in++;
	uint8 b1 = *in++;
	uint8 b2 = *in++;
	uint8 b3 = *in++;
	return ((b0<<24) | (b1<<16) | (b2<<8) | b3);
	}

/*
 *	Read a 4-byte value from a file.
 */
inline uint32 Read4high
	(
	std::FILE* in
	)
	{
	uint8 b0, b1, b2, b3;
	if (std::fread(&b0,sizeof(uint8),1,in) != 1) b0 = 0;
	if (std::fread(&b1,sizeof(uint8),1,in) != 1) b1 = 0;
	if (std::fread(&b2,sizeof(uint8),1,in) != 1) b2 = 0;
	if (std::fread(&b3,sizeof(uint8),1,in) != 1) b3 = 0;
	return ((b0<<24) | (b1<<16) | (b2<<8) | b3);
	}

inline int ReadInt(std::istream& in, int def = 0)
	{
	int num;
	if (in.eof())
		return def;
	in >> num;
	if (in.fail())
		return def;
	in.ignore(0xffffff, '/');
	return num;
	}

inline unsigned int ReadUInt(std::istream& in, unsigned int def = 0)
	{
	unsigned int num;
	if (in.eof())
		return def;
	in >> num;
	if (in.fail())
		return def;
	in.ignore(0xffffff, '/');
	return num;
	}

inline void WriteInt
	(
	std::ostream& out,
	int num,
	bool final = false
	)
	{
	out << num;
	if (final)
		out << std::endl;
	else
		out << '/';
	}

inline void WriteInt
	(
	std::ostream& out,
	unsigned int num,
	bool final = false
	)
	{
	out << num;
	if (final)
		out << std::endl;
	else
		out << '/';
	}

inline void ReadRect
	(
	std::istream& in,
	Rectangle& rect
	)
	{
	rect.x = ReadInt(in);
	rect.y = ReadInt(in);
	rect.w = ReadInt(in);
	rect.h = ReadInt(in);
	}

inline void WriteRect
	(
	std::ostream& out,
	const Rectangle& rect,
	bool final = false
	)
	{
	WriteInt(out, rect.x);
	WriteInt(out, rect.y);
	WriteInt(out, rect.w);
	WriteInt(out, rect.h, final);
	}

inline std::string ReadStr(char *&eptr, int off = 1)
	{
	eptr += off;
	char *pos = std::strchr(eptr, '/');
	char buf[150];
	std::strncpy(buf, eptr, pos - eptr);
	buf[pos - eptr] = 0;
	eptr = pos;
	return std::string(buf);
	}

inline std::string ReadStr(std::istream& in)
	{
	char buf[250];
	in.getline(buf, sizeof(buf)/sizeof(buf[0]), '/');
	std::streamsize size = in.gcount();
	buf[size] = 0;
	return std::string(buf);
	}

inline void WriteStr
	(
	std::ostream& out,
	std::string str,
	bool final = false
	)
	{
	out << str;
	if (final)
		out << std::endl;
	else
		out << '/';
	}

/*
 *	Write a 1-byte value.
 */

inline void Write1(std::ostream& out, uint16 val)
{
	out.put(static_cast<char> (val&0xff));
}

/*
 *	Write a 2-byte value, lsb first.
 */

inline void Write2
	(
	std::ostream& out,
	uint16 val
	)
	{
	out.put(static_cast<char> (val&0xff));
	out.put(static_cast<char> ((val>>8)&0xff));
	}

/*
 *	Write a 2-byte value, msb first.
 */

inline void Write2high
	(
	std::ostream& out,
	uint16 val
	)
	{
	out.put(static_cast<char> ((val>>8)&0xff));
	out.put(static_cast<char> (val&0xff));
	}

/*
 *	Write a 2-byte value to a buffer, lsb first.
 */

inline void Write2
	(
	uint8 *& out,		// Write here and update.
	uint16 val
	)
	{
	*out++ = static_cast<char> (val & 0xff);
	*out++ = static_cast<char> ((val>>8) & 0xff);
	}

/*
 *	Write a 4-byte value, lsb first.
 */

inline void Write4
	(
	std::ostream& out,
	uint32 val
	)
	{
	out.put(static_cast<char> (val&0xff));
	out.put(static_cast<char> ((val>>8)&0xff));
	out.put(static_cast<char> ((val>>16)&0xff));
	out.put(static_cast<char> ((val>>24)&0xff));
	}

/*
 *	Write a 4-byte value, msb first.
 */

inline void Write4high
	(
	std::ostream& out,
	uint32 val
	)
	{
	out.put(static_cast<char> ((val>>24)&0xff));
	out.put(static_cast<char> ((val>>16)&0xff));
	out.put(static_cast<char> ((val>>8)&0xff));
	out.put(static_cast<char> (val&0xff));
	}

/*
 *	Write a 4-byte value to a buffer, lsb first.
 */

inline void Write4
	(
	uint8 *& out,		// Write here and update.
	uint32 val
	)
	{
	*out++ = static_cast<char> (val & 0xff);
	*out++ = static_cast<char> ((val>>8) & 0xff);
	*out++ = static_cast<char> ((val>>16)&0xff);
	*out++ = static_cast<char> ((val>>24)&0xff);
	}

inline void Write4s(std::ostream& out, sint32 val)
{
	Write4(out, static_cast<uint32>(val));
}

bool U7open
	(
	std::ifstream& in,			// Input stream to open.
	const char *fname,			// May be converted to upper-case.
	bool is_text = false			// Should the file be opened in text mode
	);
bool U7open
	(
	std::ofstream& out,			// Output stream to open.
	const char *fname,			// May be converted to upper-case.
	bool is_text = false			// Should the file be opened in text mode
	);

std::FILE* U7open
	(
	const char *fname,			// May be converted to upper-case.
	const char *mode			// File access mode.
	);

bool U7open_static
	(
	std::ifstream& in,		// Input stream to open.
	const char *fname,		// May be converted to upper-case.
	bool is_text			// Should file be opened in text mode
	);
void U7remove(
	const char *fname
	);

bool U7exists(
	const char *fname
	);

inline bool U7exists(std::string fname) { return U7exists(fname.c_str()); }

#ifdef UNDER_CE
inline void perror(const char *errmsg) { return; }
#define strdup myce_strdup
char *myce_strdup(const char *s);

#endif

int U7mkdir(
	const char *dirname,
	int mode
	);

// These are not supported in WinCE (PocketPC) for now
#ifndef UNDER_CE

int U7chdir(
	const char *dirname
	);

void U7copy
	(
	const char *src,
	const char *dest
	);

#endif //UNDER_CE

bool is_system_path_defined(const std::string& key);
void store_system_paths();
void reset_system_paths();
void clear_system_path(const std::string& key);
void add_system_path(const std::string& key, const std::string& value);
void clone_system_path(const std::string& new_key, const std::string& old_key);
std::string get_system_path(const std::string &path);

void to_uppercase(std::string &str);
std::string to_uppercase(const std::string &str);

int Log2(uint32 n);
uint32 msb32(uint32 n);
int fgepow2(uint32 n);

char *newstrdup(const char *s);
char *Get_mapped_name(const char *from, int num, char *to);
int Find_next_map(int start, int maxtry);


inline int bitcount (unsigned char n)
	{
#define TWO(c)     (0x1u << (c))
#define MASK(c)    ((static_cast<uint32>(-1)) / (TWO(TWO(c)) + 1u))
#define COUNT(x,c) ((x) & MASK(c)) + (((x) >> (TWO(c))) & MASK(c))
	// Only works for 8-bit numbers.
	n = static_cast<unsigned char> (COUNT(n, 0));
	n = static_cast<unsigned char> (COUNT(n, 1));
	n = static_cast<unsigned char> (COUNT(n, 2));
	return n;
	}

#endif	/* _UTILS_H_ */
