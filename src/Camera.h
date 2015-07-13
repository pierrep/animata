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

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "Vector3D.h"

using namespace std;

namespace Animata
{

/// Camera class, manages projection and modelview matrices.
class Camera
{
	private:

		Vector3D	target;			///< position of the target
		Vector3D	orientation;	///< orientation of the camera
		Vector3D	upvector;		///< vector defining the up vector of the camera

		float		distance;		///< distance from the target
		float		fov;			///< vertical field of view in degrees

		int			width;			///< width of the camerapicture at distance
		int			height;			///< height of the camerapicture at distance
		double		aspect;			///< aspect of the camerapicture

		Camera		*parent;		///< the camera whose picture gets cloned by this
		int			pictureWidth;	///< width of the parent's picture
		int			pictureHeight;	///< height of the parent's picture

		float		zNear;			///< distance of the near plane from the camera
		float		zFar;			///< distance of the far plane from the camera

		bool		init;			///< shows if the camera target has already been set

	public:

		Camera();

		void setSize(int w, int h);
		void setAspect(int w, int h);

		void setupModelView();
		void setupPerspective();
		void setupOrtho();
		void setupPickingProjection(int x, int y, int radius);
		void setupViewport();

		/**
		 * Sets the parent camera to a given one.
		 * This will show the same picture as the parent.
		 * \param	c	The new parent camera.
		 */
		inline void setParent(Camera *c) { parent = c; }
		/**
		 * Returns the position of the camera-target.
		 * \retval	Vector3D	3D position of the camera-target.
		 */
		inline Vector3D *getTarget() { return &target; }

		inline void setTarget(Vector3D *t) { target.x = t->x; target.y = t->y; target.z = t->z; }

		/**
		 * Returns the width at the distance from the target of the camera picture.
		 * \retval	int	Width of the picture.
		 */
		inline int getWidth() { return width; }
		/**
		 * Returns the height at the distance from the target of the camera picture.
		 * \retval	int	Height of the picture.
		 */
		inline int getHeight() { return height; }

		/**
		 * Returns the distance from the camera-target.
		 * \retval	float	Distance from the target.
		 */
		inline float getDistance() { return distance; }
		/**
		 * Sets the distance to the camera-target.
		 * \param	d	The new target distance.
		 */
		inline void setDistance(float d) { distance = d; }

		/**
		 * Return the vertical field of view of the camera.
		 * \retval	float	The vertical fov of the camera.
		 */
		inline float getFOV() { return fov; }
		/**
		 * Sets the vertival field of view of the camera.
		 * \param	f	The new vertiacal fov.
		 */
		inline void setFOV(float f) { fov = f; }
};

} /* namespace Animata */

#endif

