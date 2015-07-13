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

#include "animata.h"
#include "animataUI.h"
#include "Primitives.h"
#include "Skeleton.h"
#include "Transform.h"

using namespace Animata;

/**
 * Creates a skeleton with no joints and bones.
 **/
Skeleton::Skeleton()
{
	joints = new vector<Joint*>;
	pJoint = NULL;

	bones = new vector<Bone *>;
	pBone = NULL;
}

/**
 * Destroys the skeleton and frees allocated memory.
 **/
Skeleton::~Skeleton()
{
	if (joints)
	{
		vector<Joint *>::iterator j = joints->begin();
		for (; j < joints->end(); j++)
			delete *j;	/* free joints from memory */
		joints->clear(); /* clear all vector elements */
		delete joints;
	}

	if (bones)
	{
		vector<Bone *>::iterator b = bones->begin();
		for (; b < bones->end(); b++)
			delete *b;	/* free bones from memory */
		bones->clear(); /* clear all vector elements */
		delete bones;
	}
}

/**
 * Adds a new joint to the skeleton.
 * \param x x-coordinate of the joint
 * \param y y-coordinate of the joint
 * \return pointer to the new joint
 **/
Joint *Skeleton::addJoint(float x, float y)
{
	Joint *j = new Joint(x, y);
	joints->push_back(j);

	/* add to vector of all joints */
	if (ui) // FIXME: ui should not be NULL!
		ui->editorBox->addToAllJoints(j);
	return j;
}

/**
 * Creates a new bone.
 * \param j0 pointer to one joint
 * \param j1 pointer to the other joint
 * \return pointer to the new bone
 **/
Bone *Skeleton::addBone(Joint *j0, Joint *j1)
{
	if (j0 == j1)
		return NULL;

	/* check if a previous bone exists between these two joints */
	for (unsigned i = 0; i < bones->size(); i++)
	{
		Bone *bone = (*bones)[i];
		if (((bone->j0 == j0) && (bone->j1 == j1)) ||
			((bone->j0 == j1) && (bone->j1 == j0)))
		{
			return NULL;
		}
	}

	/* make a new bone */
	Bone *b = new Bone(j0, j1);
	bones->push_back(b);

	/* add to vector of all bones */
	if (ui) // FIXME: ui should not be NULL!
		ui->editorBox->addToAllBones(b);
	return b;
}

/**
 * Moves selected joints.
 * \param dx x distance to move by
 * \param dy y distance to move by
 * \return number of joints moved
 **/
int Skeleton::moveSelectedJoints(float dx, float dy)
{
	int movedJoints = 0;

	for (unsigned i = 0; i < joints->size(); i++)
	{
		Joint *j = (*joints)[i];

		if (j->selected)
		{
			j->drag(dx, dy);
			movedJoints++;
		}
	}
	/* return the number of joints moved */
	return movedJoints;
}

/**
 * Called when the dragging of selected joints has ended.
 **/
void Skeleton::endMoveSelectedJoints(void)
{
	for (unsigned i = 0; i < joints->size(); i++)
		(*joints)[i]->dragged = false;
}

/**
 * Moves selected bones.
 * \param dx x distance to move by
 * \param dy y distance to move by
 * \return number of bones moved
 **/
int Skeleton::moveSelectedBones(float dx, float dy)
{
	int movedBones = 0;

	/* timeStamp to prevent joints moved twice if they belong to
	 * multiple bones */
	static int timeStamp = 0;

	for (unsigned i = 0; i < bones->size(); i++)
	{
		Bone *b = (*bones)[i];

		if (b->selected)
		{
			b->drag(dx, dy, timeStamp);
			movedBones++;
		}
	}

	timeStamp++;
	/* return the number of bones moved */
	return movedBones;
}

/**
 * Called when the dragging of selected bones has ended.
 **/
void Skeleton::endMoveSelectedBones(void)
{
	for (unsigned i = 0; i < bones->size(); i++)
		(*bones)[i]->release();
}

/**
 * Sets the parameters of the selected joint.
 * \param prefParam parameter to set
 * \param value parameter value cast to (void *)
 **/
