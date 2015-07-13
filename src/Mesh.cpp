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
#include <iostream>
#include <algorithm>

#include "animata.h"
#include "animataUI.h"
#include "Primitives.h"
#include "Mesh.h"
#include "Subdiv.h"
#include "Transform.h"

#if defined(__APPLE__)
	#include <OPENGL/gl.h>
#else
	#include <GL/gl.h>
#endif

using namespace Animata;

/**
 * Creates a new default mesh without an attached texture.
 */
Mesh::Mesh()
{
	vertices = new vector<Vertex*>;
	faces = new vector<Face*>;

	attachedTexture = NULL;
	pVertex = NULL;
	pFace = NULL;

	textureAlpha = 1.0f;
}

/**
 * Destroys the mesh and frees up all the memory allocated for it's vertices and faces.
 */
Mesh::~Mesh()
{
	if (vertices)
	{
		vector<Vertex *>::iterator v = vertices->begin();
		for (; v < vertices->end(); v++)
			delete *v;					// free vertices from memory
		vertices->clear();				// clear all vector elements
		delete vertices;
	}

	if (faces)
	{
		clearFaces();
		delete faces;
	}
}

/**
 * Creates new vertex at given position, and adds it to the mesh.
 * \param	x	\e x coordinate of the position
 * \param	y	\e y coordinate of the position
 * \retval Vertex* The newly created vertex.
 */
Vertex *Mesh::addVertex(float x, float y)
{
	Vector2D vector(x, y);
	Vertex *v = new Vertex(vector);
	vertices->push_back(v);
	return v;
}

/**
 * Creates a new face based on the given vertices, and adds it to the mesh.
 * If there is already a face in the same position, or the vertices aren't different, nothing happens.
 * If there is a texture attached to the mesh, texture coordinates will be added to the vertices also.
 * \param v0 First vertex of the face.
 * \param v1 Second vertex of the face.
 * \param v2 Third vertex of the face.
 */
void Mesh::addFace(Vertex *v0, Vertex *v1, Vertex *v2)
{
	// check if there are same vertices
	if (v0 == v1 || v1 == v2 || v2 == v0)
		return;
	// check if a previous face exists between these vertices
	for (unsigned i = 0; i < faces->size(); i++)
	{
		Face *face = (*faces)[i];
		if (((face->v[0] == v0) && (face->v[1] == v1) && (face->v[2] == v2)) ||
			((face->v[0] == v0) && (face->v[1] == v2) && (face->v[2] == v1)) ||
			((face->v[0] == v1) && (face->v[1] == v2) && (face->v[2] == v0)) ||
			((face->v[0] == v1) && (face->v[1] == v0) && (face->v[2] == v2)) ||
			((face->v[0] == v2) && (face->v[1] == v0) && (face->v[2] == v1)) ||
			((face->v[0] == v2) && (face->v[1] == v1) && (face->v[2] == v0)))
		{
			return;
		}
	}

	faces->push_back(new Face(v0, v1, v2));

	/* if there's a texture attached add texture coordinates also */
	if (attachedTexture)
	{
		Face *face = faces->back();

		face->attachTexture(attachedTexture);
	}
}

/**
 * Deletes every face belonging to the mesh.
 */
void Mesh::clearFaces(void)
{
	vector<Face *>::iterator f = faces->begin();
	for (; f < faces->end(); f++)
		delete *f;				// free faces from memory
	faces->clear();				// clear all vector elements
}

/**
 * Returns the number of selected vertices.
 * \retval int The number of selected vertices.
 */
int Mesh::getSelectedVerticesCount(void)
{
	int s = 0;
	for (unsigned i = 0; i < vertices->size(); i++)
	{
		if ((*vertices)[i]->selected)
			s++;
	}
	return s;
}

/**
 * Turns selected to true on the i.th vertex.
 * \param	i		The number of the vertex to select.
 * \param	type	Should be Selection::SELECT_VERTEX.
 */
void Mesh::select(unsigned i, int type)
{
	switch(type)
	{
		case Selection::SELECT_VERTEX:
			if(i < vertices->size())
			{
				(*vertices)[i]->selected = true;
			}
			break;
	}
}

/**
 * Turns selected to true on the i.th vertex if its in the given circle.
 * \param	i		The number of the vertex to select.
 * \param	type	Should be Selection::SELECT_VERTEX.
 * \param	xc		\e x coordinate of the circle's center.
 * \param	yc		\e y coordinate of the circle's center.
 * \param	r		Radius of the circle.
 */
