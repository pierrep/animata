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

#include <math.h>
#include <float.h>
#include "Vector2D.h"

using namespace Animata;

Vector2D::Vector2D()
{
	x = y = 0.f;
}

Vector2D::Vector2D(float x, float y)
{
	this->x = x;
	this->y = y;
}

int Vector2D::operator == (Vector2D &p)
{
	return (((x - p.x)*(x - p.x) + (y - p.y)*(y - p.y)) < FLT_EPSILON);
}

int Vector2D::operator != (Vector2D &p)
{
	return (((x - p.x)*(x - p.x) + (y - p.y)*(y - p.y)) >= FLT_EPSILON);
}

/*
Vector2D& Vector2D::operator + (Vector2D& v)
{
	return new Vector2D(x + v.x, y + v.y);
}

Vector2D& Vector2D::operator - (Vector2D& v)
{
	return new Vector2D(x - v.x, y - v.y);
}
*/

Vector2D& Vector2D::operator += (Vector2D& v)
{
	x += v.x;
	y += v.y;
	return *this;
}

Vector2D& Vector2D::operator -= (Vector2D& v)
{
	x -= v.x;
	y -= v.y;
	return *this;
}

Vector2D Vector2D::operator = (Vector2D& v)
{
	x = v.x;
	y = v.y;
	return Vector2D(x, y);
}

Vector2D Vector2D::operator * (float m)
{
	return Vector2D(x * m, y * m);
}

float Vector2D::cross(Vector2D& v1, Vector2D& v2)
{
	return v1.x*v2.x + v1.y*v2.y;
}

float Vector2D::dot(Vector2D& v1, Vector2D& v2)
{
	return v1.x*v2.y - v1.y*v2.x;
}

void Vector2D::normalize(void)
{
	float len = sqrt(x*x + y*y);

	if(len != 0.f)
	{
		x /= len;
		y /= len;
	}
}

float Vector2D::size(void)
{
	return sqrt(x*x + y*y);
}

