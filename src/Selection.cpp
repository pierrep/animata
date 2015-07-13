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

#include "Selection.h"
#include "Transform.h"

using namespace Animata;

/**
 * Creates a new selection object which is responsible for picking, selection and feedback.
 * Buffers for OpenGL and also for storing picked primitives are allocated with it's default size.
 */
Selection::Selection()
{
	hitCount = 0;

	selectBuffer = new GLuint[BUFSIZE];
	feedBuffer = new float[BUFSIZE];

	selectedLength = MAXHIT_INIT;
	selected = new SelectItem[selectedLength];

	pointsLength = 8190;
	points = new float[pointsLength];

	pickLayer = NULL;
}

/**
 * Deletes the selection object and frees up the memory allocated by it.
 */
Selection::~Selection()
{
	delete [] selectBuffer;
	delete [] feedBuffer;
	delete [] selected;
	delete [] points;
}

		/// starts the picking on given coordinates
/**
 * Drawing the mesh, skeleton and tetxure in GL_SELECT mode, deterimes which of their building blocks are in radius of the given coordinate.
 * This can be used for getting objects under the mouse cursor if called with the mouse coordinates.
 * Searching radius is read from \a radius.
 * The found primitives are then saved in the \a selected array, which elements hold each primitive's type and name.
 * Camera is used to determine the projection matrix for picking based on the camera properties.
 *
 * \param	camera	The camera used for the scene.
 * \param	layer	Mesh and skeleton gets searched from this layer.
 * \param	textureManager	TextureManager which holds the texture attached to the mesh on the given layer.
 * \param	x		\e x coordinate of the searching center.
 * \param	y		\e y coordinate of the searching center.
 */
void Selection::doPick(Camera *camera, Layer *layer, TextureManager *textureManager, int x, int y)
{
	GLint hits;

	// invalidate hits; if hit is valid during changing the actual layer in
	// the laterTree, there could be an invalid selection on the new layer
	// coming from the old one
	// TODO: use setPickLayer when changing layer
	if(layer != pickLayer)
		hitCount = 0;

	pickLayer = layer;

	// start selection mode
	glSelectBuffer(BUFSIZE, selectBuffer);
	(void)glRenderMode(GL_SELECT);

	// clear the name stack, push initial name on it
	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	camera->setupPickingProjection(x, y, radius);

	// drawing happens in ortho mode without any transformation in the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	if (layer->getMesh()->getAttachedTexture())
		layer->getMesh()->getAttachedTexture()->draw();

	// draw only the current layer, no sublayers
	layer->getMesh()->draw(RENDER_TEXTURE | RENDER_WIREFRAME);
	layer->getSkeleton()->draw(RENDER_TEXTURE | RENDER_WIREFRAME);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();

	// go back to render mode, and process selection buffer
	hits = glRenderMode(GL_RENDER);
	processHits(hits, selectBuffer);
}

void Selection::processHits(unsigned hits, GLuint buffer[])
{
	// every hit record has 4 item
	// - the number of names on the stack when the hit occured
	// - the min/max window coordinate z values of all the vertices	of the primitive that intersected the viewing volume
	// - contents of the name stack at the time of the hit, starting with the bottommost element first

	GLuint found = 0;
	GLuint *ptr = (GLuint*)buffer;

	hitCount = 0;

	// if selected array is to small to store hits, allocate a new one with hits lenght
	if(hits > selectedLength)
	{
		delete [] selected;

		selectedLength = hits;
		selected = new SelectItem[selectedLength];
	}

	for(unsigned i = 0; i < hits; i++)
	{
		GLuint names = *ptr;

		//printf("number of names for this hit = %d\n", names); ptr++;
		//printf("  z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
		//printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;

		ptr += 3;
		for(GLuint j = 0; j < names; j++)
		{
			found = *ptr;

			if(j == 0)
			{
				selected[hitCount].type = found;
			}
			else if(j == 1)
			{
				selected[hitCount++].name = found;
			}

			ptr++;
		}
	}
}

