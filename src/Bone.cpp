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
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "animata.h"
#include "animataUI.h"
#include "Bone.h"
#include "Primitives.h"

using namespace Animata;

/**
 * Constructs a bone from the two given joints.
 * \param j0 pointer to joint 0
 * \param j1 pointer to joint 1
 **/
Bone::Bone(Joint *j0, Joint *j1)
{
	this->j0 = j0;
	this->j1 = j1;

	damp = BONE_DEFAULT_DAMP;

	float x0 = j0->x;
	float y0 = j0->y;
	float x1 = j1->x;
	float y1 = j1->y;

	setName("");

	dOrig = sqrt((x1-x0)*(x1-x0) + (y1-y0)*(y1-y0));

	selected = false;

	dsts = weights = sa = ca = NULL;
	attachedVertices = new vector<Vertex *>;

	attachRadiusMult = 1.0;
	falloff = 1.0;

	lengthMult = BONE_DEFAULT_LENGTH_MULT;
	lengthMultMin = BONE_DEFAULT_LENGTH_MULT_MIN;
	lengthMultMax = BONE_DEFAULT_LENGTH_MULT_MAX;

	tempo = 0.0;
	time = M_PI_2; // pi/2 -> bone animation starts at maximum length

}

Bone::~Bone()
{
	disattachVertices();
	delete attachedVertices;
}

/**
 * Runs the spring simulation on the bone.
 **/
void Bone::simulate(void)
{
	if (tempo > 0)
	{
		time += tempo / 42.0f;	// FIXME
		animateLengthMult(0.5 + sin(time) * 0.5f);
	}

	float x0 = j0->x;
	float y0 = j0->y;
	float x1 = j1->x;
	float y1 = j1->y;

	float dx = (x1 - x0);
	float dy = (y1 - y0);
	float dCurrent = sqrt(dx*dx + dy*dy);

	if (dCurrent > FLT_EPSILON)
	{
		dx /= dCurrent;
		dy /= dCurrent;
	}

	float m = ((dOrig * lengthMult) - dCurrent) * damp;

	if (!j0->fixed && !j0->dragged)
	{
		j0->x -= m*dx;
		j0->y -= m*dy;
	}

	if (!j1->fixed && !j1->dragged)
	{
		j1->x += m*dx;
		j1->y += m*dy;
	}

}

/**
 * Sets bone length multiplier.
 * \param lm multiplier value
 * \remark Bone length multiplier is clamped between the minimum and maximum
 *		bone lengths.
 **/
void Bone::setLengthMult(float lm)
{
	if (lm < lengthMultMin)
		lm = lengthMultMin;
	else if (lm > lengthMultMax)
		lm = lengthMultMax;

	lengthMult = lm;
}

/**
 * Animates length multiplier between the minimum and maximum values.
 * \param t value between 0 and 1
 **/
void Bone::animateLengthMult(float t)
{
	lengthMult = lengthMultMin + (lengthMultMax - lengthMultMin) * t;

	if (selected) // TODO: find a place for this call
	{
		ui->boneLengthMult->value(lengthMult);
	}
}

/**
 * Animates bone. Changes bone length multiplier between the minimum and
 * maximum.
 * \param t float number that will be clamped between 0 and 1
 **/
void Bone::animateBone(float t)
{
	if (t < 0.0f)
		t = 0.0f;
	else if (t > 1.0f)
		t = 1.0f;
	animateLengthMult(t);
}

/**
 * Translates attached vertices of bone.
 * Attached vertices try to maintain their relative position to
 * the bone centre.
 **/
void Bone::translateVertices(void)
{
	float x0 = j0->x;
	float y0 = j0->y;
	float x1 = j1->x;
	float y1 = j1->y;

	float dx = (x1 - x0);
	float dy = (y1 - y0);

	float x = x0 + dx * 0.5f;
	float y = y0 + dy * 0.5f;

	float dCurrent = sqrt(dx*dx + dy*dy);
	if (dCurrent < FLT_EPSILON)
	{
		dCurrent = FLT_EPSILON;
	}
	dx /= dCurrent;
	dy /= dCurrent;

	for (unsigned i = 0; i < attachedVertices->size(); i++)
	{
		Vertex *v = (*attachedVertices)[i];

		float vx = v->coord.x;
		float vy = v->coord.y;

		float tx = x + (dx * ca[i] - dy * sa[i]);
		float ty = y + (dx * sa[i] + dy * ca[i]);

		vx += (tx - vx) * weights[i];
		vy += (ty - vy) * weights[i];
		v->coord.x = vx;
		v->coord.y = vy;
	}
}

