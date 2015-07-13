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
#include <float.h>
#include <algorithm>

#include "Layer.h"
#include "animata.h"
#include "animataUI.h"
#include "Transform.h"

#define MIN_SCALE 0.1f

using namespace Animata;

int Layer::layerCount = 0;

/**
 * Constructs a layer.
 * \param p pointer to parent layer
 **/
Layer::Layer(Layer *p)
{
	layers = new std::vector<Layer *>;
	parent = p;

	mesh = new Mesh();
	skeleton = new Skeleton();

	sprintf(name, "layer_%04d", Layer::layerCount);
	Layer::layerCount++;

	x = y = z = 0.f;
	alpha = 1.0;
	scale = 1.0;

	visible = true;

	calcTransformationMatrix();

	// add layer to vector of all layers
	if (ui) // FIXME: ui should not be NULL ever!
		ui->editorBox->addToAllLayers(this);
}

/**
 * Destroys layer including its sublayers.
 **/
Layer::~Layer()
{
	/* remove from all layers */
	if (ui)
	{
		ui->editorBox->lock();
		ui->editorBox->deleteFromAllLayers(this);
		ui->editorBox->unlock();
	}

	delete mesh;
	delete skeleton;

	eraseLayers();
}

/**
 * Erases all the layers.
 **/
void Layer::eraseLayers()
{
	if (layers)
	{
		std::vector<Layer *>::iterator l = layers->begin();
		for (; l < layers->end(); l++)
			delete *l;					// free layers from memory
		layers->clear();				// clear all vector elements
		delete layers;
		layers = NULL;
	}
}

/**
 * Overwrites the current layers with the given ones.
 * \param newLayers the vector containing the new layers
 * if NULL nothing will happen
 **/
void Layer::setLayers(std::vector<Layer *> *newLayers)
{
	if (newLayers)
	{
		eraseLayers();

		layers = newLayers;
	}
}

/**
 * Returns name of layer.
 * \return pointer to string
 **/
const char *Layer::getName(void)
{
	return name;
}

/**
 * Sets name of layer.
 * \param str pointer to string, only the first 15 characters will be used
 **/
void Layer::setName(const char *str)
{
	strncpy(name, str, 15);
	name[15] = 0;
}

/**
 * Sets layer parameters.
 * \param x x-coordinate
 * \param y y-coordinate
 * \param z depth
 * \param alpha alpha
 * \param scale scale
 */
void Layer::setup(float x, float y, float z, float alpha, float scale)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->alpha = alpha;
	this->scale = scale;

	calcTransformationMatrix();
}

/**
 * Changes the layer's scale around a given point relative to the layer's transformation space.
 * \param	s	new value of the layer's scale
 * \param	ox	scale center's x coordinate inside the layer's coordinate system
 * \param	oy	scale center's y coordinate inside the layer's coordinate system
 **/
// TODO: same as in Texture.cpp, if it's needed somewhere else, put it into a central place
void Layer::scaleAroundPoint(float s, float ox, float oy)
{
	if (s > MIN_SCALE)
	{
		x -= ox * (s - scale);
		y -= oy * (s - scale);

		scale = s;
	}
	calcTransformationMatrix();
}


/**
 * Draws layer.
 **/
