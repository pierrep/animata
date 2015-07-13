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

#ifndef __FACE_H__
#define __FACE_H__

#include "Vertex.h"
#include "Texture.h"

#ifndef NULL
	#define NULL 0
#endif

namespace Animata
{

/// Building element of Mesh. Consists of three Vertex.
class Face
{
	public:
		Vertex	*v[3];	///< the vertices that build up the Face

		/**
		 * Default constructor.
		 * Does nothing.
		 */
		Face() {}

		/**
		 * Constructor that attaches the given vertices to the Face.
		 * \param v1 The first Vertex.
		 * \param v2 The second Vertex.
		 * \param v3 The third Vertex.
		 */
		Face(Vertex	*v1 = NULL, Vertex *v2 = NULL, Vertex *v3 = NULL)
			{ v[0] = v1; v[1] = v2; v[2] = v3; }

		/**
		 * Moves the Face by the given distance.
		 * This is done by moving the three Vertex of the Face.
		 * \param	dx	Distance in the \e x coordinate.
		 * \param	dy	Distance in the \e y coordinate.
		 */
		void move(float dx, float dy);

		/**
		 * Calculates the center of the Face.
		 * \retval	Vector2D	The center of the Face.
		 */
		Vector2D center(void);

		/**
		 * Attaches texturecoordinates to Vertex \a v[3].
		 * The vertices get the underlying texture's texel coordinates.
		 * \param	t	The texture which will be used for the computations.
		 */
		void attachTexture(Texture *t);

		/**
		 * Sets the one of the Vertex to the given one.
		 * <b>No range checking is done!</b>
		 * \param	i		Number of the vertex to set (possible values are: 0-2).
		 * \param	vert	The new vertex.
		 */
		inline void setVertex(unsigned char i, Vertex *vert) { v[i] = vert; }
};

} /* namespace Animata */

#endif

