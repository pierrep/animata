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

#ifndef __SUBDIVISION_H__
#define __SUBDIVISION_H__

#include "Vector2D.h"
#include "QuadEdge.h"
#include "Mesh.h"

namespace Animata
{

class Mesh;
typedef void (Mesh::*FACE_PROC)(int p0, int p1, int p2);

class aLine
{
	public:
		aLine(Vector2D *p0, Vector2D *p1);
		float eval(Vector2D *p) const;
	private:
		float a, b, c;
};

/// Triangulates a set of points.
class Subdivision
{
	private:
		const float EPS;

		Edge *start_edge;
		Edge *locate(Vector2D *p);
		Vector2D *points;	// points to triangulate, space for p_count+3 points
		int p_count;		// number of points

		int right_of(const Vector2D *x, Edge *e);
		int left_of(const Vector2D *x, Edge *e);
		int on_edge(Vector2D *p, Edge *e);
		int ccw(const Vector2D *a, const Vector2D *b, const Vector2D *c);
		float triarea(const Vector2D *a, const Vector2D *b, const Vector2D *c);
		int in_circle(const Vector2D *a, const Vector2D *b,
				const Vector2D *c, const Vector2D *d);
		static int *edge_length;	// length of edges stored for subdivision
	public:
		Subdivision(int p_count, Vector2D *points);
		void insert_site(int i);
		void kill(void);
		void get_faces(FACE_PROC face_proc, Mesh *m);
};

} /* namespace Animata */

#endif

