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



#include "Flex.h"

#include <cstdio>
#include <iostream>

Flex	AccessFlexFile(const char *name)
{
	Flex	ret;
	FILE	*fp;
	ret.filename=name;
	fp=fopen(name,"rb");
	if(!fp)
		{
		return ret;
		}
	fread(ret.title,sizeof(ret.title),1,fp);
	fread(&ret.magic1,sizeof(uint32),1,fp);
	fread(&ret.count,sizeof(uint32),1,fp);
	fread(&ret.magic2,sizeof(uint32),1,fp);
	for(int i=0;i<9;i++)
		fread(&ret.padding[i],sizeof(uint32),1,fp);
#if DEBUGFLEX
	cout << "Title: " << ret.title << endl;
	cout << "Count: " << ret.count << endl;
#endif

	// We should already be there.
	fseek(fp,128,SEEK_SET);
	for(unsigned int i=0;i<ret.count;i++)
		{
		Flex::Reference f;
		fread(&f.offset,sizeof(uint32),1,fp);
		fread(&f.size,sizeof(uint32),1,fp);
#if DEBUGFLEX
		cout << "Item " << i << ": " << f.size << " bytes @ " << f.offset << endl;
#endif
		ret.object_list.push_back(f);
		}
	fclose(fp);
	return ret;
}

char	*Flex::read_object(int objnum,uint32 &length)
{
	if((unsigned)objnum>=object_list.size())
		{
		cerr << "objnum too large in read_object()" << endl;
		return 0;
		}
	FILE	*fp=fopen(filename.c_str(),"rb");
	if(!fp)
		{
		cerr << "File open failed in read_object" << endl;
		return 0;
		}
	fseek(fp,object_list[objnum].offset,SEEK_SET);
	length=object_list[objnum].size;
	char	*ret=new char[length];
	fread(ret,length,1,fp);
	fclose(fp);
	return ret;
}