void Layer::drawWithoutRecursion(int mode)
{
	if (!visible)
		return;

	// get the boundaries of actual viewport
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixf(transformation.f);

	Camera *cam = ui->editorBox->getCamera();
	float camZ = cam->getTarget()->z - cam->getDistance();

	/* if we are in mesh editing mode only draw the current layer */
	if ((((ui->settings.mode >= ANIMATA_MODE_CREATE_VERTEX) &&
		(ui->settings.mode <= ANIMATA_MODE_MESH_DELETE) &&
		(this == ui->editorBox->getCurrentLayer())) ||
		/* draw current layer only in image mode */
		((ui->settings.mode >= ANIMATA_MODE_TEXTURE_POSITION) &&
		(ui->settings.mode <= ANIMATA_MODE_TEXTURE_SCALE) &&
		(this == ui->editorBox->getCurrentLayer())) ||
		/* draw the layer if not in mesh editing or image mode */
		(((ui->settings.mode >= ANIMATA_MODE_CREATE_JOINT) &&
		  (ui->settings.mode <= ANIMATA_MODE_SKELETON_DELETE)) ||
		 ((ui->settings.mode >= ANIMATA_MODE_LAYER_MOVE) &&
		  (ui->settings.mode <= ANIMATA_MODE_LAYER_DEPTH)) ||
		 /* FIXME: camera & view have no ANIMATA_MODE set */
		 (ui->settings.mode == ANIMATA_MODE_NONE)) ||
		mode & RENDER_OUTPUT) &&
		/* don't draw the layer if its behind the camera */
		(transformation[14] > camZ))
	{
		mesh->setTextureAlpha(getAccumulatedAlpha());

		if (mode & RENDER_FEEDBACK)
		{
			// set the transformation matrices for setVertexViewCoords() and setJointViewCoords() in doFeedback
			Transform::setMatrices();

			selector->doFeedback(this);
		}

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(viewport[0], viewport[2] + viewport[0], viewport[1], viewport[3] + viewport[1], 0, 1);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		mesh->draw(mode & ~RENDER_FEEDBACK, this == ui->editorBox->getCurrentLayer());
		skeleton->draw(mode & ~RENDER_FEEDBACK, this == ui->editorBox->getCurrentLayer());

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

/**
 * Run physical simulation on the skeleton of the layer and all sublayers.
 * \param times iteration count
 **/
void Layer::simulate(int times)
{
	/* simulate only visible layers */
	if (!visible)
		return;

	skeleton->simulate(times);

	// simulate sublayers
	std::vector<Layer *>::iterator l = layers->begin();
	for (; l < layers->end(); l++)
		(*l)->simulate(times);
}

/**
 * Calculates the transformation matrix by traversing backwards the layer tree
 * and right multiplying its transformation matrices to the current matrix.
 * This way the transformation chain will be the same as in the opengl stack.
 **/
void Layer::calcTransformationMatrix()
{
	transformation.loadIdentity();
	transformation.scale(scale, scale, 1.0f);

	transformation.translate(x, y, z);

	Layer *actLayer = this;
	while(actLayer->getParent())
	{
		actLayer = actLayer->getParent();
		transformation.scale(actLayer->getScale(), actLayer->getScale(), 1.0f);
		transformation.translate(actLayer->getX(), actLayer->getY(), actLayer->getZ());
	}

	// transformation matrix of this layer changed
	// also recalculate transformation matrices of the sublayers
	std::vector<Layer *>::iterator l = layers->begin();
	for (; l < layers->end(); l++)
		(*l)->calcTransformationMatrix();
}


/**
 * Creates a new layer with the default name and pushes it after the old ones.
 * The parent of the new layer will be the container (this).
 * \return pointer to the new layer
 **/
Layer *Layer::makeLayer()
{
	Layer *l = new Layer(this);

	if (!layers)
		layers = new std::vector<Layer *>;

	layers->push_back(l);

	return l;
}

/**
 * Adds sublayer.
 * \param sublayer pointer of layer to add
 **/
void Layer::addSublayer(Layer *sublayer)
{
	layers->push_back(sublayer);
	sublayer->setParent(this);
}

/**
 * Adds sublayers.
 * \param sublayers pointer to layers vector to add
 **/
void Layer::addSublayers(std::vector<Layer *> *sublayers)
{
	std::vector<Layer *>::iterator l = sublayers->begin();
	for (; l < sublayers->end(); l++)
	{
		layers->push_back(*l);
	}
}

/**
 * Deletes sublayer.
 * \param layer pointer of layer to delete
 * \return 0 on success, -1 of failure
 **/
int Layer::deleteSublayer(Layer *layer)
{
	std::vector<Layer *>::iterator pos;

	// find position of layer in vector
	pos = find(layers->begin(), layers->end(), layer);
	if (pos == layers->end()) // not a member
		return -1;

	layers->erase(pos);
	delete layer;

	return 0;
}

/**
 * Returns alpha accumulated through the layer hierarchy.
 * The alpha value is multiplied by the parents' alpha.
 * \return layer alpha value
 **/
float Layer::getAccumulatedAlpha(void)
{
	if (parent == NULL)
	{
		return alpha;
	}
	else
	{
		return alpha * parent->getAccumulatedAlpha();
	}
}

/**
 * Sets visibility recursively.
 * \param v visibility parameter
 **/
void Layer::setVisibility(bool v)
{
	visible = v;

	vector<Layer *>::iterator l = layers->begin();
	for (; l < layers->end(); l++)
		(*l)->setVisibility(v);
}