void Skeleton::setSelectedJointParameters(enum ANIMATA_PREFERENCES prefParam, void *value)
{
	for (unsigned i = 0; i < joints->size(); i++)
	{
		Joint *j = (*joints)[i];

		if (j->selected)
		{
			switch (prefParam)
			{
				case PREFS_JOINT_NAME:
					j->setName(*((const char **)value));
					break;
				case PREFS_JOINT_X:
					j->x = *((float *)value);
					break;
				case PREFS_JOINT_Y:
					j->y = *((float *)value);
					break;
				case PREFS_JOINT_FIXED:
					j->fixed = *((int *)value);
					break;
				case PREFS_JOINT_OSC:
					{
						int osc = *((int *)value);
						j->osc = osc;
						// add or remove the joint from the vector of joints
						// needed to be sent via OSC
						if (osc)
						{
							ui->editorBox->addToOSCJoints(j);
						}
						else
						{
							ui->editorBox->deleteFromOSCJoints(j);
						}
						break;
					}
				default:
					break;
			}
		}
	}
}

/**
 * Sets the parameters of the selected bone.
 * \param str set name if it is not NULL
 * \param s set stiffness if it is bigger than zero
 * \param lm set length multiplier if it bigger or equal to zero
 * \param aRad set vertex attachment radius if it is not FLT_MAX
 * \param falloff set falloff if it is not FLT_MAX
 **/
void Skeleton::setSelectedBoneParameters(const char *str, float s, float lm, float aRad,
		float falloff)
{
	for (unsigned i = 0; i < bones->size(); i++)
	{
		Bone *b = (*bones)[i];

		if (b->selected)
		{
			if (s > FLT_EPSILON)
				b->damp = s;
			if (str)
				b->setName(str);
			if (lm >= 0)
				b->setLengthMult(lm);
			if (aRad < FLT_MAX)
				b->setRadiusMult(aRad);
			if (falloff < FLT_MAX)
			{
				b->setFalloff(falloff); // set new falloff value
				// calculate new weights of attached vertices
				// FIXME: should be calculated when the attach
				// button is pressed
				b->recalculateWeights();
			}
		}
	}
}

/**
 * Sets minimum bone length multiplier of selected bones.
 * \param p multiplier value
 **/
void Skeleton::setSelectedBoneLengthMultMin(float p)
{
	for (unsigned i = 0; i < bones->size(); i++)
	{
		Bone *b = (*bones)[i];

		if (b->selected)
		{
			b->setLengthMultMin(p);
		}
	}
}

/**
 * Sets maximum bone length multiplier of selected bones.
 * \param p multiplier value
 **/
void Skeleton::setSelectedBoneLengthMultMax(float p)
{
	for (unsigned i = 0; i < bones->size(); i++)
	{
		Bone *b = (*bones)[i];

		if (b->selected)
		{
			b->setLengthMultMax(p);
		}
	}
}

/**
 * Sets tempo of selected bones.
 * \param p tempo
 **/
void Skeleton::setSelectedBoneTempo(float p)
{
	for (unsigned i = 0; i < bones->size(); i++)
	{
		Bone *b = (*bones)[i];

		if (b->selected)
		{
			b->setTempo(p);
		}
	}
}

/**
 * Deletes the selected joint.
 **/
void Skeleton::deleteSelectedJoint(void)
{
	unsigned char hit = selector->getHitCount();
	SelectItem *selected = selector->getSelected();

	/* select the joint from the selection buffer */
	Joint *selJoint = NULL;
	for (unsigned int i = 0; i < hit; i++)
	{
		if (selected->type == Selection::SELECT_JOINT)
		{
			selJoint = (*joints)[selected->name];
			break;
		}
		selected++;
	}

	if (selJoint == NULL) /* no joint below the cursor */
		return;

	/* if the selected joint is part of a bone, delete it -
	 * checking vector elements backwards to be able to iterate and erase
	 * at the same time */
	for (int i = bones->size() - 1; i >= 0; i--)
	{
		Bone *bone = (*bones)[i];
		if ((bone->j0 == selJoint) ||
			(bone->j1 == selJoint))
		{
			vector<Bone *>::iterator boneIter = bones->begin() + i;
			delete bone;
			bones->erase(boneIter);
		}
	}

	/* delete the joint */
	vector<Joint *>::iterator iter = joints->begin() + selected->name;
	/* delete the joint from vector of all joints */
	if (ui) // FIXME: ui should not be NULL
		ui->editorBox->deleteFromAllJoints(*iter);
	delete *iter; /* delete object */
	joints->erase(iter); /* remove it from the vector */
	/* current selection points to the next joint after the deleted one */
	selector->clearSelection();
}

