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

#ifndef __VECTOR3D_H__
#define __VECTOR3D_H__

namespace Animata
{

class Matrix;

class Vector3D
{
	public:
		float x, y, z;

		Vector3D();
		Vector3D(float x, float y, float z);
		Vector3D(Vector3D *p);

		inline void set(float _x, float _y, float _z) { x = _x; y = _y; }

		int operator == (Vector3D &p);
		int operator != (Vector3D &p);
		Vector3D& operator += (Vector3D&);
		Vector3D& operator -= (Vector3D&);

		Vector3D operator = (Vector3D&);

		Vector3D operator * (float m);

		Vector3D& transform(Matrix& m);
		Vector3D& rotate(Matrix& m);

		void normalize(void);
		float size(void);
};

} /* namespace Animata */

#endif

