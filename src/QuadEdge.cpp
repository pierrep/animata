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

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "QuadEdge.h"

using namespace Animata;

QuadEdge::QuadEdge()
{
	e[0].num = 0;
	e[1].num = 1;
	e[2].num = 2;
	e[3].num = 3;

	e[0].next = &(e[0]);
	e[1].next = &(e[3]);
	e[2].next = &(e[2]);
	e[3].next = &(e[1]);
}

// return the dual of the current edge, directed from its right to its left.
Edge *Edge::rot(void)
{
	return (num < 3) ? this + 1 : this - 3;
}

// return the dual of the current edge, directed from its left to its right.
Edge *Edge::invrot(void)
{
	return (num > 0) ? this - 1 : this + 3;
}

// return the edge from the destination to the origin of the current edge.
Edge *Edge::sym(void)
{
	return (num < 2) ? this + 2 : this - 2;
}

// return the next ccw edge around (from) the origin of the current edge.
Edge *Edge::onext(void)
{
	return next;
}

// return the next cw edge around (from) the origin of the current edge.
Edge *Edge::oprev(void)
{
	return rot()->onext()->rot();
}

// return the next ccw edge around (into) the destination of the current edge.
Edge *Edge::dnext(void)
{
	return sym()->onext()->sym();
}

// return the next cw edge around (into) the destination of the current edge.
Edge *Edge::dprev(void)
{
	return invrot()->onext()->invrot();
}

// return the ccw edge around the left face following the current edge.
Edge *Edge::lnext(void)
{
	return invrot()->onext()->rot();
}

// return the ccw edge around the left face before the current edge.
Edge *Edge::lprev(void)
{
	return onext()->sym();
}

// return the edge around the right face ccw following the current edge.
Edge *Edge::rnext(void)
{
	return rot()->onext()->invrot();
}

// return the edge around the right face ccw before the current edge.
Edge *Edge::rprev(void)
{
	return sym()->onext();
}

// ----- access to data pointers

int Edge::org(void)
{
	return data;
}

int Edge::dest(void)
{
	return sym()->data;
}

void Edge::endpoints(int orig, int dest)
{
	data = orig;
	sym()->data = dest;
}

// ----- basic topological operators

vector<QuadEdge *> *QuadEdge::qe_array = NULL;

Edge *Edge::make_edge(void)
{
	if (QuadEdge::qe_array == NULL)
		QuadEdge::qe_array = new vector<QuadEdge *>;

	QuadEdge *ql = new QuadEdge;
	QuadEdge::qe_array->push_back(ql);

	return ql->e;
}

void Edge::kill_edges(void)
{
	if (QuadEdge::qe_array)
	{
		vector<QuadEdge *>::iterator q = QuadEdge::qe_array->begin();
		for (; q < QuadEdge::qe_array->end(); q++)
			delete *q;
		QuadEdge::qe_array->clear();
		delete QuadEdge::qe_array;
		QuadEdge::qe_array = NULL;
	}
}

/*
 * this operator affects the two edge rings around the origins of a and b, and,
 * independently, the two edge rings around the left faces of a and b.  in each
 * case, (i) if the two rings are distinct, splice will combine them into one;
 * (ii) if the two are the same ring, splice will break it into two separate
 * pieces.  thus, splice can be used both to attach the two edges together, and
 * to break them apart. see guibas and stolfi (1985) p.96 for more details and
 * illustrations.
 */
void Edge::splice(Edge *a, Edge *b)
{
	Edge *alpha = a->onext()->rot();
	Edge *beta  = b->onext()->rot();

	Edge *t1 = b->onext();
	Edge *t2 = a->onext();
	Edge *t3 = beta->onext();
	Edge *t4 = alpha->onext();

	a->next = t1;
	b->next = t2;
	alpha->next = t3;
	beta->next = t4;
}

void Edge::delete_edge(Edge *e)
{
	splice(e, e->oprev());
	splice(e->sym(), e->sym()->oprev());

	QuadEdge *q = (QuadEdge *)(e - e->num);		// quad-edge pointer of edge

	vector<QuadEdge *>::iterator qi = QuadEdge::qe_array->begin();
	for (; qi < QuadEdge::qe_array->end(); qi++)
	{
		if (*qi == q)
		{
			QuadEdge::qe_array->erase(qi);
			break;
		}
	}

	delete q;
}

/*
   add a new edge e connecting the destination of a to the
   origin of b, in such a way that all three have the same
   left face after the connection is complete.
   additionally, the data pointers of the new edge are set.
   */
Edge* Edge::connect_edge(Edge *a, Edge *b)
{
	Edge *e = make_edge();
	splice(e, a->lnext());
	splice(e->sym(), b);
	e->endpoints(a->dest(), b->org());
	return e;
}

/*
   essentially turns edge e counterclockwise inside its enclosing
   quadrilateral. the data pointers are modified accordingly.
   */
void Edge::swap(Edge *e)
{
	Edge *a = e->oprev();
	Edge *b = e->sym()->oprev();
	splice(e, a);
	splice(e->sym(), b);
	splice(e, a->lnext());
	splice(e->sym(), b->lnext());
	e->endpoints(a->dest(), b->dest());
}