/**
 * Deletes the selected bone.
 **/
void Skeleton::deleteSelectedBone(void)
{
	unsigned hit = selector->getHitCount();
	SelectItem *selected = selector->getSelected();

	/* select the bone from the selection buffer */
	Bone *selBone = NULL;
	for (unsigned int i = 0; i < hit; i++)
	{
		if (selected->type == Selection::SELECT_BONE)
		{
			selBone = (*bones)[selected->name];
			break;
		}
		selected++;
	}

	if (selBone == NULL) /* no bone below the cursor */
		return;

	/* delete the bone and references to it*/
	vector<Bone *>::iterator iter = bones->begin() + selected->name;
	/* delete the bone from vector of all bones */
	if (ui) // FIXME: ui should not be NULL
		ui->editorBox->deleteFromAllBones(*iter);
	delete *iter; /* delete object */
	bones->erase(iter); /* remove it from the vector */
	/* clear selection, because it contains a non-existing object */
	selector->clearSelection();

}

/**
 * Clears the selection of bones and joints.
 **/
void Skeleton::clearSelection(void)
{
	for (unsigned i = 0; i < joints->size(); i++)
	{
		(*joints)[i]->selected = false;
	}

	for (unsigned i = 0; i < bones->size(); i++)
	{
		(*bones)[i]->selected = false;
	}
}

/**
 * Attaches vertices to the selected bone.
 * \param verts pointer to the vertices vector holding the vertices to
 * be attached
 **/
void Skeleton::attachVertices(vector<Vertex *> *verts)
{
	Bone *selectedBone = NULL;

	int s = 0;
	for (unsigned i = 0; i < bones->size(); i++)
	{
		if ((*bones)[i]->selected)
		{
			selectedBone = (*bones)[i];
			s++;
		}
	}

	if (s == 1)
	{
		selectedBone->attachVertices(verts);
		delete verts;
	}
}

/**
 * Select vertices in bone range
 * if there are no attached vertices use circle selection
 * otherwise select attached vertices
 * \param mesh pointer to mesh
 **/
void Skeleton::selectVerticesInRange(Mesh *mesh)
{
	mesh->clearSelection();
	for (unsigned i = 0; i < bones->size(); i++)
	{
		Bone *b = (*bones)[i];

		/* select vertices in selection circle only if there are no vertices
		 * attached */
		if (b->selected)
		{
			if (b->getAttachedVerticesCount() == 0)
			{
				// selection happens in screen coordinate system, just like the drawSelectionBox in animata.cpp
				// so get the view radius as in Bone.draw()
				Vector2D v = b->getCenter();
				// float r = b->getRadius();
				float r = b->getViewRadius();
				selector->doCircleSelect(mesh, Selection::SELECT_VERTEX,
						(int)v.x, (int)v.y, (int)r);
			}
			else
			{
				b->selectAttachedVertices();
			}
		}
	}
}

/**
 * Disattaches vertices from bone.
 **/
void Skeleton::disattachVertices(void)
{
	for (unsigned i = 0; i < bones->size(); i++)
	{
		Bone *b = (*bones)[i];
		if (b->selected)
		{
			b->selectAttachedVertices(false); // clear selection
			b->disattachVertices();
		}
	}
}

/**
 * Disattaches the given vertex.
 * \param v pointer to vertex
 **/
void Skeleton::disattachSelectedVertex(Vertex *v)
{
	if (v == NULL) /* no vertex below the cursor */
		return;

	for (unsigned i = 0; i < bones->size(); i++)
	{
		(*bones)[i]->disattachVertex(v);
	}
}

/**
 * Sets the view coordinates of the joints of this skeleton.
 * Setting the transformation matrices by Transform::setMatrices() is neccesary before calling this,
 * because Transform::project() is used for non visible primitives.
 **/
void Skeleton::setJointViewCoords(float *coords, unsigned int size)
{
	for(unsigned i = 0; i < size; i+=3)
	{
		unsigned n = (unsigned)coords[i];
		Joint *j = (*joints)[n];

		// joint is out of screen lets do projection here
		if(i + 1 < size && coords[i + 1] == Selection::OUT_OF_SCREEN)
		{
			Vector3D view = Transform::project(j->x, j->y, 0);

			j->vx = view.x;
			j->vy = view.y;
		}
		else
		{
			j->vx = coords[i + 1];
			j->vy = coords[i + 2];
		}
	}
}

