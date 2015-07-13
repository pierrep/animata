/*
 Animata

 Copyright (C) 2007 Peter Nemeth, Gabor Papp, Bence Samu
 Kitchen Budapest, <http://animata.kibu.hu/>

 This file is part of Animata.

 Animata is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Animata is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Animata. If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef __IMAGEBOX_H__
#define __IMAGEBOX_H__

#include <FL/Fl_Box.H>
#include <FL/Fl_Image.H>

#include "Texture.h"

#ifndef PATH_MAX
	#define PATH_MAX 4096
#endif

namespace Animata
{

class ImageBox : public Fl_Box
{
		Fl_Image* origImage;
		Fl_Image* boxImage;

		int handle(int);
		void handleRelease(void);

		char filename[PATH_MAX+1];

	public:
		ImageBox(const char *filename, Fl_Image* i, int x, int y, int w,
				int h = 0, const char* label = 0);

		/// adds texture to texture manager
		void addTexture(void);

		/// returns a newly allocated texture object based on the image stored in this box
		Texture *allocateTexture();

		inline char *getFilename() { return filename; }
};

} /* namespace Animata */

#endif

