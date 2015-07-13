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

#ifndef __ANIMATA_H__
#define __ANIMATA_H__

#if defined(__APPLE__)

#include <OPENGL/gl.h>
#include <OPENGL/glu.h>

#else

#include <GL/gl.h>
#include <GL/glu.h>

#endif

#include <float.h>
#include <pthread.h>

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>

#include "Mesh.h"
#include "Skeleton.h"
#include "Selection.h"
#include "TextureManager.h"
#include "Primitives.h"
#include "Layer.h"
#include "IO.h"
#include "Camera.h"
#include "OSCManager.h"
#include "ImageBox.h"
#include "Preferences.h"

using namespace std;

#define ANIMATA_MAJOR_VERSION '0'
#define ANIMATA_MINOR_VERSION '004'
#define ANIMATA_VERSION ANIMATA_MAJOR_VERSION + '.' + ANIMATA_MINOR_VERSION


namespace Animata
{

/**
 * Operational modes set by pressing buttons on the GUI.
 **/
enum ANIMATA_MODES
{
	ANIMATA_MODE_NONE = 0,
	/* mesh */
	ANIMATA_MODE_CREATE_VERTEX,
	ANIMATA_MODE_CREATE_TRIANGLE,
	ANIMATA_MODE_TEXTURIZE,
	ANIMATA_MODE_MESH_SELECT,
	ANIMATA_MODE_MESH_DELETE,
	/* skeleton */
	ANIMATA_MODE_CREATE_JOINT,
	ANIMATA_MODE_CREATE_BONE,
	ANIMATA_MODE_ATTACH_VERTICES,
	ANIMATA_MODE_SKELETON_SELECT,
	ANIMATA_MODE_SKELETON_DELETE,
	/* texture */
	ANIMATA_MODE_TEXTURE_POSITION,
	ANIMATA_MODE_TEXTURE_SCALE,
	/* layer */
	ANIMATA_MODE_LAYER_MOVE,
	ANIMATA_MODE_LAYER_SCALE,
	ANIMATA_MODE_LAYER_DEPTH
};

enum ANIMATA_DISPLAY_ELEMENTS
{
	DISPLAY_EDITOR_VERTEX = 0x01,
	DISPLAY_EDITOR_TRIANGLE = 0x02,
	DISPLAY_EDITOR_JOINT = 0x04,
	DISPLAY_EDITOR_BONE = 0x08,
	DISPLAY_EDITOR_TEXTURE = 0x10,
	DISPLAY_OUTPUT_VERTEX = 0x10000,
	DISPLAY_OUTPUT_TRIANGLE = 0x20000,
	DISPLAY_OUTPUT_JOINT = 0x40000,
	DISPLAY_OUTPUT_BONE = 0x80000,
	DISPLAY_OUTPUT_TEXTURE = 0x100000
};

enum ANIMATA_RENDER_MODE
{
	RENDER_FEEDBACK = 0x01,
	RENDER_SELECTION = 0x02,
	RENDER_OUTPUT = 0x04,
	RENDER_TEXTURE = 0x10,
	RENDER_WIREFRAME = 0x20
};

/// Various settings coming from the GUI.
class AnimataSettings
{
	public:
		enum ANIMATA_MODES mode; /**< current operational mode */
		enum ANIMATA_MODES prevMode; /**< previous operational mode */

		int playSimulation; /**< whether to run simulation or not */
		int gravity; /**< use gravity force */
		float gravityForce; /**< strength of gravity */
		float gravityX; /**< x component of the gravity direction vector */
		float gravityY; /**< y component of the gravity direction vector */

		int iteration; /**< number of times to run the simulation */
		int fps; /**< frames per second */
		int display_elements; /**< flags to display elements in windows */

		int triangulateAlphaThreshold; /**< triangulation threshold */

		AnimataSettings();
};

/// Main application window class.
class AnimataWindow : public Fl_Gl_Window
{
	private:
		/** mouse coordinates */
		int mouseX, mouseY;
		/** previous mouse coordinates to measure mouse movement */
		int prevMouseX, prevMouseY;
		/** mouse coordinates where the dragging started */
		int dragMouseX, dragMouseY;
		bool dragging; /**< set to true while dragging the mouse */

		/** transformed mouse coordinates, based on current layers transformation */
		Vector2D	transMouse;
		/** transformed mouse coordinates where dragging sarted, based on current layers transformation */
		Vector2D	transDragMouse;

