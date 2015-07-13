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

#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "Vector2D.h"

namespace Animata
{

/// A point that builds up a Face.
class Vertex
{
	public:
		Vector2D	coord;			///< 2D position of the Vertex in the world
		Vector2D	texCoord;		///< texture coordinate used at this point
		Vector2D	view;			///< the position of the Vertex on the screen

		bool		selected;		///< selection state

		/**
		 * Creates a new Vertex at a given position.
		 * \param c The position where to place the new Vertex.
		 * \param tc Texture coordinate assigned to the Vertex.
		 */
		Vertex(Vector2D c, Vector2D tc = Vector2D()) { coord = c; texCoord = tc; selected = false; }

		/**
		 * Draws the Vertex onscreen.
		 * \param mouseOver Indicates if the mouseOver state should be drawn.
		 * \param active Indicates if the active state should be drawn.
		 */
		void draw(int mouseOver = 0, int active = 1);

		/// Inverts the selection state of the Vertex.
		void flipSelection(void);
};

} /* namespace Animata */

#endif

