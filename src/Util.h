/************************************************************************************\
This source file is part of the KS(X) audio library                                  *
For latest info, see http://code.google.com/p/libxal/                                *
**************************************************************************************
Copyright (c) 2010 Kresimir Spes (kreso@cateia.com), Boris Mikic                     *
*                                                                                    *
* This program is free software; you can redistribute it and/or modify it under      *
* the terms of the BSD license: http://www.opensource.org/licenses/bsd-license.php   *
\************************************************************************************/
#ifndef XAL_UTIL_H
#define XAL_UTIL_H

#include <hltypes/harray.h>
#include <hltypes/hstring.h>

namespace xal
{
	harray<hstr> getPathFiles(chstr path);
	harray<hstr> getPathDirectories(chstr path);
	harray<hstr> getPathFilesRecursive(chstr path);
	harray<hstr> getPathDirectoriesRecursive(chstr path);
	bool isDirectory(chstr path);
	bool isFile(chstr path);

}

#endif