/**
 * Selects the building elements of \c type from \c node in the given rectangle by calling select() for each element.
 * \c node has to be an inherited from Drawable, so it gets a valid select()
 * \param	node	The an object inherited from Drawable which gets searched for its primitives.
 * \param	type	Type of primitive which are searched.
 * \param	x		\e x coordinate of the searching reactangle's upper left corner.
 * \param	y		\e y coordinate of the searching reactangle's upper left corner.
 * \param	w		Width of the searching reactangle.
 * \param	h		Height of the searching reactangle.
 * \sa doCircleSelect()
 */
void Selection::doSelect(ADrawable *node, unsigned type, float x, float y, float w, float h)
{
	GLint hits;

	// get the boundaries of actual viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// start selection mode
	glSelectBuffer(BUFSIZE, selectBuffer);
	(void)glRenderMode(GL_SELECT);

	// clear the name stack, push initial name on it
	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// mirror the screen along y coordinate because of the different coordinate systems
	glOrtho(x, w, viewport[3] - h, viewport[3] - y, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	node->draw(RENDER_TEXTURE | RENDER_WIREFRAME);

	// restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();

	// go back to render mode, and process selection buffer
	hits = glRenderMode(GL_RENDER);

	GLuint *ptr = (GLuint*)selectBuffer;

	for(GLint i = 0; i < hits; i++)
	{
		GLuint names = *ptr;
		ptr += 3;

		if(names == 2 && *ptr == type)
		{
			ptr++;
			node->select(*ptr, type);
			ptr++;
		}
		else
		{
			ptr += names;
		}
	}
}

/**
 * Selects the building elements of \c type from \c node in the given circle by calling circleSelect() for each element.
 * \c node has to be an inherited from Drawable, so it gets a valid circleSelect()
 * \param	node	The an object inherited from Drawable which gets searched for its primitives.
 * \param	type	Type of primitive which are searched.
 * \param	xc		\e x coordinate of the searching circle's center.
 * \param	yc		\e y coordinate of the searching circle's center.
 * \param	r		Radius of the searching circle.
 * \sa doSelect()
 */
void Selection::doCircleSelect(ADrawable *node, unsigned type, int xc, int yc, int r)
{
	int x0 = xc - r;
	int y0 = yc - r;
	int x1 = xc + r;
	int y1 = yc + r;

	GLint hits;

	// start selection mode
	glSelectBuffer(BUFSIZE, selectBuffer);
	(void)glRenderMode(GL_SELECT);

	// clear the name stack, push initial name on it
	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	// no mirroring is required here as coordinates come from the bone structure (bone.getCenter),
	// which has its coordinates in the opengl coordinate system, not in the window coordinate system
	glOrtho(x0, x1, y0, y1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	node->draw(RENDER_TEXTURE | RENDER_WIREFRAME);

	// restore the projection matrix
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();

	// go back to render mode, and process selection buffer
	hits = glRenderMode(GL_RENDER);

	GLuint *ptr = (GLuint*)selectBuffer;

	for (GLint i = 0; i < hits; i++)
	{
		GLuint names = *ptr;
		ptr += 3;

		if (names == 2 && *ptr == type)
		{
			ptr++;
			node->circleSelect(*ptr, type, xc, yc, r);
			//node->select(*ptr, type);
			ptr++;
		}
		else
		{
			ptr += names;
		}
	}
}

int Selection::processFeedback(int size, float buffer[], float points[])
{
	int count = size;
	int index = 0;
	int passthrough = 0;

	while(count)
	{
		GLfloat token = buffer[size - count];
		count--;

		if(token == GL_PASS_THROUGH_TOKEN)
		{
//			printf("GL_PASS_THROUGH_TOKEN\n");
//			printf("  %4.2f\n", buffer[size - count]);

			// if the actual passthrough token comes directly after the previous passthrough token
			// then the primitive after the previous passthrough is out of screen
			if(passthrough == 1)
			{
				points[index++] = OUT_OF_SCREEN;
				points[index++] = OUT_OF_SCREEN;
			}

			points[index++] = buffer[size - count];
			passthrough = 1;
			count--;
		}
		else if(token == GL_POINT_TOKEN)
		{
//			printf("GL_POINT_TOKEN\n");
			for(int i = 0; i < 2; i++)
			{
				points[index++] = buffer[size - count];
//				printf("%4.2f ", buffer[size - count]);
				count--;
			}
			passthrough = 0;
//			printf("\n");
		}

		// others should not come
		else if(token == GL_LINE_TOKEN)
		{
			printf("GL_LINE_TOKEN\n");
			for(int n = 0; n < 2; n++)
			{
				for(int i = 0; i < 2; i++)
				{
					printf("%4.2f ", buffer[size - count]);
					count--;
				}
				printf("\n");
			}
			assert(false);
		}
		else if(token == GL_LINE_RESET_TOKEN)
		{
			printf("GL_LINE_RESET_TOKEN\n");
			for(int n = 0; n < 2; n++)
			{
				for(int i = 0; i < 2; i++)
				{
					printf("%4.2f ", buffer[size - count]);
					count--;
				}
				printf("\n");
			}
			assert(false);
		}
		else if(token == GL_POLYGON_TOKEN)
		{
			printf("GL_POLYGON_TOKEN\n");
			int n = (int)buffer[size - count];
			count--;
			for(; n > 0; n--)
			{
				for(int i = 0; i < 2; i++)
				{
					printf("%4.2f ", buffer[size - count]);
					count--;
				}
				printf("\n");
			}
			assert(false);
		}
		else
		{
			printf("UNRECOGNIZED\n");
			assert(false);
		}
	}

	// check if the last primitive is also out of screen
	if(passthrough == 1)
	{
		points[index++] = OUT_OF_SCREEN;
		points[index++] = OUT_OF_SCREEN;
	}

	return index;
}

/**
 * Draws the given layer in \c GL_FEEDBACK mode, and sets the view coordinates of every primitive to the result of the feedback.
 * Drawing the mesh and skeleton in \c GL_FEEDBACK mode, result in drawing only an individual vertex to the vertices and joints.
 * This position is then used for the particular primitive's view coordinate. All of this is required to let primitive's size
 * view-distance independent.
 *
 * \param	layer	The layer with a mesh and skeleton which view coordinates get computed.
 * \sa Mesh::setVertexViewCoords(), Skeleton::setJointViewCoords()
 */
void Selection::doFeedback(Layer *layer)
{
	int size, count;

	// if points array is to small to hold points found in feedback mode, allocate new array with double size
	// every point requires 3 value in the array
	if(pointsLength < layer->getMesh()->getVertices()->size() * 3)
	{
		pointsLength *= 2;

		delete [] points;
		points = new float[pointsLength];
	}

	if(pointsLength < layer->getSkeleton()->getJoints()->size() * 3)
	{
		pointsLength *= 2;

		delete [] points;
		points = new float[pointsLength];
	}

	// go to feedback mode
	glFeedbackBuffer(BUFSIZE, GL_2D, feedBuffer);
	glRenderMode(GL_FEEDBACK);

	// draw mesh in feedback mode
	layer->getMesh()->draw(RENDER_FEEDBACK);
	size = glRenderMode(GL_RENDER);

	// process the feedback buffer and set the vertex positions
	count = processFeedback(size, feedBuffer, points);
	layer->getMesh()->setVertexViewCoords(points, count);

	// go to feedback mode
	glFeedbackBuffer(BUFSIZE, GL_2D, feedBuffer);
	glRenderMode(GL_FEEDBACK);

	// draw skeleton in feedback mode
	layer->getSkeleton()->draw(RENDER_FEEDBACK);
	size = glRenderMode(GL_RENDER);

	// process the feedback buffer and set the joint positions
	count = processFeedback(size, feedBuffer, points);
	layer->getSkeleton()->setJointViewCoords(points, count);
}