/**
 * Draws the skeleton.
 * \param mode bitmask of RENDER_WIREFRAME, RENDER_FEEDBACK or RENDER_OUTPUT, determines
 *		how the primitives are drawn.
 * \param active active state
 **/
void Skeleton::draw(int mode, int active)
{
	unsigned hit = 0;
	SelectItem *selected = NULL;

	if(selector->getPickLayer() && selector->getPickLayer()->getSkeleton() == this)
	{
		hit = selector->getHitCount();
		selected = selector->getSelected();
	}

	// select objects from the selection buffer
	pJoint = NULL;
	pBone = NULL;
	for (unsigned int i = 0; i < hit; i++)
	{
		if (selected->type == Selection::SELECT_JOINT &&
			((ui->settings.mode == ANIMATA_MODE_CREATE_JOINT) ||
			 (ui->settings.mode == ANIMATA_MODE_SKELETON_SELECT) ||
			 (ui->settings.mode == ANIMATA_MODE_SKELETON_DELETE) ||
			 (ui->settings.mode == ANIMATA_MODE_CREATE_BONE)))
		{
			/* joints are prefered to bones if they overlap */
			pJoint = (*joints)[selected->name];
			pBone = NULL;
			break;
		}
		else
		if (selected->type == Selection::SELECT_BONE &&
				(ui->settings.mode != ANIMATA_MODE_CREATE_BONE))
		{
			pBone = (*bones)[selected->name];
		}

		selected++;
	}

	if ((mode & RENDER_WIREFRAME) &&
		((!(mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_EDITOR_BONE) ||
		((mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_OUTPUT_BONE)))
	{
		glLoadName(Selection::SELECT_BONE);	/* type of primitive */
		glPushName(0);						/* id of primitive */

		for (unsigned i = 0; i < bones->size(); i++)
		{
			Bone *bone = (*bones)[i];

			glLoadName(i);

			if(mode & RENDER_OUTPUT)
				bone->draw(false);
			else
				bone->draw(bone == pBone, active);
		}

		glPopName();
	}

	if(mode & RENDER_FEEDBACK)
	{
		for (unsigned i = 0; i < joints->size(); i++)
		{
			Joint *joint = (*joints)[i];

			glPassThrough(i);
			glBegin(GL_POINTS);
				glVertex2f(joint->x, joint->y);
			glEnd();
		}
	}
	else if ((mode & RENDER_WIREFRAME) &&
			 ((!(mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_EDITOR_JOINT) ||
			 ((mode & RENDER_OUTPUT) && ui->settings.display_elements & DISPLAY_OUTPUT_JOINT)))
	{
		glLoadName(Selection::SELECT_JOINT);	/* type of primitive */
		glPushName(0);							/* id of primitive */

		for (unsigned i = 0; i < joints->size(); i++)
		{
			Joint *joint = (*joints)[i];

			glLoadName(i);

			if(mode & RENDER_OUTPUT)
				joint->draw(false);
			else
				joint->draw(joint == pJoint, active);
		}

		glPopName();
	}
}

/**
 * Selects skeleton primitives.
 * \param i primitive index
 * \param type primitive type (Selection::SELECT_JOINT)
 **/
void Skeleton::select(unsigned i, int type)
{
	switch (type)
	{
		case Selection::SELECT_JOINT:
			if (i < joints->size())
			{
				(*joints)[i]->selected = true;
			}
			break;
	}
}

/**
 * Not implemented.
 **/
void Skeleton::circleSelect(unsigned i, int type, int xc, int yc, float r)
{
}

/**
 * Runs the simulation on joints and bones
 * \param times number of times to run the simulation
 **/
void Skeleton::simulate(int times /* = 1 */)
{
	for (int t = 0; t < times; t++)
	{
		for (unsigned i = 0; i < joints->size(); i++)
		{
			((*joints)[i])->simulate();
		}
		for (unsigned i = 0; i < bones->size(); i++)
		{
			((*bones)[i])->simulate();
			((*bones)[i])->translateVertices();
		}
	}
}