/**
 * Moves bone by the given vector.
 * \param dx x distance
 * \param dy y distance
 * \param timeStamp timestamp to prevent moving joints multiple times when
 *		they belong to several bones
 **/
void Bone::drag(float dx, float dy, int timeStamp /* = 0*/)
{
	j0->drag(dx, dy, timeStamp);
	j1->drag(dx, dy, timeStamp);
}

/**
 * Releases bone after dragging.
 **/
void Bone::release(void)
{
	j0->dragged = false;
	j1->dragged = false;
}

/**
 * Draws bone.
 * \param mouseOver 1 if the mouse is over the bone
 * \param active 1 if it is active
 **/
void Bone::draw(int mouseOver, int active)
{
	Primitives::drawBone(this, mouseOver, active);

	// FIXME: screws up selection
	if (ui->settings.mode == ANIMATA_MODE_ATTACH_VERTICES)
	{
		int cx = (int)((j0->vx + j1->vx) * .5f);
		int cy = (int)((j0->vy + j1->vy) * .5f);
		// cannot use getRadius(), as it isn't view dependent, like joint.vx and joint.vy
		// int r = (int)(getRadius());
		int r = (int)(getViewRadius());

		if (selected)
		{
			Primitives::drawSelectionCircle(cx, cy, r);
			for (unsigned i = 0; i < attachedVertices->size(); i++)
			{
				Vertex *v = (*attachedVertices)[i];
				Primitives::drawVertexAttached(v);
			}
		}
	}

}

/**
 * Inverts the bone's state of selection.
 **/
void Bone::flipSelection(void)
{
	selected = !selected;
}

/**
 * Returns the name of the bone.
 * \return pointer to name
 **/
const char *Bone::getName(void)
{
	return name;
}

/**
 * Sets the name of the bone.
 * \param str pointer to name string
 * \remark Only the first 15 characters will be used.
 **/
void Bone::setName(const char *str)
{
	strncpy(name, str, 15);
	name[15] = 0;
}

/**
 * Gets bone center coordinates.
 * \return center vector
 **/
Vector2D Bone::getCenter(void)
{
	Vector2D c((j0->vx + j1->vx) * .5f, (j0->vy + j1->vy) * .5f);
	return c;
}

/**
 * Gets radius of vertex selection circle in view coordinate-system.
 * \return The radius of the selection circle in view coordinate-system.
 */
float Bone::getViewRadius()
{
	// distance has to be recalculated every time as it's based on the view position of the joints
	float dView = sqrt((j1->vx-j0->vx)*(j1->vx-j0->vx) + (j1->vy-j0->vy)*(j1->vy-j0->vy));
	return dView * 0.5f * attachRadiusMult;
}

/**
 * Attaches vertices from given vector to the bone.
 * \param verts pointer to vector of vertices
 **/
void Bone::attachVertices(vector<Vertex *> *verts)
{
	unsigned count = verts->size();

	/* clear previously attached vertices */
	disattachVertices();

	dsts = new float[count];
	weights = new float[count];
	sa = new float[count];
	ca = new float[count];

	float x0 = j0->x;
	float y0 = j0->y;
	float x1 = j1->x;
	float y1 = j1->y;

	float alpha = atan2(y1 - y0, x1 - x0);
	float dx = (x1 - x0);
	float dy = (y1 - y0);
	float dCurrent = sqrt(dx*dx + dy*dy);
	float x = x0 + dx * 0.5f;
	float y = y0 + dy * 0.5f;

	dx /= dCurrent;
	dy /= dCurrent;

	for (unsigned i = 0; i < count; i++)
	{
		Vertex *v = (*verts)[i];

		if (v->selected)
		{
			float vx = v->coord.x;
			float vy = v->coord.y;
			float vd = sqrt((x - vx) * (x - vx) + (y - vy) * (y - vy));

			dsts[i] = vd;

			float vdnorm = vd / (attachRadiusMult * dOrig * .5f);

			if (vdnorm >= 1)
			{
				weights[i] = BONE_MINIMAL_WEIGHT;
			}
			else
			{
				weights[i] = pow(1.0 - vdnorm, 1.0 / falloff);
			}

			float a = atan2(vy - y, vx - x) - alpha;
			sa[i] = vd * (sin(a));
			ca[i] = vd * (cos(a));

			attachedVertices->push_back(v);
		}
	}
}

