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

#include "Playback.h"
#include "animataUI.h"

using namespace Animata;

/**
 * Creates a new playback window.
 * \param x \e x position of window.
 * \param y \e y position of window.
 * \param w Window width.
 * \param h Window height.
 * \param l Pointer to label string.
 */
Playback::Playback(int x, int y, int w, int h, const char* l) :
	Fl_Gl_Window(x, y, w, h, l)
{
	camera = new Camera();
	// rootLayer = NULL;
	allLayers = NULL;

	fullscreen = 0;
	this->resizable(this);

	glContext = NULL;
}

/**
 * Destroys the window and frees up memory allocated by \a camera.
 */
Playback::~Playback()
{
	delete camera;
}

/**
 * Draws the playback picture in the window.
 * Rendering is done in \a RENDER_PLAYBACK mode.
 */
void Playback::draw()
{
	if (!valid())
	{
		AnimataWindow::setupOpenGL();

		camera->setSize(w(), h());
	}

	// FIXME: somehow the context of the editorwindow has feedback on the
	// playbackwindow's context on windows
	// setting the viewport in the editorwindow causes to change it in the
	// playbackwindow too
	// camera->setupViewport();

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera->setupModelView();

	/*
	if (rootLayer)
		rootLayer->draw(RENDER_FEEDBACK | RENDER_OUTPUT | RENDER_TEXTURE | RENDER_WIREFRAME);
	*/
	if (allLayers)
	{
		vector<Layer *>::iterator l = allLayers->begin();
		for (; l < allLayers->end(); l++)
		{
			(*l)->drawWithoutRecursion(RENDER_FEEDBACK | RENDER_OUTPUT | RENDER_TEXTURE | RENDER_WIREFRAME);
		}
	}
}

/**
 * Event handler for playback window.
 * Used for switching between fullscreen and windowed mode.
 */
int Playback::handle(int event)
{
	switch(event)
	{
		case FL_PUSH:
			if(Fl::event_button() == FL_LEFT_MOUSE)
				return 1;
		case FL_KEYUP:
			if(Fl::event_key() == 32)
			{
				if(fullscreen)
				{
                    Fl_Window::hide();
					Fl_Window::fullscreen_off(ox, oy, ow, oh);
                    Fl_Window::show();
				}
				else
				{
					ox = this->x();
					oy = this->y();
					ow = this->w();
					oh = this->h();
                    Fl_Window::hide();
					Fl_Window::fullscreen();
					Fl_Window::show();
				}

				fullscreen = !fullscreen;
			}

			return 1;
			break;
		case FL_KEYDOWN:
			// do not quit if esc is pressed
			if(Fl::event_key() == FL_Escape) {
				return 1;
			}
			break;
		default:
			break;
	}

	return Fl_Gl_Window::handle(event);
}

/**
 * Overrides Fl_Gl_Window::show() so it wont create a new opengl context every time.
 * \todo This function is not implemented.
 */
void Playback::show()
{
	// context should be recreated, because on win the old one wont be used
	// TODO: reuse the main window's context
	context(NULL);

	Fl_Gl_Window::show();
}

/**
 * Overrides Fl_Gl_Window::hide() so the opengl context wont be destroyed.
 * If the context is destroyed, the editor context also stops on Mac OSX.
 */
void Playback::hide()
{
//	ui->editorBox->getCamera()->setPicture(NULL);

	Fl_Window::hide();
}

