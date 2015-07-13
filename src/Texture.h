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

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#if defined(__APPLE__)
#include <OPENGL/gl.h>
#include <OPENGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "Vector2D.h"

namespace Animata
{

/// Represent a texture that can be attached to a Mesh.
class Texture
{
	private:
		unsigned char	*data;			///< pixel values of the texture
		int				depth;			///< color depth value

		float			scale;			///< scale multiplier for the size

		GLuint			glResource;		///< OpenGL resource of the texture

		int				sWrap;			///< \c GL_TEXTURE_WRAP_S OpenGL parameter
		int				tWrap;			///< \c GL_TEXTURE_WRAP_T OpenGL parameter

		int				minFilter;		///< \c GL_TEXTURE_MIN_FILTER OpenGL parameter
		int				magFilter;		///< \c GL_TEXTURE_MAG_FILTER OpenGL parameter

		const char		*filename;		///< filename from which the texture is created

		int getTexelAlpha(float x, float y);

	public:

		static const int BORDER = 0;	///< size of the border around the texture when mouse over

		float		x;					///< \e x coordinate of the position in world coordinate-system
		float		y;					///< \e y coordinate of the position in world coordinate-system

		int			width;				///< width of the texture
		int			height;				///< height of the texture

		Vector2D	viewTopLeft;
		Vector2D	viewBottomRight;


		Texture(const char *filename, int w, int h, int d, unsigned char* p, int reuseResource = 0);
		~Texture();

		void draw(int mouseOver = 0);

		int getTriangleAlpha(float x0, float y0, float x1, float y1,
				float x2, float y2, int maxIter = 3, int iterLevel = 1);

		void scaleAroundPoint(float s, float ox, float oy);

		/**
		 * Returns an array holding pixel values of the texture.
		 * \retval	unsigned char*	Array of the pixel values.
		 */
		inline unsigned char* getData() { return data; }

		/**
		 * Returns scale multiplier of the texture.
		 * \retval	float	Scale multiplier.
		 */
		inline float getScale(void) { return scale; }
		/**
		 * Sets scale multiplier value.
		 * \param	s	New scale multiplier.
		 */
		inline void setScale(float s) { scale = s; }

		/**
		 * Returns width of the texture.
		 * \retval	int	Width of the texture.
		 */
		inline int getWidth() { return width; }
		/**
		 * Returns height of the texture.
		 * \retval	int	Height of the texture.
		 */
		inline int getHeight() { return height; }
		/**
		 * Returns the OpenGL resource which holds the texture.
		 * \retval	GLuint	OpenGL resource that represent the texture.
		 */
		inline GLuint getGlResource() { return glResource; }

		/**
		 * Returns the filename from which the texture was created.
		 * \retval const char* String of the filename.
		 */
		inline const char *getFilename(void) { return filename; }

		Texture *clone();
};

} /* namespace Animata */

#endif

