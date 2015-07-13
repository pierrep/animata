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

#include "TextureManager.h"
#include "animata.h"
#include "animataUI.h"
#include "Transform.h"

using namespace Animata;

/**
 * Creates a new TextureManager object and allocates memory for holding textures in the \a textures vector.
 */
TextureManager::TextureManager()
{
	textures = new std::vector<Texture*>;

	pTexture = NULL;
}

/**
 * Removes every texture from the \a textures vector and deletes them also.
 */
TextureManager::~TextureManager()
{
	if (textures)
	{
		std::vector<Texture *>::iterator t = textures->begin();
		for (; t < textures->end(); t++)
			delete *t;					// free textures from memory
		textures->clear();				// clear all vector elements
		delete textures;
	}
}

/**
 * Returns the texture which was created from the given file.
 * \param filename Filename of texture.
 * \return Pointer to texture or NULL if does not exist.
 **/
Texture *TextureManager::getTexture(const char *filename)
{
	std::vector<Texture *>::iterator t = textures->begin();
	for (; t < textures->end(); t++)
	{
		if (strcmp((*t)->getFilename(), filename) == 0)
			return (*t);
	}
	return NULL;
}

/**
 * Adds an already allocated texture to the TextureManager.
 * \param t The texture to add.
 **/
void TextureManager::addTexture(Texture* t)
{
	textures->push_back(t);
}

/**
 * Removes the given texture if it presents in the TextureManager.
 * \param t The texture to remove.
 **/
void TextureManager::removeTexture(Texture* t)
{
	std::vector<Texture *>::iterator textureIter = textures->begin();
	for(; textureIter < textures->end(); textureIter++)
	{
		if(*textureIter == t)
		{
			// TODO: do not delete the texture if there is any other, which uses its gl resource
			delete *textureIter;
			textures->erase(textureIter);
		}
	}
}

/**
 * Allocates a new texture based on the given ImageBox if neccessary, and adds it to the TextureManager.
 * \todo If there is already a texture with the same image as the one in the ImageBox, use its glResource.
 * \param box the imagebox as the source for the texture
 * \return the newly allocated texture
 **/
Texture *TextureManager::createTexture(ImageBox *box)
{
/*
	Texture *sameImage = getTexture(box->getFilename());

	Texture *texture;

	if(sameImage != NULL)
	{
		texture = sameImage->clone();
	}
	else
	{
		texture = box->allocateTexture();
	}
*/

	Texture *texture = box->allocateTexture();
	addTexture(texture);

	return texture;
}

/**
 * Draws only the texture of the mesh on the currently active layer.
 * First the screen coordinates get computed by Transform::project(), then the texture gets draw in the resulting position.
 * \param	mode	Render mode. Screen coordinates gets computed only if mode is \c RENDER_FEEDBACK.
 */
void TextureManager::draw(int mode)
{
	unsigned hit = selector->getHitCount();
	SelectItem* selected = selector->getSelected();

	if((ui->settings.mode >= ANIMATA_MODE_TEXTURE_POSITION) &&
	    (ui->settings.mode <= ANIMATA_MODE_TEXTURE_SCALE))
	{

		// select the first texture from the selection buffer
		pTexture = NULL;
		for(unsigned int i = 0; i < hit; i++)
		{
			if(selected->type == Selection::SELECT_TEXTURE)
			{
				pTexture = (*textures)[selected->name];
			}

			selected++;
		}
	}

	// draw the textures only in image and mesh mode
	if(((ui->settings.mode >= ANIMATA_MODE_TEXTURE_POSITION) &&
	    (ui->settings.mode <= ANIMATA_MODE_TEXTURE_SCALE)) ||
	   ((ui->settings.mode >= ANIMATA_MODE_CREATE_VERTEX) &&
	    (ui->settings.mode <= ANIMATA_MODE_MESH_DELETE)))
	{
		glColor3f(1.f, 1.f, 1.f);
		glEnable(GL_TEXTURE_2D);

		glLoadName(Selection::SELECT_TEXTURE);
		glPushName(0);
		for(unsigned int i = 0; i < textures->size(); i++)
		{
			Texture* texture = (*textures)[i];

			// draw the texture only for the mesh on the current layer
			if (ui->editorBox->getMesh()->getAttachedTexture() == texture)
			{
				activeTexture = texture;
				glLoadName(i);

				if (mode & RENDER_FEEDBACK)
				{
					glMatrixMode(GL_MODELVIEW);
					glPushMatrix();
					glMultMatrixf(ui->editorBox->getCurrentLayer()->getTransformationMatrix()->f);

					Transform::setMatrices();

					glMatrixMode(GL_MODELVIEW);
					glPopMatrix();

					Vector3D view0 = Transform::project(texture->x, texture->y, 0);
					texture->viewTopLeft.set(view0.x, view0.y);
					Vector3D view1 = Transform::project(texture->x + texture->width, texture->y + texture->height, 0);
					texture->viewBottomRight.set(view1.x, view1.y);
				}

				if (mode & RENDER_TEXTURE)
				{
					// get the boundaries of actual viewport
					GLint viewport[4];
					glGetIntegerv(GL_VIEWPORT, viewport);

					glMatrixMode(GL_PROJECTION);
					glPushMatrix();
					glLoadIdentity();
					//glOrtho(0, viewport[2], 0, viewport[3], 0, 1);
					glOrtho(viewport[0], viewport[2] + viewport[0], viewport[1], viewport[3] + viewport[1], 0, 1);

					glMatrixMode(GL_MODELVIEW);
					glPushMatrix();
					glLoadIdentity();

					texture->draw(pTexture == texture);

					glMatrixMode(GL_MODELVIEW);
					glPopMatrix();

					glMatrixMode(GL_PROJECTION);
					glPopMatrix();
				}
			}
		}
		glPopName();

		glDisable(GL_TEXTURE_2D);
	}
}

