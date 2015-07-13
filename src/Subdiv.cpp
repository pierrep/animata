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
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include "Subdiv.h"

using namespace Animata;

// computes the normalized line equation through the points p0 and p1.
aLine::aLine(Vector2D *p0, Vector2D *p1)
{
	Vector2D t;
	t.x = p1->x - p0->x;
	t.y = p1->y - p0->y;
	float len = t.size();
	a =  t.y / len;
	b = -t.x / len;
	c = -(a*p0->x + b*p0->y);
}

// plugs point p into the line equation.
float aLine::eval(Vector2D *p) const
{
	return (a*p->x + b*p->y + c);
}

// ---------- topological operations for delaunay diagrams

int *Subdivision::edge_length = NULL;

/**
 * Initializes a subdivision.
 * \param p_count number of points
 * \param points pointer to a Vector2D array holding the points
 **/
Subdivision::Subdivision(int p_count, Vector2D *points)
	: EPS(1e-6)
{
	this->points = points;
	this->p_count = p_count;

	edge_length = (int *)malloc(4*p_count*sizeof(int));	// edges <= 3*n - 6
	if (!edge_length)
	{
		// fprintf(stderr, "subdiv: not enough memory");
		return;
	}

	float minx, miny, maxx, maxy;
	minx = FLT_MAX;
	miny = FLT_MAX;
	maxx = FLT_MIN;
	maxy = FLT_MIN;
	for (int i = 0; i < p_count; i++)
	{
		if (points[i].x < minx)
			minx = points[i].x;
		else if (points[i].x > maxx)
			maxx = points[i].x;
		if (points[i].y < miny)
			miny = points[i].y;
		else if (points[i].y > maxy)
			maxy = points[i].y;
	}

	// bounding triangle
	float w = maxx - minx;
	float h = maxy - miny;

	points[p_count].x = w/2;
	points[p_count].y = h/2 - h*10;
	points[p_count+1].x = w/2 + w*10;
	points[p_count+1].y = h/2 + h*10;
	points[p_count+2].x = w/2 - w*10;
	points[p_count+2].y = h/2 + h*10;

	Edge *ea = Edge::make_edge();
	ea->endpoints(p_count, p_count+1);
	Edge *eb = Edge::make_edge();
	Edge::splice(ea->sym(), eb);
	eb->endpoints(p_count+1, p_count+2);
	Edge *ec = Edge::make_edge();
	Edge::splice(eb->sym(), ec);
	ec->endpoints(p_count+2, p_count);
	Edge::splice(ec->sym(), ea);
	start_edge = ea;
}

/*
 * returns twice the area of the oriented triangle (a, b, c), i.e., the area is
 * positive if the triangle is oriented counterclockwise.
 */
float Subdivision::triarea(const Vector2D *a, const Vector2D *b, const Vector2D *c)
{
	return (b->x - a->x)*(c->y - a->y) - (b->y - a->y)*(c->x - a->x);
}

/*
 * circumcircle of three points given by (x1, y1), (x2, y2), (x3, y3)
 * returns 1 if d is inside circle
 */
int Subdivision::in_circle(const Vector2D *a, const Vector2D *b,
		const Vector2D *c, const Vector2D *d)
{
	float A = b->x - a->x;
	float B = b->y - a->y;
	float C = c->x - a->x;
	float D = c->y - a->y;

	float E = A*(a->x + b->x) + B*(a->y + b->y);
	float F = C*(a->x + c->x) + D*(a->y + c->y);

	float G = 2.0*(A*(c->y - b->y)-B*(c->x - b->x));

	if (fabs(G) < EPS)  // collinear points
		return 0;

	float xc = (D*E - B*F) / G;
	float yc = (A*F - C*E) / G;

	float r2  = (a->x - xc)*(a->x - xc) + (a->y - yc)*(a->y - yc);
	float r2d = (d->x - xc)*(d->x - xc) + (d->y - yc)*(d->y - yc);
	return (r2d < r2);
}

/**
 * Deletes subdivision and frees up allocated memory.
 **/
void Subdivision::kill(void)
{
	Edge::kill_edges();
	if (edge_length)
	{
		free(edge_length);
		edge_length = NULL;
	}
}

// returns 1 if the points a, b, c are in a counterclockwise order
inline int Subdivision::ccw(const Vector2D *a, const Vector2D *b, const Vector2D *c)
{
	return (triarea(a, b, c) > 0);
}

inline int Subdivision::right_of(const Vector2D *x, Edge *e)
{
	return ccw(x, &points[e->dest()], &points[e->org()]);
}

