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

#ifndef __SKELETON_H__
#define __SKELETON_H__

#include <float.h>
#include <vector>

#include "Vector2D.h"
#include "Joint.h"
#include "Bone.h"
#include "Preferences.h"

using namespace std;

namespace Animata
{

/// Skeleton attached to the mesh
class Skeleton : public ADrawable
{
	public:
		Skeleton();
		~Skeleton();

		Joint *addJoint(float x, float y);
		Bone *addBone(Joint *j0, Joint *j1);

		int moveSelectedJoints(float dx, float dy);
		int moveSelectedBones(float dx, float dy);
		void endMoveSelectedJoints(void);
		void endMoveSelectedBones(void);

		void setSelectedJointParameters(enum ANIMATA_PREFERENCES prefParam,
				void *value);
		void setSelectedBoneParameters(const char *str = NULL,
			float s = FLT_EPSILON, float lengthMultiply = -1, float aRad = FLT_MAX,
			float falloff = FLT_MAX);

		void setSelectedBoneLengthMultMin(float p);
		void setSelectedBoneLengthMultMax(float p);
		void setSelectedBoneTempo(float p);

		void deleteSelectedJoint(void);
		void deleteSelectedBone(void);

		void clearSelection(void);

		void setJointViewCoords(float *coords, unsigned int size);

		virtual void draw(int mode, int active = 1);
		virtual void select(unsigned i, int type);
		virtual void circleSelect(unsigned i, int type, int xc, int yc, float r);

		void simulate(int times = 1);

		void attachVertices(vector<Vertex *> *verts);
		void disattachVertices(void);
		void disattachSelectedVertex(Vertex *v);

		void selectVerticesInRange(Mesh *mesh);

		/// Returns the joint below the mouse cursor.
		inline Joint *getPointedJoint(void) { return pJoint; }
		/// Returns the bone below the mouse cursor.
		inline Bone *getPointedBone(void) { return pBone; }

		/// Returns skeleton joints.
		inline vector<Joint *> *getJoints(void) { return joints; }
		/// Returns skeleton bones.
		inline vector<Bone *> *getBones(void) { return bones; }

	private:
		vector<Joint *> *joints;
		vector<Bone *> *bones;

		Joint	*pJoint;	/**< joint below the cursor */
		Bone	*pBone;		/**< bone below the cursor */
};

} /* namespace Animata */

#endif

