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

#include "Face.h"

using namespace Animata;

void Face::move(float dx, float dy)
{
	v[0]->coord.x += dx;
	v[0]->coord.y += dy;
	v[1]->coord.x += dx;
	v[1]->coord.y += dy;
	v[2]->coord.x += dx;
	v[2]->coord.y += dy;
}

Vector2D Face::center(void)
{
	Vector2D c;

	c.x = (v[0]->coord.x + v[1]->coord.x + v[2]->coord.x) / 3.0;
	c.y = (v[0]->coord.y + v[1]->coord.y + v[2]->coord.y) / 3.0;

	return c;
}

void Face::attachTexture(Texture *t)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	float scale = t->getScale();

	float sx = (float)t->width * scale;
	float sy = (float)t->height * scale;

	for(unsigned int i = 0; i < 3; i++)
	{
		v[i]->texCoord.x = (v[i]->coord.x - t->x) / sx;
		v[i]->texCoord.y = (v[i]->coord.y - t->y) / sy;

		// v[i]->texCoord.x = (v[i]->view.x - t->x) / sx;
		// v[i]->texCoord.y = (viewport[3] - v[i]->view.y - t->y) / sy;
	}
}

