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

#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#if defined(__APPLE__)
	#include <OPENGL/gl.h>
	#include <OPENGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include "Vector3D.h"

namespace Animata
{

/// Transforms points between screen and world coordinate-systems.
class Transform
{
	private:
		static double modelview[16];	///< modelview matrix
		static double projection[16];	///< projection matrix
		static GLint viewport[4];		///< viewport parameters

	public:
		Transform() {}
		virtual ~Transform () {}

		static void setMatrices();

		static Vector3D unproject(float x, float y, float z);
		static Vector3D project(float x, float y, float z);
		static float getDepth(float x, float y);
};

} /* namespace Animata */

#endif

