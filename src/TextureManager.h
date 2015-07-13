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

#ifndef __TEXTUREMANAGER_H__
#define __TEXTUREMANAGER_H__

#include <vector>
#include "Texture.h"
#include "ImageBox.h"

using namespace std;

namespace Animata
{

/// Holds and manages textures.
class TextureManager
{
	private:

		vector<Texture*>* textures;	///< textures vector holding all the textures

		Texture *pTexture;					///< texture below the mouse cursor
		Texture *activeTexture;				///< texture attached to the mesh on currently active layer

		/// Adds an already allocated texture to the manager.
		void addTexture(Texture* t);

	public:

		TextureManager();
		~TextureManager();

		/// allocates a new texture if neccessary, and adds it to the TextureManager
		Texture *createTexture(ImageBox *box);

		/// remove a texture
		void removeTexture(Texture* t);

		/// get texture with given name
		Texture *getTexture(const char *name);

		void draw(int mode);

		/**
		 * Returns the \a textures vector holding all the textures uploaded to the manager.
		 * \retval	std::vector<Texture*>* Texture vector.
		 */
		inline vector<Texture*>* getTextures() { return textures; }
		/**
		 * Returns texture currently under the mouse cursor.
		 * \retval Texture* Texture below the mouse cursor.
		 */
		inline Texture *getPointedTexture() { return pTexture; }
		/**
		 * Returns texture attached to the mesh on the currently active layer.
		 * \retval Texture* Texture attached to the mesh on the active layer.
		 */
		inline Texture *getActiveTexture() { return activeTexture; }
};

} /* namespace Animata */

#endif