		Vertex			*pointedVertex;
		Vertex			*pointedPrevVertex;
		Vertex			*pointedPrevPrevVertex;

		Face			*pointedFace;

		Joint			*pointedJoint;
		Joint			*pointedPrevJoint;

		Bone			*pointedBone;

		Texture			*selectedTexture;

		TextureManager	*textureManager;
		Layer			*rootLayer; /**< the root of all the layers */

		vector<Layer *> selectedLayers;

		/* FIXME: use multimap instead of vectors and store only named elements */
		/* the following vectors are needed to reach the elements quickly
		 * without traversing the whole hierarcy recursively */
		/** vector of all layers without the hierarchical structure */
		vector<Layer *> *allLayers;
		/** vector of all bones without the hierarchical structure */
		vector<Bone *> *allBones;
		/** vector of all joints without the hierarchical structure */
		vector<Joint *> *allJoints;

		/** vector of all joints needed to be send via OSC */
		vector<Joint *> *oscJoints;

		Layer			*cLayer; /**< current layer */
		Mesh			*cMesh;	 /**< mesh of current layer */
		Skeleton		*cSkeleton; /**< skeleton of current layer */
		Matrix			*cMatrix; /**< transformation matrix for the current layer */

		IO				*io; /**< handles scene saving/loading */

		OSCListener		*oscListener; /**< handles osc messages */
		OSCSender		*oscSender; /**< transmits osc messages */

		Camera			*camera;

		pthread_mutex_t mutex;

		void handleLeftMousePress(void);
		void handleRightMousePress(void);
		void handleLeftMouseRelease(void);
		void handleRightMouseRelease(void);
		void handleMouseMotion(void);
		void handleMouseDrag(void);
		void handleMouseWheel(void);

		/// Erases scene and initialises values.
		void cleanup(void);

		void drawScene(void);

		void setJointUIPrefs(Joint *j);
		void setBoneUIPrefs(Bone *b);

		/// Handles vertex selection.
		void selectVertices(void);

		/// Transforms a screen coordinate to world coordinate.
		Vector2D transformMouseToWorld(int x, int y);

		char filename[PATH_MAX+1]; ///< filename of scene

		bool bDoUpdateTextures;

	public:
		AnimataWindow(int x, int y, int w, int h, const char* l);
		~AnimataWindow();

		void startup(void);

		/// Returns the filename of the scene.
		inline const char *getFilename(void) { return filename; }
		/// Sets the scene filename.
		void setFilename(const char *filename);

		/// Saves scene under the specified filename.
		void saveScene(const char *filename);
		/// Loads scene from the file specified by filename.
		void loadScene(const char *filename);
		/// Imports scene from the file specified by filename.
		void importScene(const char *filename);

		void newScene(void); ///< Starts a new scene.

        void updateTextures(void);
        void flagUpdateTextures(void);

		/// Initializes opengl parameters.
		static void setupOpenGL();

		void draw(void);
		void triangulate(void);

		void attachVertices(void);
		void disattachVertices(void);

		int handle(int);

		void setBonePrefsFromUI(const char *name = NULL, float stiffness = 0, float length = 1);
		void setJointPrefsFromUI(enum ANIMATA_PREFERENCES prefParam, void *value);
		void setAttachPrefsFromUI(float area = FLT_MAX, float falloff = FLT_MAX);
		void setAttachUIPrefs(Bone *b);

		void setLayerPrefsFromUI(enum ANIMATA_PREFERENCES prefType, void *value);

		void setLayerUIPrefs(Layer *l);

		void setBoneLengthMultMin(float p);
		void setBoneLengthMultMax(float p);
		void setBoneTempo(float p);

		/**
		 * Returns texture manager.
		 * \return pointer to texture manager
		 **/
		inline TextureManager* getTextureManager(void) { return textureManager; }
		/**
		 * Returns camera.
		 * \return pointer to camera
		 **/
		inline Camera *getCamera() { return camera; }
		/**
		 * Returns current mesh.
		 * \return pointer to mesh
		 **/
		inline Mesh *getMesh() { return cMesh; }
		/**
		 * Returns current skeleton.
		 * \return pointer to skeleton
		 **/
		inline Skeleton *getSkeleton() { return cSkeleton; }

		/** Gets current layer.
		 * \return pointer to current layer
		 **/
		inline Layer *getCurrentLayer() { return cLayer; }
		/** Sets current (active) layer.
		 * \param l layer to be the current
		 **/
		void setCurrentLayer(Layer *l);
		void setSelectedLayers(Layer *l, int num);

