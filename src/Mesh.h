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

#ifndef __MESH_H__
#define __MESH_H__

#include <vector>

#include "Vertex.h"
#include "Face.h"
#include "Joint.h"
#include "Texture.h"
#include "ADrawable.h"

using namespace std;

namespace Animata
{

/// Represents an image which can be manipulated by a Skeleton.
class Mesh : public ADrawable
{
	private:

		vector<Vertex*>	*vertices;					///< vertices building up the mesh
		vector<Face*>		*faces;						///< faces formed from vertices, representing the texture

		Texture					*attachedTexture;			///< texture attached to the mesh

		Vertex					*pVertex;					///< vertex below the mouse cursor
		Face					*pFace;						///< face below the mouse cursor

		float					textureAlpha;				///< texture alpha for drawing
		int						*selectedPointIndices;		///< helper array for triangulateSelected()

		int getSelectedVerticesCount(void);
		void triangulateSelected(void);
		void triangulateAll(void);

		void sortFaces(void);
		void sortFaces(vector<Face *>::iterator begin,
						vector<Face *>::iterator end);
	public:

		Mesh();
		virtual ~Mesh();

		Vertex *addVertex(float x, float y);

		void deleteSelectedVertex(void);
		void deleteSelectedFace(Face *f);

		int moveSelectedVertices(float dx, float dy);
		void clearSelection(void);
		vector<Vertex *> *getSelectedVertices();

		void setVertexViewCoords(float *coords, unsigned int size);

		/**
		 * Returns the vertex below the mouse cursor.
		 * \retval Vertex* The vertex below the mouse cursor.
		 */
		inline Vertex *getPointedVertex(void) { return pVertex; }
		/**
		 * Returns the face below the mouse cursor.
		 * \retval Face* The face below the mouse cursor.
		 */
		inline Face *getPointedFace(void) { return pFace; }

		/**
		 * Returns the vertices of the mesh.
		 * \retval std::vector<Vertex *> Vertices of the mesh.
		 */
		inline vector<Vertex *> *getVertices(void) { return vertices; }
		/**
		 * Returns the faces of the mesh.
		 * \retval std::vector<Face *> Faces of the mesh.
		 */
		inline vector<Face *> *getFaces(void) { return faces; }

		vector<Vertex *>::iterator getSelectedVertex(Vertex **ppv = NULL);

		void addFace(Vertex *v0, Vertex *v1, Vertex *v2);
		void clearFaces(void);

		void triangulate(void);
		void triangulateFaceProc(int p0, int p1, int p2);
		void triangulateFaceProcSelected(int p0, int p1, int p2);

		/// attach texture and calculate texture coordinates for vertices
		void attachTexture(Texture *t);

		/**
		 * Attaches a texture to the mesh.
		 * Doesn't calculat new texturecoordinates.
		 * \param	t	The new texture to attach.
		 * \sa attachTexture()
		 */
		inline void setAttachedTexture(Texture *t) { attachedTexture = t; }
		/**
		 * Returns the texture attached to the mesh.
		 * \retval	Texture*	The attached texture.
		*/
		inline Texture *getAttachedTexture(void) { return attachedTexture; }

		/**
		 * Sets texture alpha for drawing.
		 * \param	alpha	The new texture alpha.
		 */
		inline void setTextureAlpha(float alpha) { textureAlpha = alpha; }

		virtual void draw(int mode, int active = 1);
		virtual void select(unsigned i, int type);
		virtual void circleSelect(unsigned i, int type, int xc, int yc, float r);
};

} /* namespace Animata */

#endif

