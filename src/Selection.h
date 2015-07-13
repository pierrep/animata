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

#ifndef __SELECTION_H__
#define __SELECTION_H__

#if defined(__APPLE__)
	#include <OPENGL/gl.h>
	#include <OPENGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include "animata.h"
#include "Camera.h"
#include "Layer.h"
#include "TextureManager.h"

using namespace std;

namespace Animata
{

/// Describes a picked primitive under the mouse cursor with its type and name.
struct SelectItem
{
	unsigned int type;	///< type of the selected primitive
	unsigned int name;	///< name of the selected primitive
};

/**
 * Uses various alternative OpenGL rendermodes to implement basic picking, selection and feedback mechanisms.
 * Has an own \a selected buffer to store which objects are found on a 2d coordinate or range during a pick or selection call.
 */
class Selection
{
		static const int radius = 5;		///< searching radius used during picking to find out what's under the mouse cursor

		static const int MAXHIT_INIT = 8;	///< default length of \a selected array

		SelectItem *selected;				///< list of the various primitives under mouse cursor
		unsigned selectedLength;			///< actual lenght of the \a selected array
		unsigned hitCount;					///< number of primitives under the mouse cursor after doPick()

		float *points;						///< holds vertices' position after calling doFeedback(), every vertex stores three value: name, x, y coordinate
		unsigned pointsLength;				///< actual length of the \a points array

		Layer *pickLayer;					///< layer where on the picking happens

		static const int BUFSIZE = 65536;	///< size of the OpenGL buffer during selection and feedback modes
		GLuint *selectBuffer;				///< internal selection buffer for OpenGL
		float *feedBuffer;					///< internal feedback buffer for OpenGL

		/// Processes selection buffer.
		void processHits(unsigned hits, GLuint buffer[]);
		/// Processes feedback buffer.
		int processFeedback(int size, float buffer[], float points[]);

	public:

		/**
		 * Primitves types that can be selected.
		 */
		enum {
			SELECT_VERTEX = 0,
			SELECT_JOINT,
			SELECT_TRIANGLE,
			SELECT_BONE,
			SELECT_TEXTURE
		};

		static const int OUT_OF_SCREEN = 0xABCD;	///< value for coordinates that are out of screen

		Selection();
		~Selection();

		void doPick(Camera *camera, Layer *layer, TextureManager *textureManager, int x, int y);

		void doSelect(ADrawable *node, unsigned type, float x, float y, float w, float h);
		void doCircleSelect(ADrawable *node, unsigned type, int x, int y, int r);

		void doFeedback(Layer *layer);

		/**
		 * Returns the \a selected array, which holds the primitives that are under the mouse cursor.
		 * Subsequent calls of doPick() actualizes the array.
		 * Get the size of the array by calling getHitCount().
		 * \retval SelectItem* A pointer to the first element of \a selected array.
		 */
		inline SelectItem* getSelected(void) { return selected; }
		/**
		 * Returns the number of primitives under the mouse cursor after calling doPick().
		 * \retval int The number of the picked primitives.
		 */
		inline int getHitCount(void) { return hitCount; }
		/**
		 * Virtually clears the \a selected array by setting the size of it to 0.
		 */
		inline void clearSelection(void) { hitCount = 0; }

		/**
		 * Returns the Layer which was the argument for the last doPick() call.
		 */
		inline Layer *getPickLayer() { return pickLayer; }
		/**
		 * Sets \a pickLayer to the given one.
		 * \param	l	The new pick layer.
		 */
		inline void setPickLayer(Layer *l) { pickLayer = l; }
		/**
		 * Invalidates pick layer, necessary when loading or starting a new file.
		 */
		inline void cancelPickLayer() { pickLayer = NULL; }
};

} /* namespace Animata */

#endif