		void createAttachedTexture(ImageBox *box);

		/**
		 * Returns root layer
		 * \return pointer to root layer
		 **/
		inline Layer *getRootLayer() { return rootLayer; }

		/**
		 * Adds layer to vector of all layers.
		 * \param l layer pointer to add
		 **/
		void addToAllLayers(Layer *l);
		void deleteFromAllLayers(Layer *layer);
		/// Returns the vector storing all layers.
		inline vector<Layer *> *getAllLayers() { return allLayers; }

		/** Adds bone to vector of all bones.
		 * \param b bone pointer to add
		 **/
		inline void addToAllBones(Bone *b) { allBones->push_back(b); }
		void deleteFromAllBones(Bone *bone);
		/// Returns the vector storing all bones.
		inline vector<Bone *> *getAllBones() { return allBones; }

		/** Adds joint to vector of all joints.
		 * \param j joint pointer to add
		 **/
		inline void addToAllJoints(Joint *j) { allJoints->push_back(j); }
		/// Deletes joint from the vector of all joints.
		void deleteFromAllJoints(Joint *joint);
		/// Returns the vector storing all joints.
		inline vector<Joint *> *getAllJoints() { return allJoints; }

		/** Adds joint to vector of OSC joints.
		 * \param j joint pointer to add
		 **/
		inline void addToOSCJoints(Joint *j) { oscJoints->push_back(j); }
		/// Deletes joint from the vector of OSC joints.
		void deleteFromOSCJoints(Joint *joint);
		/// Returns the vector storing OSC joints.
		inline vector<Joint *> *getOSCJoints() { return oscJoints; }

		void lock(void);
		void unlock(void);
};

class Selection;
extern Selection* selector;

} /* namespace Animata */

class AnimataUI;
extern AnimataUI* ui;

#endif

/**
 * \mainpage
 * \section intro_sec Introduction
 *
 * Animata is a real-time animation software, designed to create interactive
 * background projections for concerts, theatre and dance performances, and
 * promotional screenings.
 *
 * The peculiarity of the software is that the animation - the movement of the
 * puppets, the changes of the background - is generated in real-time, making
 * continuous interaction possible. This ability also permits that physical
 * sensors, cameras or other environmental variables can be attached to the
 * animation of characters, creating a cartoon reacting to its environment. For
 * example, it is quite simple to create a virtual puppet band reacting to live
 * audio input, or set up a scene of drawn characters controlled by the movement
 * of dancers.
 *
 * In contrast with the traditional 3D animation programs, creating characters in
 * Animata is quite simple and takes only a few minutes. On the basis of the still
 * images, which serve as the skeleton of the puppets, we produce a network of
 * triangles, some parts of which we link with the bone structure. The  movement
 * of the bones is based on a physical model, which allows the characters to be
 * easily moved.
 *
 * The software can be run on multiple operation systems like Mac OS X, GNU Linux
 * and Windows. Animata can be connected with widespread programming environments
 * (e.g. Max/MSP, Pure Data, EyesWeb) used by multimedia developers and artists in
 * order to make use of the possibilities of these applications in the fields of
 * image editing, sound analysis, or motion capture.
 *
 *
 * \section install_sec Installing
 *
 * Animata requires:
 *
 * fltk (1.1.x)	http://www.fltk.org/
 *
 *
 * To build Animata, type:
 *
 * scons
 *
 * To run the software, type ./animata in the build directory.
 *
 *
 * \section info_sec Where to get more information
 *
 * There is a web page for Animata at
 *
 * http://animata.kibu.hu/
 *
 * This page also has information about mailing lists. Users should consider
 * subscribing to animata-users:
 *
 * http://lists.kitchenbudapest.hu/cgi-bin/mailman/listinfo/animata-users
 *
 * Animata-devel is for people interested in helping with the development:
 *
 * http://lists.kitchenbudapest.hu/cgi-bin/mailman/listinfo/animata-devel
 *
 *
 * \section problem_sec Reporting problems
 *
 * There are two mailing lists, animata-users@lists.kitchenbudapest.hu and
 * animata-devel@lists.kitchenbudapest.hu. Use the latter if you are prepared
 * for a more technical discussion with the developers of the package.
 *
 * Issues should be reported through the google code project page at
 * http://code.google.com/p/animata/issues/list
 *
 */

