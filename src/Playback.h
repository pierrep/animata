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

#ifndef __PLAYBACK_H__
#define __PLAYBACK_H__

#if defined(__APPLE__)
	#include <OPENGL/gl.h>
	#include <OPENGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include "Camera.h"
#include "Layer.h"
#include "animata.h"

using namespace std;

namespace Animata
{

/// Playback window class.
/// Show the same sized image as the main application window,
/// but can display different primitive types as the actives in the main window.
class Playback : public Fl_Gl_Window
{
	private:

		Camera		*camera;				///< Camera which displays the playback window's picture. It's \a parent is set to AnimataWindow::camera.
		/*
		Layer		*rootLayer;				///< The root of all layers, same as AnimataWindow::rootLayer.
		*/
		std::vector<Layer *> *allLayers;	///< Every layer on the scene, same as AnimataWindow::allLayers.

		bool		fullscreen;				///< fullscreen flag
		int			ox, oy, ow, oh;			///< last position of the playback window before it has been but to fullscreen

		void		*glContext;				///< OpenGL context of this window

	public:

//		static const unsigned RENDER_PLAYBACK = RENDER_LAST;	///< Constant for drawing functions to render only the playback picture.

		Playback(int x, int y, int w, int h, const char* l = NULL);
		~Playback();

		void draw();
		int handle(int);

		void show();
		void hide();

		/**
		 * Returns the camera of the playback window.
		 * \retval	Camera*	A pointer to the camera object.
		 */
		inline Camera *getCamera() { return camera; }

		/**
		 * Sets the \a rootLayer to the given one.
		 * \param	r	The new rootLayer.
		 */
		/*
		inline void setRootLayer(Layer *r) { rootLayer = r; }
		*/
		inline void setAllLayers(std::vector<Layer *> *l) { allLayers = l; }
};

} /* namespace Animata */

#endif