/**
 * Attaches vertices with given parameters.
 * \param verts pointer to vector of vertices
 * \param dsts distance array holding distance from bone centre
 *				for all vertices
 * \param weights array holding vertex weights
 * \param ca array of cosinus angles
 * \param sa array of sinus angles
 **/
void Bone::attachVertices(vector<Vertex *> *verts, float *dsts,
		float *weights, float *ca, float *sa)
{
	unsigned count = verts->size();

	/* clear previously attached vertices */
	disattachVertices();

	for (unsigned i = 0; i < count; i++)
	{
		attachedVertices->push_back((*verts)[i]);
	}
	this->dsts = dsts;
	this->weights = weights;
	this->ca = ca;
	this->sa = sa;
}

/**
 * Recalculates weights of attached vertices.
 **/
void Bone::recalculateWeights(void)
{
	unsigned count = attachedVertices->size();
	for (unsigned i = 0; i < count; i++)
	{
		float vd = dsts[i];

		float vdnorm = vd / (attachRadiusMult * dOrig * .5f);
		if (vdnorm >= 1)
		{
			weights[i] = BONE_MINIMAL_WEIGHT;
		}
		else
		{
			weights[i] = pow(1.0 - vdnorm, 1.0 / falloff);
		}
	}
}

/**
 * Disattach vertex from bone.
 * \param v pointer to vertex
 **/
void Bone::disattachVertex(Vertex *v)
{
	if (attachedVertices->empty() || v == NULL)
		return;

	// delete vertex pointer from vector
	int lasti = attachedVertices->size() - 1; // index of last element
	for (int vi = 0; vi <= lasti; vi++)
	{
		if ((*attachedVertices)[vi] == v)
		{
			// move last vertex to the place of the deleted one
			(*attachedVertices)[vi] = (*attachedVertices)[lasti];

			// rearrange helper arrays
			dsts[vi] = dsts[lasti];
			weights[vi] = weights[lasti];
			sa[vi] = sa[lasti];
			ca[vi] = ca[lasti];

			// remove last element of the vertices vector
			attachedVertices->pop_back();
			break;
		}
	}
}

/**
 * Selects attached vertices.
 * \param s bool, select/deselect
 **/
void Bone::selectAttachedVertices(bool s /* = true */)
{
	for (unsigned i = 0; i < attachedVertices->size(); i++)
	{
		((*attachedVertices)[i])->selected = s;
	}
}

/**
 * Disattaches vertices.
 **/
void Bone::disattachVertices(void)
{
	/* clear previously attached vertices */
	attachedVertices->clear();

	if (dsts)
	{
		delete [] dsts;
		dsts = NULL;
	}

	if (weights)
	{
		delete [] weights;
		weights = NULL;
	}

	if (sa)
	{
		delete [] sa;
		sa = NULL;
	}

	if (ca)
	{
		delete [] ca;
		ca = NULL;
	}
}

/**
 * Returns the vector of attached vertices.
 * \param dsts pointer to return distance array
 * \param weights pointer to return weights array
 * \param sa pointer to sinus values array
 * \param ca pointer to cosinus values array
 * \return attached vertices, their weight and distance from the bone centre
 *			as arrays
 **/
vector<Vertex *> *Bone::getAttachedVertices(float **dsts, float **weights,
		float **ca, float **sa)
{
	*dsts = this->dsts;
	*weights = this->weights;
	*ca = this->ca;
	*sa = this->sa;
	return attachedVertices;
}

/**
 * Gets the number of attached vertices.
 * \return number of attached vertices
 **/
int Bone::getAttachedVerticesCount(void)
{
	return (int)(attachedVertices->size());
}