void Mesh::circleSelect(unsigned i, int type, int xc, int yc, float r)
{
	switch (type)
	{
		case Selection::SELECT_VERTEX:
			if (i < vertices->size())
			{
				Vertex *v = (*vertices)[i];
				float dx = v->view.x - xc;
				float dy = v->view.y - yc;
				if (dx*dx + dy*dy <= r*r)
					v->selected = true;
			}
			break;
	}
}

void Mesh::triangulateSelected(void)
{
	int selectedCount = getSelectedVerticesCount();

	/* create an array of the selected points */
	Vector2D *points = new Vector2D[selectedCount + 3];
	selectedPointIndices = new int[selectedCount + 3];

	int pCount = vertices->size();
	int j = 0;
	for (int i = 0; i < pCount; i++)
	{
		Vertex *v = (*vertices)[i];

		if (v->selected)
		{
			points[j].x = v->coord.x;
			points[j].y = v->coord.y;
			selectedPointIndices[j] = i; /* store the original point index */
			j++;
		}
	}

	/* delete faces of selected vertices */
	for (int s = 0; s < selectedCount; s++)
	{
		Vertex *v = (*vertices)[selectedPointIndices[s]];
		for (int i = faces->size() - 1; i >= 0; i--)
		{
			Face *face = (*faces)[i];
			if ((face->v[0] == v) || (face->v[1] == v) || (face->v[2] == v))
			{
				vector<Face *>::iterator faceIter = faces->begin() + i;
				delete face;
				faces->erase(faceIter);
			}
		}
	}

	/* store number of old faces to sort only new ones */
	int oldFaceCount = faces->size();

	/* generate new faces */
	Subdivision *s = new Subdivision(selectedCount, points);

	for (int i = 0; i < selectedCount; i++)
	{
		s->insert_site(i);
	}
	s->get_faces(&Mesh::triangulateFaceProcSelected, this);
	s->kill();

	delete s;
	delete [] points;
	delete [] selectedPointIndices;

	/* sort the new faces only */
	sortFaces(faces->begin() + oldFaceCount, faces->end());
}

void Mesh::triangulateAll(void)
{
	clearFaces();

	int pCount = vertices->size();
	// make an array for pCount + 3 elements
	Vector2D *points = new Vector2D[pCount + 3];
	for (int i = 0; i < pCount; i++)
	{
		Vertex *v = (*vertices)[i];

		points[i].x = v->coord.x;
		points[i].y = v->coord.y;
	}

	Subdivision *s = new Subdivision(pCount, points);

	for (int i = 0; i < pCount; i++)
	{
		s->insert_site(i);
	}
	s->get_faces(&Mesh::triangulateFaceProc, this);
	s->kill();

	delete s;
	delete [] points;

	sortFaces(); // sort all faces
}

/**
 * Triangulates vertices. Calculates triangulation for the selected vertices
 * or for all the vertices if no, or less then 3 vertices are selected.
 **/
void Mesh::triangulate(void)
{
	int p = getSelectedVerticesCount();
	if (p == 0)
		triangulateAll();
	else if (p >= 3)
		triangulateSelected();
}

/**
 * Face callback procedure for triangulation called from the Subdivision
 * object.
 * \param p0 index of first triangle vertex
 * \param p1 index of second triangle vertex
 * \param p2 index of third triangle vertex
 **/
void Mesh::triangulateFaceProc(int p0, int p1, int p2)
{
	Vertex *v0 = (*vertices)[p0];
	Vertex *v1 = (*vertices)[p1];
	Vertex *v2 = (*vertices)[p2];

	if (attachedTexture)
	{
		// if triangle alpha is below the threshold reject the face
		Texture *t = attachedTexture;
		float scale = t->getScale();

		float t0x = t->width * ((v0->coord.x - t->x) / ((float)t->width * scale));
		float t0y = t->height * ((v0->coord.y - t->y) / ((float)t->height * scale));
		float t1x = t->width * ((v1->coord.x - t->x) / ((float)t->width * scale));
		float t1y = t->height * ((v1->coord.y - t->y) / ((float)t->height * scale));
		float t2x = t->width * ((v2->coord.x - t->x) / ((float)t->width * scale));
		float t2y = t->height * ((v2->coord.y - t->y) / ((float)t->height * scale));

		/*
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		float t0x = t->width * ((v0->coord.x - t->x) / ((float)t->width * scale));
		float t0y = t->height * ((viewport[3] -v0->coord.y - t->y) / ((float)t->height * scale));
		float t1x = t->width * ((v1->coord.x - t->x) / ((float)t->width * scale));
		float t1y = t->height * ((viewport[3] - v1->coord.y - t->y) / ((float)t->height * scale));
		float t2x = t->width * ((v2->coord.x - t->x) / ((float)t->width * scale));
		float t2y = t->height * ((viewport[3] - v2->coord.y - t->y) / ((float)t->height * scale));
		*/

		int alpha = attachedTexture->getTriangleAlpha(t0x, t0y,
				t1x, t1y, t2x, t2y, 4);
		if (alpha < ui->settings.triangulateAlphaThreshold)
			return;
	}

	addFace(v0, v1, v2);
}

