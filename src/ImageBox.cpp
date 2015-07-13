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

#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "ImageBox.h"
#include "animataUI.h"

using namespace Animata;

ImageBox::ImageBox(const char *filename, Fl_Image* i, int x, int y, int w,
		int h, const char* label) :
	Fl_Box(x, y, w, h, label)
{
	double aspect = (double)i->h() / (double)i->w();
	int width = w;
	int height = (int)(aspect * width);

	strncpy(this->filename, filename, PATH_MAX);

	origImage = i;
	boxImage = i->copy(width, height);
	this->w(width);
	this->h(height);
	this->image(boxImage);
}

int ImageBox::handle(int event)
{
	switch(event)
	{
		case FL_PUSH:
			return 1;
		case FL_RELEASE:
			handleRelease();
			return 1;
		default:
			return Fl_Box::handle(event);
	}
}

Texture *ImageBox::allocateTexture()
{
	return new Texture(filename,
				origImage->w(), origImage->h(), origImage->d(),
				(unsigned char*)origImage->data()[0]);
}

void ImageBox::addTexture(void)
{
	ui->editorBox->createAttachedTexture(this);
}

void ImageBox::handleRelease()
{
	addTexture();
}

