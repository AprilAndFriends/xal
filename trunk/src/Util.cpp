/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://libatres.sourceforge.net/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#include <hltypes/harray.h>
#include <hltypes/hstring.h>
#include "Source.h"
#include "AudioManager.h"
#include "Util.h"

//2DO - add #ifdef _WIN32 ?
#include <_dirent_win32.h>
#include <stdio.h>
namespace xal
{
	harray<hstr> getPathFiles(chstr path)
	{
		harray<hstr> result;
		if (isDirectory(path))
		{
			DIR* dir = opendir(path.c_str());
			struct dirent* entry;
			while ((entry = readdir(dir)) != NULL)
			{
				if (isFile(hsprintf("%s/%s", path.c_str(), entry->d_name)))
				{
					result += hstr(entry->d_name);
				}
			}
			closedir(dir);
		}
		return result;
	}
	
	harray<hstr> getPathDirectories(chstr path)
	{
		harray<hstr> result;
		if (isDirectory(path))
		{
			DIR* dir = opendir(path.c_str());
			struct dirent* entry;
			hstr name;
			while ((entry = readdir(dir)) != NULL)
			{
				name = entry->d_name;
				if (isDirectory(hsprintf("%s/%s", path.c_str(), entry->d_name)) && name != "." && name != "..")
				{
					result += name;
				}
			}
			closedir(dir);
		}
		return result;
	}
	
	harray<hstr> getPathFilesRecursive(chstr path)
	{
		harray<hstr> dirs = getPathDirectoriesRecursive(path);
		dirs += path;
		harray<hstr> files;
		for (hstr* it = dirs.iterate(); it; it = dirs.next())
		{
			files += getPathFiles(*it);
		}
		return files;
	}
	
	harray<hstr> getPathDirectoriesRecursive(chstr path)
	{
		harray<hstr> pending;
		pending += path;
		harray<hstr> dirs;
		harray<hstr> current;
		hstr dir;
		while (pending.size() > 0)
		{
			dir = pending.pop_front();
			current = getPathDirectories(dir);
			pending += current;
			dirs += current;
		}
		return dirs;
	}
	
	bool isDirectory(chstr path)
	{
		DIR* dir;
		if ((dir = opendir(path.c_str())) != NULL)
		{
			closedir(dir);
			return true;
		}
		return false;
	}

	bool isFile(chstr path)
	{
		return (!isDirectory(path));
	}

}