/**
 * Face callback procedure for the triangulation of selected vertices called
 * from the Subdivision object.
 * \param p0 index of first triangle vertex from selected vertices
 * \param p1 index of second triangle vertex from selected vertices
 * \param p2 index of third triangle vertex from selected vertices
 **/
void Mesh::triangulateFaceProcSelected(int p0, int p1, int p2)
{
	/* translate selected point indices to original point indices */
	int i0 = selectedPointIndices[p0];
	int i1 = selectedPointIndices[p1];
	int i2 = selectedPointIndices[p2];
	Vertex *v0 = (*vertices)[i0];
	Vertex *v1 = (*vertices)[i1];
	Vertex *v2 = (*vertices)[i2];

	if (attachedTexture)
	{
		// if triangle alpha is below the threshold reject the face
		Texture *t = attachedTexture;
		float scale = t->getScale();

		float t0x = t->width * ((v0->coord.x - t->x) / ((float)t->width * scale));
		float t0y = t->height * ((v0->coord.y - t->y) / ((float)t->height * scale));
		float t1x = t->width * ((v1->coord.x - t->x) / ((float)t->width * scale));
		float t1y = t->height * ((v1->coord.y - t->y) / ((float)t->height * scale));
		float t2x = t->width * ((v2->coord.x - t->x) / ((float)t->width * scale));
		float t2y = t->height * ((v2->coord.y - t->y) / ((float)t->height * scale));

		/*
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		float t0x = t->width * ((v0->coord.x - t->x) / ((float)t->width * scale));
		float t0y = t->height * ((viewport[3] -v0->coord.y - t->y) / ((float)t->height * scale));
		float t1x = t->width * ((v1->coord.x - t->x) / ((float)t->width * scale));
		float t1y = t->height * ((viewport[3] - v1->coord.y - t->y) / ((float)t->height * scale));
		float t2x = t->width * ((v2->coord.x - t->x) / ((float)t->width * scale));
		float t2y = t->height * ((viewport[3] - v2->coord.y - t->y) / ((float)t->height * scale));
		*/

		int alpha = attachedTexture->getTriangleAlpha(t0x, t0y,
				t1x, t1y, t2x, t2y, 4);
		if (alpha < ui->settings.triangulateAlphaThreshold)
			return;
	}
	addFace(v0, v1, v2);
}

static bool triangleSortPredicate(Face *a, Face *b)
{
	Vector2D ac = a->center();
	Vector2D bc = b->center();

	if (ac.y < bc.y)
		return true;
	else if ((ac.y == bc.y) && (ac.x < ac.x))
		return true;
	else
		return false;
}

void Mesh::sortFaces(void)
{
	sort(faces->begin(), faces->end(), triangleSortPredicate);
}

void Mesh::sortFaces(vector<Face *>::iterator begin, vector<Face *>::iterator end)
{
	sort(begin, end, triangleSortPredicate);
}

/**
 * Finds the selected vertex.
 * \param ppv Pointer to the vertex pointer.
 * \return Returns a pointer and an iterator to it, or NULL if no vertex is found.
 */
vector<Vertex *>::iterator Mesh::getSelectedVertex(Vertex **ppv /* = NULL */)
{
	unsigned char hit = selector->getHitCount();
	SelectItem *selected = selector->getSelected();

	Vertex *selVertex = NULL;
	for (unsigned int i = 0; i < hit; i++)
	{
		if (selected->type == Selection::SELECT_VERTEX)
		{
			selVertex = (*vertices)[selected->name];
			break;
		}
		selected++;
	}

	if (selVertex && (ppv != NULL))
		*ppv = selVertex;

	return vertices->begin() + selected->name;
}

/**
 * Deletes already selected vertices of the mesh.
 * If there are any faces belonging to this vertices, they gets also deleted.
 */
