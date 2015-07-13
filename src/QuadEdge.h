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

#ifndef __QUADEDGE_H__
#define __QUADEDGE_H__

#include <vector>

using namespace std;

namespace Animata
{

class Edge;
class QuadEdge;

class Edge
{
	friend class QuadEdge;
	friend void splice(Edge *a, Edge *b);
	friend void delete_edge(Edge *e);
	private:
		int num;
		Edge *next;
		int data;	// point index
	public:
		Edge() { data = 0; };
		Edge *rot(void);
		Edge *invrot(void);
		Edge *sym(void);
		Edge *onext(void);
		Edge *oprev(void);
		Edge *dnext(void);
		Edge *dprev(void);
		Edge *lnext(void);
		Edge *lprev(void);
		Edge *rnext(void);
		Edge *rprev(void);
		int org(void);
		int dest(void);
		void endpoints(int orig, int dest);

		static Edge *make_edge(void);
		static void kill_edges(void);
		static void splice(Edge *a, Edge *b);
		static void delete_edge(Edge *e);
		static Edge* connect_edge(Edge *a, Edge *b);
		static void swap(Edge *e);
};

class QuadEdge
{
	friend class Edge;
	friend class Subdivision;

	private:
		Edge e[4];
		static vector<QuadEdge *> *qe_array;
	public:
		QuadEdge();
};

} /* namespace Animata */

#endif

