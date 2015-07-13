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

#ifndef __BONE_H__
#define __BONE_H__

#include "Mesh.h"
#include "Joint.h"

#define BONE_DEFAULT_DAMP .5
#define BONE_DEFAULT_LENGTH_MULT 1
#define BONE_DEFAULT_LENGTH_MULT_MIN .01
#define BONE_DEFAULT_LENGTH_MULT_MAX 1
#define BONE_MINIMAL_WEIGHT .01

using namespace std;

namespace Animata
{

/// Building element of Skeleton, the base of the physical simulation.
class Bone
{
	public:
		Bone(Joint *j0, Joint *j1);
		~Bone();

		void simulate(void);
		void translateVertices(void);

		void drag(float dx, float dy, int timeStamp = 0);
		void release(void);

		void draw(int mouseOver, int active = 1);
		void flipSelection(void);

		const char *getName(void);
		void setName(const char *str);

		void attachVertices(vector<Vertex *> *verts);

		void attachVertices(vector<Vertex *> *verts, float *dsts,
			float *weights, float *ca, float *sa);

		/// Selects attached vertices.
		void selectAttachedVertices(bool s = true);
		void disattachVertices(void);
		/// Disattaches one vertex.
		void disattachVertex(Vertex *v);

		/// Returns the vector of attached vertices.
		vector<Vertex *> *getAttachedVertices(float **dsts,
			float **weights, float **ca, float **sa);

		/// Returns number of attached vertices.
		int getAttachedVerticesCount(void);

		/// Gets bone size.
		inline float getOrigSize(void) { return dOrig; };
		/// Sets bone size.
		inline void setOrigSize(float s) { dOrig = s; };

		/// Sets bone length multiplier.
		void setLengthMult(float lm);
		/// Sets length multiplier minimum.
		inline void setLengthMultMin(float p) { lengthMultMin = p; }
		/// Sets length multiplier maximum.
		inline void setLengthMultMax(float p) { lengthMultMax = p; }

		/// Animates bone using its length multiplier.
		void animateBone(float t);

		/// Sets tempo of bone length change.
		inline void setTempo(float p) { tempo = p; }
		/// Sets oscillator time.
		inline void setTime(float p) { time = p; }

		/// Returns bone length multiplier.
		inline float getLengthMult(void) { return lengthMult; }
		/// Gets minimume length.
		inline float getLengthMultMin(void) { return lengthMultMin; }
		/// Gets maximum length.
		inline float getLengthMultMax(void) { return lengthMultMax; }

		/// Gets tempo of bone length change.
		inline float getTempo(void) { return tempo; }
		/// Returns current time of oscillator.
		inline float getTime(void) { return time; }

		/// Returns bone centre.
		Vector2D getCenter(void);

		Joint *j0; ///< one endpoint of bone
		Joint *j1; ///< the other endpoint of bone
		float damp; ///< stiffness
		bool selected; ///< set to true if the bone is selected

		/// Sets radius multiplier used when attaching vertices to bone.
		inline void setRadiusMult(float f) { attachRadiusMult = f; }
		/// Gets radius multiplier used when attaching vertices to bone.
		inline float getRadiusMult(void) { return attachRadiusMult; }
		/// Gets radius of vertex selection circle.
		inline float getRadius(void) { return dOrig*.5f*attachRadiusMult; }
		/// Sets attachment falloff.
		inline void setFalloff(float f) { falloff = f; }
		/// Gets attachment falloff.
		inline float getFalloff(void) { return falloff; }
		void recalculateWeights(void);
		float getViewRadius();

	private:
		float dOrig;	//< original length of bone

		char name[16]; //< name of bone

		/// list of attached vertices
		vector<Vertex *> *attachedVertices;

		float *dsts; ///< vertex distances from bone centre
		/** array of cosinus values of the angles that the segments
		 * from the vertices to the centre of the bone form
		 **/
		float *ca;
		float *sa; ///< sinus values
		float *weights; ///< interpolation weights when moving vertices

		/**
		 * radius multipler when vertices attached automatically to the bone
		 **/
		float attachRadiusMult;

		/**
		 * bone strength falloff for the movement of attached vertices,
		 * between 0.01 and 1, the bigger the value, the bigger the impact of
		 * the bone on the vertices proportional to the distance of the vertex
		 * from the centre of the bone
		 **/
		float falloff;

		/// bone length is multiplied by this value
		float lengthMult;
		float lengthMultMin; ///< the minimum bone length multiplier
		float lengthMultMax; ///< the maximum bone length multiplier

		float tempo; ///< the tempo of the oscillator
		float time; ///< the current state of the oscillator

		/// change bone length multiplier with time
		void animateLengthMult(float t);
};

} /* namespace Animata */

#endif

