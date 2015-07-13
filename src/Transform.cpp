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

#include "Transform.h"

using namespace Animata;

double Transform::modelview[16];
double Transform::projection[16];
GLint Transform::viewport[4];

/**
 * Reads and stores the actual opengl modelview and projection matrices, and viewport parameters.
 */
void Transform::setMatrices()
{
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);
}

/**
 * Projects a given 3D point (in world coordinates) to the screen coordinates
 * based on the previously saved transformation parameters.
 *
 * \param x \e x coordiate of the point
 * \param y \e y coordiate of the point
 * \param z \e z coordiate of the point
 */
Vector3D Transform::project(float x, float y, float z)
{
	double wx, wy, wz;

	gluProject(x, y, z, modelview, projection, viewport, &wx, &wy, &wz);

	return Vector3D(wx, wy, wz);
}

/**
 * Unprojects a given 3D point (in screen coordinates) to world coordinates
 * based on the previously saved transformation parameters.
 *
 * \param x \e x coordiate of the point
 * \param y \e y coordiate of the point
 * \param z \e z coordiate of the point
 */
Vector3D Transform::unproject(float x, float y, float z)
{
	double vx, vy, vz;

	gluUnProject((double)x, (double)viewport[3] - (double)y, (double)z, modelview, projection, viewport, &vx, &vy, &vz);

	return Vector3D(vx, vy, vz);
}

/**
 * Gets the depth value from the depthbuffer at the given pixel.
 *
 * \param x \e x coordiate of the point
 * \param y \e y coordiate of the point
 */
float Transform::getDepth(float x, float y)
{
	float depth;

	glReadPixels((GLint)x, (GLint)(viewport[3] - y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);

	return depth;
}

