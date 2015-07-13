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

#ifndef __LAYER_H__
#define __LAYER_H__

#include "Skeleton.h"
#include "Mesh.h"
#include "Matrix.h"

using namespace std;

namespace Animata
{

/**
 * Layer holding Skeleton and Mesh data
 **/
class Layer
{
	private:
		Layer		*parent;			///< pointer to the parent layer if its not the root
		std::vector<Layer *> *layers;	///< vector storing the sublayers

		Mesh		*mesh;				///< the mesh belonging to this layer
		Skeleton	*skeleton;			///< the skeleton belonging to this layer

		char name[16];					///< name of layer
		float x, y, z;					///< position of the layer
		float alpha;					///< layer alpha
		float scale;					///< layer scale
		bool visible;					///< visibility on/off

		/** Counts the created layers so far.
		 *  Used for automatic naming of layers */
		static int layerCount;

		/** transformation matrix returned by getTransformationMatrix() */
		Matrix transformation;

	public:

		Layer(Layer *p = NULL);
		~Layer();

		void eraseLayers();

		/** Returns sublayers.
		 * \return pointer to Layer * vector
		 **/
		inline std::vector<Layer *> *getLayers() { return layers; }

		/// overwrites sublayers
		void setLayers(std::vector<Layer *> *newLayers);

		void addSublayer(Layer *layers);

		void addSublayers(std::vector<Layer *> *layers);

		int deleteSublayer(Layer *layer);

		void drawWithoutRecursion(int mode);

		void simulate(int times = 1);

		/// makes a new layer
		Layer *makeLayer();

		/**
		 * calculates the transformation matrix based on the transformations of this layer
		 * also recalculates transformation matrices of the sublayers
		 **/
		void calcTransformationMatrix();

		/**
		 * Returns the transformation matrix of this layer.
		 * \return pointer to the transformation matrix which is held in a static
		 * storage place by each layer. Subsequent calls overwrite the content where
		 * the returned pointer points to.
		 **/
		inline Matrix *getTransformationMatrix() { return &transformation; }

		const char *getName(void);
		void setName(const char *str);

		/// Returns mesh.
		inline Mesh *getMesh() { return mesh; }
		/// Returns skeleton.
		inline Skeleton *getSkeleton() { return skeleton; }

		/// Returns parent of layer.
		inline Layer *getParent() { return parent; }
		/// Sets layer parent.
		inline void setParent(Layer *p) { parent = p; }

		/// Returns x position.
		inline float getX(void) const { return x; }
		/// Returns y position.
		inline float getY(void) const { return y; }
		/// Returns z position.
		inline float getZ(void) const { return z; }
		/// Returns cummulated z position.
		inline float getTotalDepth() const { return transformation.f[14]; }
		/// Returns scale.
		inline float getScale(void) const { return scale; }
		/// Returns alpha.
		inline float getAlpha(void) const { return alpha; }

		float getAccumulatedAlpha(void);

		/// Returns visibility.
		inline bool getVisibility() const { return visible; }

		/// Sets x position.
		inline void setX(float x) { this->x = x; /* calcTransformationMatrix(); */ }
		/// Sets y position.
		inline void setY(float y) { this->y = y; /* calcTransformationMatrix(); */ }
		/// Sets z position.
		inline void setZ(float z) { this->z = z; /* calcTransformationMatrix(); */ }
		/// Sets scale.
		inline void setScale(float scale) { this->scale = scale; calcTransformationMatrix(); }
		/// Sets alpha.
		inline void setAlpha(float alpha) { this->alpha = alpha; }

		void setVisibility(bool v);

		/**
		 * Moves layer.
		 * \param x x-distance to move by
		 * \param y y-distance to move by
		 **/
		inline void move(float x, float y) { this->x += x; this->y += y; calcTransformationMatrix(); }
		/**
		 * Resizes layer.
		 * \param s value added to scale
		 **/
		inline void resize(float s) { this->scale += s; calcTransformationMatrix(); }
		/**
		 * Changes layer depth.
		 * \param z value to add to the layer z-coordinate
		 **/
		inline void depth(float z) { this->z += z; calcTransformationMatrix(); }

		void scaleAroundPoint(float s, float ox, float oy);

		void setup(float x, float y, float z, float alpha, float scale);

		inline static bool zorder(const Layer *a, const Layer *b)
			{ return a->getTotalDepth() > b->getTotalDepth(); }
};

} /* namespace Animata */

#endif