void Mesh::deleteSelectedVertex(void)
{
	vector<Vertex *>::iterator iter;
	Vertex *selVertex = NULL;

	iter = getSelectedVertex(&selVertex);

	if (selVertex == NULL) /* no vertex below the cursor */
		return;

	// checking vector elements backwards to be able to iterate and erase at
	// the same time
	for (int i = faces->size() - 1; i >= 0; i--)
	{
		Face *face = (*faces)[i];

		if ((face->v[0] == selVertex) ||
			(face->v[1] == selVertex) ||
			(face->v[2] == selVertex))
		{
			vector<Face *>::iterator faceIter = faces->begin() + i;
			delete face;
			faces->erase(faceIter);
		}
	}

	//if (hit && selected->type == Selection::SELECT_VERTEX)
	{
		delete *iter;			// delete object
		vertices->erase(iter);	// remove it from the vector

		// current selection points to the next joint after the deleted one
		selector->clearSelection();
	}
}

/**
 * Removes the given face from the mesh.
 * \param	f	A pointer to the face to remove.
 */
void Mesh::deleteSelectedFace(Face *f)
{
	/* delete the face */
	vector<Face *>::iterator iter = faces->begin();
	for (unsigned i = 0; i < faces->size(); i++)
	{
		Face *face = (*faces)[i];
		if (face == f)
		{
			vector<Face *>::iterator faceIter = faces->begin() + i;
			delete face;
			faces->erase(faceIter);
			break;
		}
	}
	/* clear selection, because it contains a non-existing object */
	selector->clearSelection();
}

/**
 * Attaches a texture to the mesh.
 * Texture coordinates of the face's vertices gets also calculated.
 * \param	t	Texture to attach.
 */
void Mesh::attachTexture(Texture *t)
{
	attachedTexture = t;

	for (unsigned int i = 0; i < faces->size(); i++)
	{
		(*faces)[i]->attachTexture(attachedTexture);
	}
}

/**
 * Moves the already selected vertices by a given distance.
 * Also returns the number of moved vertices.
 * \param	dx	Distance in the \e x coordinate.
 * \param	dy	Distance in the \e y coordinate.
 * \retval	int	Number of the moved vertices.
 */
int Mesh::moveSelectedVertices(float dx, float dy)
{
	int movedVertices = 0;

	for (unsigned i = 0; i < vertices->size(); i++)
	{
		Vertex *v = (*vertices)[i];

		if (v->selected)
		{
			v->coord.x += dx;
			v->coord.y += dy;
			movedVertices++;
		}
	}

	/* return the number of vertices moved */
	return movedVertices;
}

/**
 * Returns every selected vertex of the mesh in a newly allocated std::vector.
 * \retval	std::vector<Vertex *>	Vector of the selected vertices.
 */
vector<Vertex *> *Mesh::getSelectedVertices()
{
	vector<Vertex *> *selectedVertices = new vector<Vertex *>;

	for (unsigned i = 0; i < vertices->size(); i++)
	{
		Vertex *v = (*vertices)[i];

		if (v->selected)
		{
			selectedVertices->push_back(v);
		}
	}

	return selectedVertices;
}

/**
 * Sets the view coordinates of the vertices of this mesh.
 * Setting the transformation matrices by Transform::setMatrices() is neccesary before calling this,
 * because Transform::project() is used for non visible primitives. If any coordinate's value is Selection::OUT_OF_SCREEN,
 * Transform::project() tries to calculate the vertex's screen coordinate.
 * \param	coords	An array which holds the screen coordinates of the vertices. Three elements in a row will represent a vertex's
					screen coordinate. First one is the vertex's number, second and third is the \e x and \e y screen coordinate.
 * \param	size	Size of the \c coords array.
 **/
void Mesh::setVertexViewCoords(float *coords, unsigned int size)
{
	for(unsigned i = 0; i < size; i+=3)
	{
		unsigned n = (unsigned)coords[i];
		Vertex *v = (*vertices)[n];

		// vertex is out of screen lets do projection here
		if(i + 1 < size && coords[i + 1] == Selection::OUT_OF_SCREEN)
		{
			Vector3D view = Transform::project(v->coord.x, v->coord.y, 0);

			v->view.x = view.x;
			v->view.y = view.y;
		}
		else
		{
			v->view.x = coords[i + 1];
			v->view.y = coords[i + 2];
		}
	}
}

/**
 * Turns every vertex's selected flag to false.
 */
void Mesh::clearSelection(void)
{
	for (unsigned i = 0; i < vertices->size(); i++)
	{
		(*vertices)[i]->selected = false;
	}
}

/**
 * Draws the mesh.
 * Vertices, faces and faces with textures attached to the mesh get drawn
 * based on the actual AnimataSettings::display_elements.
 *
 * There are four drawing modes at this time, which can be combined by
 * bitwise or.
 *	- RENDER_FEEDBACK\n
 *		Only draws a point to every vertex of the mesh, to get their exact
 *		position that can be used to set their screen coordinates.
 *	- RENDER_WIREFRAME\n
 *		Draws vertex and face outlines.
 *	- RENDER_TEXTURE\n
 *		Draws textured faces if there is a texture attached to the mesh.
 *	- RENDER_OUTPUT\n
 *		Indicates that the drawing happens in the output window, there is no
 *		need for mouseOver for the primitives.
 *
 * \param	mode	In which mode to draw the mesh.
 * \param	active	Flag represents whether the mesh is on the active Layer.
 */
