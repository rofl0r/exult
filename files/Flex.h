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

#ifndef	__FLEX_H_
#define	__FLEX_H_

#include <vector>
#include <string>
#include "exult_types.h"
#include "U7file.h"


class Flex : public U7file
{
protected:
	char	title[80];
	uint32	magic1;
	uint32	count;
	uint32	magic2;
	uint32	padding[9];
	struct Reference
	{
		uint32 offset;
		uint32 size;
		Reference() : offset(0),size(0) {};
	};
	std::vector<Reference> object_list;

public:
	Flex(const std::string &fname);
	Flex(const Flex &f) : magic1(f.magic1),count(f.count),magic2(f.magic2),object_list(f.object_list)
		{ std::memcpy(title,f.title,sizeof(title));
		  std::memcpy(padding,f.padding,sizeof(padding)); }
	Flex &operator=(const Flex &f)
		{
			magic1=f.magic1;
			count=f.count;
			magic2=f.magic2;
			object_list=f.object_list;
			std::memcpy(title,f.title,sizeof(title));
			std::memcpy(padding,f.padding,sizeof(padding));
			return *this;
		}
		
   virtual uint32	number_of_objects(void) { return object_list.size(); };
   virtual	char *	retrieve(uint32 objnum,std::size_t &len); // To a memory block
	virtual const char *get_archive_type() { return "FLEX"; };
					// Write header for a Flex file.
	static void write_header(ostream& out, const char *title, int count);
private:
	Flex();	// No default constructor
	void IndexFlexFile(void);
};


#endif