inline int Subdivision::left_of(const Vector2D *x, Edge *e)
{
	return ccw(x, &points[e->org()], &points[e->dest()]);
}

/*
   a predicate that determines if the point x is on the edge e.
   the point is considered on if it is in the eps-neighborhood
   of the edge.
   */
int Subdivision::on_edge(Vector2D *p, Edge *e)
{
	float t1, t2, t3;
	Vector2D *orig = &points[e->org()];
	Vector2D *dest = &points[e->dest()];
	Vector2D to = Vector2D(p->x - orig->x, p->y - orig->y);
	Vector2D td = Vector2D(p->x - dest->x, p->y - dest->y);
	t1 = to.size();
	t2 = td.size();
	if ((t1 < EPS) || (t2 < EPS))
		return 1;

	Vector2D tod = Vector2D(orig->x - dest->x, orig->y - dest->y);
	t3 = tod.size();
	if ((t1 > t3) || (t2 > t3))
		return 0;

	aLine line(orig, dest);
	return (fabs(line.eval(p)) < EPS);
}

/*
 * an incremental algorithm for the construction of delaunay diagrams
 */

/*
   returns an edge e, s.t. either x is on e, or e is an edge of
   a triangle containing x. the search starts from start_edge
   and proceeds in the general direction of x. based on the
   pseudocode in guibas and stolfi (1985) p.121.
 */
Edge *Subdivision::locate(Vector2D *p)
{
	Edge *e = start_edge;
	int l = 0;
	//for(;;)
	for(; l < 400;l++)	// FIXME: some bugs make an infinite loop from this
	{
		if ( (*p == *(points + e->org())) || (*p == *(points + e->dest())) )
			return NULL;	// point is already in
		else if (right_of(p, e))
			e = e->sym();
		else if (!right_of(p, e->onext()))
			e = e->onext();
		else if (!right_of(p, e->dprev()))
			e = e->dprev();
		else
			return e;
	}
	return NULL;
}

/**
 * Inserts a new point into a subdivision representing a Delaunay
 * triangulation, and fixes the affected edges so that the result
 * is still a Delaunay triangulation. This is based on the
 * pseudocode from guibas and stolfi (1985) p.120, with slight
 * modifications and a bug fix.
 * \param i point index
 **/
void Subdivision::insert_site(int i)
{
	Vector2D *p = &points[i];
	Edge *e = locate(p);
	if (e == NULL)  // point is already in
		return;
	else if (on_edge(p, e))
	{
		e = e->oprev();
		Edge::delete_edge(e->onext());
		}

	// connect the new point to the vertices of the containing
	// triangle (or quadrilateral, if the new point fell on an
	// existing edge)
	Edge *base = Edge::make_edge();
	base->endpoints(e->org(), i);
	Edge::splice(base, e);
	start_edge = base;
	do
	{
		base = Edge::connect_edge(e, base->sym());
		e = base->oprev();
	} while (e->lnext() != start_edge);
	// examine suspect edges to ensure that the Delaunay condition
	// is satisfied.
	for (;;)
	{
		Edge *t = e->oprev();
		if (right_of(&points[t->dest()], e) &&
			in_circle(&points[e->org()], &points[t->dest()], &points[e->dest()], p))
		{
			Edge::swap(e);
			e = e->oprev();
		}
		else if (e->onext() == start_edge)  // no more suspect edges
			return;
		else  // pop a suspect edge
			e = e->onext()->lprev();
	}
}

/**
 * Sends back all faces to caller.
 * \param face_proc this method will be called with three vertex indices
 * \param m pointer to mesh, whose method is face_proc
 **/
void Subdivision::get_faces(FACE_PROC face_proc, Mesh *m)
{
	if (QuadEdge::qe_array == NULL) // no inserted sites
	{
		return;
	}
	std::vector<QuadEdge *>::iterator qi = QuadEdge::qe_array->begin();
	// TODO: every face is sent a couple of times
	for (; qi < QuadEdge::qe_array->end(); qi++)
	{
		Edge *e = (*qi)->e;
		int p0 = e->org();
		int p1 = e->dest();
		int p2 = e->lnext()->dest();
		if ((p0 < p_count) && (p1 < p_count) && (p2 < p_count))
		{
			(m->*face_proc)(p0, p1, p2);
		}

		p2 = e->oprev()->dest();
		if ((p0 < p_count) && (p1 < p_count) && (p2 < p_count))
		{
			(m->*face_proc)(p0, p1, p2);
		}
	}
}