void Mesh::draw(int mode, int active)
{
	unsigned hit = 0;
	SelectItem *selected = NULL;

	if(selector->getPickLayer() && selector->getPickLayer()->getMesh() == this)
	{
		hit = selector->getHitCount();
		selected = selector->getSelected();
	}

	// select the first vertex from the selection buffer
	pVertex = NULL;
	pFace = NULL;
	for (unsigned int i = 0; i < hit; i++)
	{
		if (selected->type == Selection::SELECT_VERTEX &&
			((ui->settings.mode == ANIMATA_MODE_MESH_SELECT) ||
			 (ui->settings.mode == ANIMATA_MODE_MESH_DELETE) ||
			 (ui->settings.mode == ANIMATA_MODE_CREATE_VERTEX) ||
			 (ui->settings.mode == ANIMATA_MODE_ATTACH_VERTICES) ||
			 (ui->settings.mode == ANIMATA_MODE_CREATE_TRIANGLE)))
		{
			// vertices are preferred to faces if they overlap
			pVertex = (*vertices)[selected->name];
			pFace = NULL;
			break;
		}
		else
		if (selected->type == Selection::SELECT_TRIANGLE &&
			((ui->settings.mode == ANIMATA_MODE_MESH_SELECT) ||
			 (ui->settings.mode == ANIMATA_MODE_MESH_DELETE)))
		{
			pFace = (*faces)[selected->name];
		}

		selected++;
	}

	if (attachedTexture && (mode & RENDER_TEXTURE) &&
		((!(mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_EDITOR_TEXTURE) ||
		((mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_OUTPUT_TEXTURE)))
	{
		glColor3f(1.f, 1.f, 1.f);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, attachedTexture->getGlResource());

		for (unsigned int i = 0; i < faces->size(); i++)
		{
			Face *face = (*faces)[i];

			glColor4f(1.f, 1.f, 1.f, textureAlpha);
			glBegin(GL_TRIANGLES);
				glTexCoord2f(face->v[0]->texCoord.x, face->v[0]->texCoord.y);
				glVertex2f(face->v[0]->view.x, face->v[0]->view.y);
				glTexCoord2f(face->v[1]->texCoord.x, face->v[1]->texCoord.y);
				glVertex2f(face->v[1]->view.x, face->v[1]->view.y);
				glTexCoord2f(face->v[2]->texCoord.x, face->v[2]->texCoord.y);
				glVertex2f(face->v[2]->view.x, face->v[2]->view.y);
			glEnd();
			glColor3f(1.f, 1.f, 1.f);
		}

		glDisable(GL_TEXTURE_2D);
	}

	if ((mode & RENDER_WIREFRAME) &&
		((!(mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_EDITOR_TRIANGLE) ||
		((mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_OUTPUT_TRIANGLE)))
	{
		glLoadName(Selection::SELECT_TRIANGLE);	// type of the primitive
		glPushName(0);							// id of the primitive

		for (unsigned int i = 0; i < faces->size(); i++)
		{
			Face *face = (*faces)[i];

			glLoadName(i);

			if(mode & RENDER_OUTPUT)
				Primitives::drawFace(face);
			else
				Primitives::drawFace(face, face == pFace, active);
		}
		glPopName();
	}

	if(mode & RENDER_FEEDBACK)
	{
		for(unsigned int i = 0; i < vertices->size(); i++)
		{
			Vertex *vertex = (*vertices)[i];

			glPassThrough(i);
			glBegin(GL_POINTS);
				glVertex2f(vertex->coord.x, vertex->coord.y);
			glEnd();
		}
	}
	else if ((mode & RENDER_WIREFRAME) &&
			 ((!(mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_EDITOR_VERTEX) ||
			 ((mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_OUTPUT_VERTEX)))
	{
		glLoadName(Selection::SELECT_VERTEX);	// type of the primitive
		glPushName(0);							// id of the primitive

		for(unsigned int i = 0; i < vertices->size(); i++)
		{
			Vertex *vertex = (*vertices)[i];

			glLoadName(i);

			if(mode & RENDER_OUTPUT)
				vertex->draw(false);
			else
				vertex->draw(vertex == pVertex, active);
		}

		glPopName();
	}

}


