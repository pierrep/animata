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
#include "Vector3D.h"
#include "Matrix.h"

using namespace Animata;

Vector3D::Vector3D()
{
	x = y = z = 0.f;
}

Vector3D::Vector3D(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3D::Vector3D(Vector3D *p)
{
	this->x = p->x;
	this->y = p->y;
	this->z = p->z;
}

int Vector3D::operator == (Vector3D &p)
{
	return (((x - p.x)*(x - p.x) + (y - p.y)*(y - p.y) + (z - p.z)*(z - p.z)) < FLT_EPSILON);
}

int Vector3D::operator != (Vector3D &p)
{
	return (((x - p.x)*(x - p.x) + (y - p.y)*(y - p.y) + (z - p.z)*(z - p.z)) >= FLT_EPSILON);
}

Vector3D& Vector3D::operator += (Vector3D& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector3D& Vector3D::operator -= (Vector3D& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector3D Vector3D::operator = (Vector3D& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return Vector3D(x, y, z);
}

Vector3D Vector3D::operator * (float m)
{
	return Vector3D(x * m, y * m, z * m);
}

Vector3D& Vector3D::rotate(Matrix& m)
{
	Vector3D *rot = new Vector3D();

	rot->x = x * m[0] + y * m[4] + z * m[8];
	rot->y = x * m[1] + y * m[5] + z * m[9];
	rot->z = x * m[2] + y * m[6] + z * m[10];

	return *rot;
}

Vector3D& Vector3D::transform(Matrix& m)
{
	Vector3D *rot = new Vector3D();

	rot->x = x * m[0] + y * m[4] + z * m[8] + m[12];
	rot->y = x * m[1] + y * m[5] + z * m[9] + m[13];
	rot->z = x * m[2] + y * m[6] + z * m[10] + m[14];

	return *rot;
}

void Vector3D::normalize(void)
{
	float len = sqrt(x*x + y*y + z*z);

	if(len != 0.f)
	{
		x /= len;
		y /= len;
		z /= len;
	}
}

float Vector3D::size(void)
{
	return sqrt(x*x + y*y + z*z);
}

