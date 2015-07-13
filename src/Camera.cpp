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
#include <stdio.h>

#if defined(__APPLE__)
	#include <OPENGL/gl.h>
	#include <OPENGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include "Camera.h"

using namespace Animata;

/**
 * Creates a new camera with default parameters.
 * - \a target = (0, 0, 0)
 * - \a orientation = (0, 0, 0)
 * - \a upvector = (0, -1, 0)
 * - \a distance = 150
 * - \a fov = 90
 * - \a zNear = 0.1
 * - \a zFar = 1000
 */
Camera::Camera()
{
	target.set(0.f, 0.f, 0.f);
	orientation.set(0.f, 0.f, 0.f);
	upvector.set(0.f, -1.f, 0.f);

	distance = 150.f;
	fov = 90.f;

	width = height = 0;
	pictureWidth = pictureHeight = 0;

	zNear = 0.1f;
	zFar = 1000.f;

	init = false;

	parent = NULL;
}

/**
 * Sets up the camera-picture's aspect, and calculates the picture's size.
 * Positions the camera's target if it hasn't been already positioned.
 * If this \a parent of this camera is already set every calculation will be based on the parent's parameters.
 * /param w Width of the view.
 * /param h Height of the view.
 **/
void Camera::setSize(int w, int h)
{
	width = pictureWidth = w;
	height = pictureHeight = h;

	if(!init)
	{
		target.x = width  / 2;
		target.y = height / 2;

		setAspect(width, height);

		if (parent)
		{
			target = *parent->getTarget();
			distance = parent->getDistance();
			fov = parent->getFOV();
		}

		init = true;
	}

	// setAspect(width, height);
	aspect = height > 0 ? (double)width / (double)height : 1.0;

	if (parent)
	{
		int pw = parent->getWidth();
		int ph = parent->getHeight();

		// setAspect(pw, ph);
		aspect = ph > 0 ? (double)pw / (double)ph : 1.0;

		if(width / aspect > height)
		{
			pictureHeight = height;
			pictureWidth = (int)(pictureHeight * aspect);
		}
		else
		{
			pictureWidth = width;
			pictureHeight = (int)(pictureWidth / aspect);
		}
	}

	setupPerspective();
}

/**
 * Calculates the aspect of the picture based on the given parameters.
 * The vertical field-of-view gets also calculated and stored, what is required for an undistorted picture.
 * \param	w	Width of the picture at camera distance from the target.
 * \param	h	Height of the picture at camera distance from the target.
 */
void Camera::setAspect(int w, int h)
{
	aspect = h > 0 ? (double)w / (double)h : 1.0;

	double radtheta, degtheta;
	radtheta = 2.0 * atan2(h/2, distance);
	degtheta = (180.0 * radtheta) / M_PI;

	fov = degtheta;
}

/**
 * Sets up the projection matrix by perspective transformation.
 * Also the viewport is set.
 * In the beginning the projection matrix gets cleared.
 */
void Camera::setupPerspective()
{
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);

	setupViewport();
}

/**
 * Sets up the projection matrix by orthographic transformation.
 * Also the viewport is set.
 * In the beginning the projection matrix gets cleared.
 */
void Camera::setupOrtho()
{
	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
//	glOrtho(0, width, 0, height, -1, 1);
	glOrtho((width - pictureWidth) / 2, width + (width - pictureWidth) / 2, (height - pictureHeight) / 2, height + (height - pictureHeight) / 2, -1, 1);

	setupViewport();
}

/**
 * Sets up the projection matrix which will be used by picking functions.
 * This is done via multiplying the a special picking matrix onto the current projection matrix.
 * /param	x	\e x coordinate of the picking position
 * /param	y	\e y coordinate of the picking position
 * /param	radius	The radius around the given point where picking occures.
 */
void Camera::setupPickingProjection(int x, int y, int radius)
{
	// get the boundaries of actual viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPickMatrix((GLdouble)x, (GLdouble)(viewport[3] - y), (GLdouble)radius, (GLdouble)radius, viewport);

//	glOrtho(0, width, 0, height, -1, 1);
	glOrtho((width - pictureWidth) / 2, width + (width - pictureWidth) / 2, (height - pictureHeight) / 2, height + (height - pictureHeight) / 2, -1, 1);
}

/**
 * Sets up the modelview matrix by gluLookAt(), based on the camera properties \a target, \a distance and \a upvector.
 * In the beginning the modelview matrix gets cleared.
 */
void Camera::setupModelView()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(target.x, target.y, target.z - distance,
			  target.x, target.y, target.z,
			  upvector.x, upvector.y, upvector.z);
}

/**
 * Sets up the viewport to be the exact size as the camera-picture at \a distance.
 */
void Camera::setupViewport()
{
//	glViewport(0, 0, pictureWidth, pictureHeight);
	glViewport((width - pictureWidth) / 2, (height - pictureHeight) / 2, pictureWidth, pictureHeight);
}

