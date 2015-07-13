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

#include <math.h>
#include <float.h>
#include <unistd.h>
#include <algorithm>
#include <iterator>

#include "animata.h"
#include "animataUI.h"
#include "Transform.h"

AnimataUI *ui;

using namespace Animata;

namespace Animata
{
	Selection *selector;
}

AnimataSettings::AnimataSettings()
{
	mode = prevMode = ANIMATA_MODE_NONE;
	fps = 30;

	playSimulation = 1;
	iteration = 40;

	gravity = 0;
	gravityForce = 1;
	gravityX = 0;
	gravityY = 1;

	triangulateAlphaThreshold = 100;
}

/**
 * Creates the application window.
 * \param x x-position of window
 * \param y y-position of window
 * \param w window width
 * \param h window height
 * \param l pointer to label string
 **/
AnimataWindow::AnimataWindow(int x, int y, int w, int h, const char* l) :
	Fl_Gl_Window(x, y, w, h, l)
{
	camera = new Camera();

	selector = new Selection();
	textureManager = new TextureManager();

	rootLayer = NULL; // FIXME: this is replaced by the vector of root layers

	allLayers = NULL;
	allBones = NULL;
	allJoints = NULL;
	oscJoints = NULL;

	pthread_mutex_init(&mutex, NULL);

	io = new IO();

	oscListener = new OSCListener();
	oscSender = new OSCSender(OSC_HOST);

	bDoUpdateTextures = false;
}

/**
 * Starts up animatawindow.
 * this method has to be called right after instantiation
 **/
void AnimataWindow::startup(void)
{
	/* the calls here require a valid AnimataUI pointer, *ui, so they cannot
	 * be in the constructor */
	oscListener->start();
	oscSender->start();
	newScene();
}

AnimataWindow::~AnimataWindow()
{
	cleanup();

	pthread_mutex_destroy(&mutex);

	delete oscListener;
	delete oscSender;

	delete selector;

	delete textureManager;
	delete io;

	delete camera;
}

void AnimataWindow::cleanup(void)
{
	selector->cancelPickLayer();

	if (ui)
		ui->clearLayerTree();

	if (rootLayer)
	{
		delete rootLayer;
		rootLayer = NULL;
	}

	if (allLayers)
	{
		delete allLayers;
		allLayers = NULL;
	}

	if (allBones)
	{
		delete allBones;
		allBones = NULL;
	}

	if (allJoints)
	{
		delete allJoints;
		allJoints = NULL;
	}

	if (oscJoints)
	{
		delete oscJoints;
		oscJoints = NULL;
	}

	pointedVertex = pointedPrevVertex = pointedPrevPrevVertex = NULL;
	pointedFace = NULL;
	pointedJoint = pointedPrevJoint = NULL;
	pointedBone = NULL;
	selectedTexture = NULL;

	dragging = false;
	filename[0] = 0; // empty filename
}

void AnimataWindow::addToAllLayers(Layer *l)
{
	allLayers->push_back(l);
	sort(allLayers->begin(), allLayers->end(), Layer::zorder);
}

/**
 * Deletes layer from vector of all layers.
 * \param layer pointer to layer
 **/
void AnimataWindow::deleteFromAllLayers(Layer *layer)
{
	vector<Layer *>::iterator pos;

	// find position of layer in vector
	pos = std::find(allLayers->begin(), allLayers->end(), layer);
	if (pos == allLayers->end()) // not a member
	{
		fprintf(stderr, "error deleting %s (%x)\n", layer->getName(), layer);

		return;
	}

	allLayers->erase(pos);
}

/**
 * Deletes bone from vector of all bones.
 * \param bone pointer to bone
 **/
void AnimataWindow::deleteFromAllBones(Bone *bone)
{
	vector<Bone *>::iterator pos;

	// find position of bone in vector
	pos = std::find(allBones->begin(), allBones->end(), bone);
	if (pos == allBones->end()) // not a member
		return;

	allBones->erase(pos);
}

/**
 * Deletes joint from vector of all joints.
 * \param joint pointer to joint
 **/
void AnimataWindow::deleteFromAllJoints(Joint *joint)
{
	vector<Joint *>::iterator pos;

	// find position of joint in vector
	pos = std::find(allJoints->begin(), allJoints->end(), joint);
	if (pos == allJoints->end()) // not a member
		return;

	allJoints->erase(pos);
}

/**
 * Deletes joint from vector of OSC joints.
 * \param joint pointer to joint
 **/
void AnimataWindow::deleteFromOSCJoints(Joint *joint)
{
	vector<Joint *>::iterator pos;

	// find position of joint in vector
	pos = std::find(oscJoints->begin(), oscJoints->end(), joint);
	if (pos == oscJoints->end()) // not a member
		return;

	oscJoints->erase(pos);
}

void AnimataWindow::saveScene(const char *filename)
{
	io->save(filename, rootLayer);
}

void AnimataWindow::loadScene(const char *filename)
{
	cleanup();
	allLayers = new vector<Layer *>;
	allBones = new vector<Bone *>;
	allJoints = new vector<Joint *>;
	oscJoints = new vector<Joint *>;

	Layer *layer = io->load(filename);

	if (layer)
	{
		rootLayer = layer;

		setCurrentLayer(rootLayer);
		cMatrix = cLayer->getTransformationMatrix();

		oscListener->setRootLayer(rootLayer);

		sort(allLayers->begin(), allLayers->end(), Layer::zorder);
		if (ui)
		{
			//ui->playback->setRootLayer(rootLayer);
			ui->refreshLayerTree(rootLayer);
		}
	}
	else /* loading error */
	{
		delete allLayers;
		allLayers = NULL;
		delete allBones;
		allBones = NULL;
		delete allJoints;
		allJoints = NULL;
		delete oscJoints;
		oscJoints = NULL;
		newScene();
	}

}

/**
 * Imports scene to current layer.
 * \param filename filename to import from
 **/
void AnimataWindow::importScene(const char *filename)
{
	Layer *layer = io->load(filename);
	/* if no error add the layer to the current layer's children */
	if (layer)
	{
		cLayer->addSublayer(layer);

		#if 0
		// set parent of imported root level layers to the current layer
		vector<Layer *>::iterator l = layers->begin();
		for (; l < layers->end(); l++)
		{
			(*l)->setParent(cLayer);
			// FIXME: this is an ugly hack to add root level layers
			// because these layers have parent NULL and parent->makeLayer
			// does not work
			allLayers->push_back(*l);
		}
		#endif
	}

	if (ui)
	{
		//ui->playback->setRootLayer(rootLayer);
		ui->refreshLayerTree(rootLayer);
	}
}

void AnimataWindow::newScene(void)
{
	cleanup();

	allLayers = new vector<Layer *>;
	allBones = new vector<Bone *>;
	allJoints = new vector<Joint *>;
	oscJoints = new vector<Joint *>;

	rootLayer = new Layer();

	cLayer = rootLayer; //->makeLayer();
	cMesh = cLayer->getMesh();
	cSkeleton = cLayer->getSkeleton();
	cMatrix = cLayer->getTransformationMatrix();

	oscListener->setRootLayer(rootLayer);

	// TODO: erasing image boxes

	if (ui)
	{
		//ui->playback->setRootLayer(rootLayer);
		ui->refreshLayerTree(rootLayer);
	}
}

/// Refresh all textures on disk
void AnimataWindow::updateTextures(void)
{
    vector<Layer *>::iterator l = allLayers->begin();
	for (; l < allLayers->end(); l++)
	{
	    Mesh *tmpMesh;
	    tmpMesh = (*l)->getMesh();
	    if(tmpMesh) {
            if(tmpMesh->getAttachedTexture()) {
                float x, y, scale;
                string fullpath = tmpMesh->getAttachedTexture()->getFilename();
                //cout << "updating texture:" << fullpath << endl;

            	x = tmpMesh->getAttachedTexture()->x;
            	y = tmpMesh->getAttachedTexture()->y;
                scale = tmpMesh->getAttachedTexture()->getScale();

                ImageBox *box = ui->loadImage(fullpath.c_str(),true);

                // deallocate the previously attached texture
                textureManager->removeTexture(tmpMesh->getAttachedTexture());
                tmpMesh->setAttachedTexture(NULL);

                Texture *texture = textureManager->createTexture(box);

                // set texture parameters
                texture->x = x;
                texture->y = y;
                texture->setScale(scale);

                tmpMesh->setAttachedTexture(texture);
            }
	    }
	}



}

void AnimataWindow::flagUpdateTextures() {

    bDoUpdateTextures = true;
}

/// Sets filename of the scene.
void AnimataWindow::setFilename(const char *filename)
{
	strncpy(this->filename, filename, PATH_MAX);
}

/// Calls the mesh triangulation routine.
void AnimataWindow::triangulate(void)
{
	cMesh->triangulate();
	pointedFace = NULL;
}

/**
 * Attaches selected vertices to current bone.
 **/
void AnimataWindow::attachVertices(void)
{
	cSkeleton->attachVertices(cMesh->getSelectedVertices());
}

/**
 * Disattaches vertices from selected bones.
 **/
void AnimataWindow::disattachVertices(void)
{
	cSkeleton->disattachVertices();
}

/**
 * Initializes opengl parameters.
 **/
void AnimataWindow::setupOpenGL()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
}

/// Draws the scene
void AnimataWindow::drawScene(void)
{
    if(bDoUpdateTextures) {
        updateTextures();
        bDoUpdateTextures = false;
    }

	camera->setupPerspective();
	camera->setupModelView();

	textureManager->draw(RENDER_FEEDBACK | RENDER_TEXTURE);
	// rootLayer->draw(RENDER_FEEDBACK | RENDER_TEXTURE);
	// rootLayer->draw(RENDER_WIREFRAME);

	lock();
	vector<Layer *>::iterator l = allLayers->begin();
	for (; l < allLayers->end(); l++)
	{
		(*l)->calcTransformationMatrix();
	}
	unlock();

	l = allLayers->begin();
	for (; l < allLayers->end(); l++)
	{
		(*l)->drawWithoutRecursion(RENDER_FEEDBACK | RENDER_TEXTURE);
	}

	l = allLayers->begin();
	for (; l < allLayers->end(); l++)
	{
		(*l)->drawWithoutRecursion(RENDER_WIREFRAME);
	}
}

/**
 * Draws the application window. Handles mode changes and primitive selection.
 **/
void AnimataWindow::draw()
{
	if (ui->settings.mode != ui->settings.prevMode)
	{
		if (pointedJoint)
			pointedJoint->dragged = false;
		if (pointedPrevJoint)
			pointedPrevJoint->dragged = false;

		pointedVertex = pointedPrevVertex = pointedPrevPrevVertex = NULL;
		pointedFace = NULL;
		pointedJoint = pointedPrevJoint = NULL;
		pointedBone = NULL;
		selectedTexture = NULL;

		/* show related preferences tabs at mode change */
		switch (ui->settings.mode)
		{
			case ANIMATA_MODE_CREATE_JOINT:
				ui->skeletonPrefTabs->value(ui->jointPrefs);
				break;
			case ANIMATA_MODE_CREATE_BONE:
				ui->skeletonPrefTabs->value(ui->bonePrefs);
				break;
			case ANIMATA_MODE_ATTACH_VERTICES:
				ui->skeletonPrefTabs->value(ui->attachVertices);
				break;
			default:
				ui->skeletonPrefTabs->value(ui->noPrefs);
				break;
		}
		ui->settings.prevMode = ui->settings.mode;
	}

	if (!valid())
	{
		setupOpenGL();

		camera->setSize(w(), h());

		// recalculate picture size in playback window
		ui->playback->invalidate();
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* run the spring model simulation on all bones of the skeleton */
	if (ui->settings.playSimulation == 1)
		rootLayer->simulate(ui->settings.iteration);

	drawScene();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	// helper primitives will be drawn at screen coordinates in ortho mode as well
	camera->setupOrtho();

	switch (ui->settings.mode)
	{
		// show hint line for triangle creation
		case ANIMATA_MODE_CREATE_TRIANGLE:
			if (pointedVertex)
			{
				Primitives::drawFaceWhileConnecting(pointedVertex->view.x, pointedVertex->view.y,
										mouseX, h() - mouseY);
			}
			if (pointedVertex && pointedPrevVertex)
			{
				Primitives::drawFaceWhileConnecting(pointedVertex->view.x, pointedVertex->view.y,
										pointedPrevVertex->view.x, pointedPrevVertex->view.y);

			}
			break;
		// show hint line for bone creation
		case ANIMATA_MODE_CREATE_BONE:
			if (pointedJoint)
			{
				Primitives::drawBoneWhileConnecting(pointedJoint->vx, pointedJoint->vy,
										mouseX, h() - mouseY);
			}
			break;
		default:
			break;
	}

	// multiple box-selection
	if (dragging)
	{
		// for vertex
		if (!pointedVertex && !pointedFace &&
				(ui->settings.mode == ANIMATA_MODE_MESH_SELECT
					|| ui->settings.mode == ANIMATA_MODE_ATTACH_VERTICES))
		{
			/* TODO: store selected vertices prior to dragging and clear only
			 * currently selected vertices */
			if (!Fl::event_state(FL_SHIFT))
				cMesh->clearSelection();

			// drawSelectionBox(transDragMouse.x, transDragMouse.y, transMouse.x, transMouse.y);
			Primitives::drawSelectionBox(dragMouseX, h() - dragMouseY, mouseX, h() - mouseY);

			// use view coordinates as vertices are drawn on an ortho layer
			selector->doSelect(cMesh, Selection::SELECT_VERTEX,
					dragMouseX, dragMouseY, mouseX, mouseY);

			/* circle selection
			float r = sqrt((mouseX - dragMouseX)*(mouseX - dragMouseX) +
				(mouseY - dragMouseY)*(mouseY - dragMouseY));
			drawSelectionCircle(dragMouseX, dragMouseY, r);
			selector->doCircleSelect(cMesh, Selection::SELECT_VERTEX,
				dragMouseX, dragMouseY, r);
			*/
		}
		// for joint
		else if (!pointedJoint && !pointedBone &&
			ui->settings.mode == ANIMATA_MODE_SKELETON_SELECT)
		{
			/* TODO: store selected joints prior to dragging and clear only
			 * currently selected joints */
			if (!Fl::event_state(FL_SHIFT))
				cSkeleton->clearSelection();

			// drawSelectionBox(transDragMouse.x, transDragMouse.y, transMouse.x, transMouse.y);
			Primitives::drawSelectionBox(dragMouseX, h() - dragMouseY, mouseX, h() - mouseY);

			// use view coordinates as joints are drawn on an ortho layer
			selector->doSelect(cSkeleton, Selection::SELECT_JOINT,
				dragMouseX, dragMouseY, mouseX, mouseY);
		}
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// call selection
	selector->doPick(camera, cLayer, textureManager, mouseX, mouseY);
}

/**
 * Handles vertex selection.
 **/
void AnimataWindow::selectVertices(void)
{
	if (pointedVertex)
	{
		// if CTRL is pressed flip the selection of the current vertex
		if (Fl::event_state(FL_CTRL))
		{
			pointedVertex->flipSelection();
		}
		else
		// if there's a not selected vertex below the cursor select it,
		// and clear the selection if no SHIFT or CTRL is pressed
		if (!(pointedVertex->selected))
		{
			if (!Fl::event_state(FL_SHIFT | FL_CTRL))
				cMesh->clearSelection();
			pointedVertex->selected = true;
		}
	}
	else
	// clear selection if there is nothing below the cursor or SHIFT
	// or CTRL are not pressed
	if (!pointedFace & (!Fl::event_state(FL_SHIFT | FL_CTRL)))
	{
		cMesh->clearSelection();
	}
}

void AnimataWindow::setCurrentLayer(Layer *l)
{
	cLayer = l;

	cSkeleton = cLayer->getSkeleton();
	cMesh = cLayer->getMesh();
	cMatrix = cLayer->getTransformationMatrix();
}

void AnimataWindow::setSelectedLayers(Layer *l, int num)
{
	Layer *layer = l;
	selectedLayers.clear();

	for (int i = 0; i < num; i++, layer++)
	{
		selectedLayers.push_back(layer);
	}
}

/**
 * Creates and sets the attached texture for the mesh on the current layer.
 * A texture will be allocated, and added to the TexureManager.
 * If it's required, the previously attached texture will be deleted.
 * \param box the imagebox as the source of the texture
 **/
void AnimataWindow::createAttachedTexture(ImageBox *box)
{
	if(cMesh->getAttachedTexture())
	{
		// do not create a new texture if its already attached
		if(!strcmp(cMesh->getAttachedTexture()->getFilename(), box->getFilename()))
			return;

		// deallocate the previously attached texture
		textureManager->removeTexture(cMesh->getAttachedTexture());
		cMesh->setAttachedTexture(NULL);
	}

	Texture *texture = textureManager->createTexture(box);
	cMesh->attachTexture(texture);
}

Vector2D AnimataWindow::transformMouseToWorld(int x, int y)
{
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	camera->setupModelView();
	glMultMatrixf(cMatrix->f);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	camera->setupPerspective();

	Transform::setMatrices();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	Vector3D o = Transform::unproject(x, y, 0);
	Vector3D v = Transform::unproject(x, y, 1);

	Vector3D d(v.x - o.x, v.y - o.y, v.z - o.z);
	d.normalize();

	// a simplified equation can be used instead of the general plane-line intersection
	// as all planes are paralell to the screen
//	Vector3D n(0.f, 0.f, -1.f);
//	float x = -n.x * o.x - n.y * o.y - n.z * o.z;
//	float y = n.x * d.x + n.y * d.y + n.z * d.z;
//	float t = x/y;

	float t = -o.z / d.z;

	Vector3D p = d * t;
	p.y += o.y;
	p.x += o.x + 0.5f;	// FIXME: why + 0.5 ???
	p.z += o.z;

	return Vector2D(p.x, p.y);
}

/*
Vector2D AnimataWindow::transformMouseToWorld(int x, int y)
{
	float wx = (x - cMatrix->f[12]) * (1 + cMatrix->f[14] / camera->getDistance()) / cMatrix->f[0];
	float wy = (y - cMatrix->f[13]) * (1 + cMatrix->f[14] / camera->getDistance()) / cMatrix->f[5];

	return Vector2D(wx, wy);
}
*/

void AnimataWindow::handleLeftMousePress(void)
{
	prevMouseX = dragMouseX = mouseX = Fl::event_x();
	prevMouseY = dragMouseY = mouseY = Fl::event_y();

	// transform the mouse coordinates based on the transformation of the current layer
	Vector2D p = transformMouseToWorld(mouseX, mouseY);
	transDragMouse.x = transMouse.x = p.x;
	transDragMouse.y = transMouse.y = p.y;

	pointedPrevPrevVertex = pointedPrevVertex;
	pointedPrevVertex = pointedVertex;
	pointedVertex = cMesh->getPointedVertex();

	pointedFace = cMesh->getPointedFace();

	pointedPrevJoint = pointedJoint;
	pointedJoint = cSkeleton->getPointedJoint();

	pointedBone = cSkeleton->getPointedBone();

	// only allow selection of a texture if no vertex is selected
	if (!pointedVertex)
	{
		selectedTexture = textureManager->getPointedTexture();
	}

	switch (ui->settings.mode)
	{
		case ANIMATA_MODE_CREATE_VERTEX:
			/* do not create a vertex if there is one below the cursor */
			if(!pointedVertex)
			{
				cMesh->addVertex((float)transMouse.x,
								 (float)transMouse.y);
			}
			break;

		case ANIMATA_MODE_MESH_SELECT:
			selectVertices();
			break;

		case ANIMATA_MODE_MESH_DELETE:
			if (pointedVertex)
			{
				Vertex *v;
				cMesh->getSelectedVertex(&v);
				cSkeleton->disattachSelectedVertex(v);

				cMesh->deleteSelectedVertex();
				pointedVertex = NULL;
			}
			else
			if (pointedFace)
			{
				cMesh->deleteSelectedFace(pointedFace);
				pointedFace = NULL;
			}
			break;

		case ANIMATA_MODE_CREATE_JOINT:
			{
				/* do not create a joint if there is one below the cursor */
				if (!pointedJoint)
				{
					Joint *j = cSkeleton->addJoint((float)transMouse.x,
												   (float)transMouse.y);
					cSkeleton->clearSelection();
					j->selected = true;

					setJointUIPrefs(j);
				}
			}
			break;

		case ANIMATA_MODE_CREATE_BONE:
			if (pointedPrevJoint && pointedJoint)
			{
				Bone *b = cSkeleton->addBone(pointedPrevJoint, pointedJoint);
				if (b)
				{
					cSkeleton->clearSelection();
					b->selected = true;

					setBoneUIPrefs(b);
				}
				pointedPrevJoint = NULL;
				//pointedJoint = NULL;
			}
			break;

		case ANIMATA_MODE_SKELETON_SELECT:
			if (pointedJoint)
			{
				/* if CTRL is pressed flip the selection of the current
				 * joint */
				if (Fl::event_state(FL_CTRL))
				{
					pointedJoint->flipSelection();
				}
				else
				/* if there's a not selected joint below the cursor select it,
				 * and clear the selection if no SHIFT or CTRL is pressed */
				if (!(pointedJoint->selected))
				{
					if (!Fl::event_state(FL_SHIFT | FL_CTRL))
						cSkeleton->clearSelection();
					pointedJoint->selected = true;
				}
				setJointUIPrefs(pointedJoint);
			}
			else
			if (pointedBone)
			{
				if (Fl::event_state(FL_CTRL))
				{
					pointedBone->flipSelection();
				}
				else
				if (!(pointedBone->selected))
				{
					if (!Fl::event_state(FL_SHIFT | FL_CTRL))
						cSkeleton->clearSelection();
					pointedBone->selected = true;
				}
				setBoneUIPrefs(pointedBone);
			}
			else if (!pointedBone & (!Fl::event_state(FL_SHIFT | FL_CTRL)))
			{
				cSkeleton->clearSelection();
			}
			break;

		case ANIMATA_MODE_ATTACH_VERTICES:
			selectVertices(); /* allow the selection of vertices also */
			/* prefer the selection of vertices to bones, do not allow
			 * the selection of bones when there's a vertex below the
			 * cursor */
			if (pointedBone && !pointedVertex)
			{
				if (!(pointedBone->selected))
				{
					cSkeleton->clearSelection();
					pointedBone->selected = true;

					cMesh->clearSelection();
					pointedBone->selectAttachedVertices();
				}
				setAttachUIPrefs(pointedBone);
			}
			break;

		case ANIMATA_MODE_SKELETON_DELETE:
			if (pointedJoint)
			{
				cSkeleton->deleteSelectedJoint();
				pointedJoint = NULL;
			}
			else
			if (pointedBone)
			{
				cSkeleton->deleteSelectedBone();
				pointedBone = NULL;
			}
			break;

		case ANIMATA_MODE_CREATE_TRIANGLE:
			if (pointedPrevPrevVertex && pointedPrevVertex && pointedVertex)
			{
				cMesh->addFace(pointedPrevPrevVertex, pointedPrevVertex, pointedVertex);
				pointedPrevVertex = pointedPrevPrevVertex = NULL;
			}
			break;

		default:
			break;
	}
}

void AnimataWindow::handleRightMousePress()
{
	prevMouseX = dragMouseX = mouseX = Fl::event_x();
	prevMouseY = dragMouseY = mouseY = Fl::event_y();

	// transform the mouse coordinates based on the transformation of the current layer
	Vector2D p = transformMouseToWorld(mouseX, mouseY);
	transDragMouse.x = transMouse.x = p.x;
	transDragMouse.y = transMouse.y = p.y;
}

void AnimataWindow::handleLeftMouseRelease()
{
	if ((ui->settings.mode == ANIMATA_MODE_MESH_SELECT) && pointedVertex)
	{
		pointedVertex = NULL;
	}

	selectedTexture = NULL;

	switch (ui->settings.mode)
	{
		case ANIMATA_MODE_CREATE_VERTEX:
			break;
		case ANIMATA_MODE_CREATE_TRIANGLE:
			break;
		case ANIMATA_MODE_TEXTURIZE:
			if (Texture *t = textureManager->getPointedTexture())
				cMesh->attachTexture(t);
			break;
		case ANIMATA_MODE_MESH_SELECT:
			pointedFace = NULL;
			break;
		case ANIMATA_MODE_MESH_DELETE:
			break;
		case ANIMATA_MODE_CREATE_JOINT:
			break;
		case ANIMATA_MODE_CREATE_BONE:
			break;
		case ANIMATA_MODE_ATTACH_VERTICES:
			glColor3f(1, 0, 1);
			break;
		case ANIMATA_MODE_SKELETON_SELECT:
			if (pointedJoint)
			{
				/* if there was no dragging select the current joint and
				 * clear the selection if no SHIFT or CTRL is pressed */
				if (!dragging)
				{
					if (!Fl::event_state(FL_SHIFT | FL_CTRL))
						cSkeleton->clearSelection();
					/* select the current joint if CTRL is not pressed,
					 * because the selection has been flipped already on
					 * mouse button press */
					if (!Fl::event_state(FL_CTRL))
						pointedJoint->selected = true;
				}
				else
				{
					/* after dragging clear the drag attribute of joints */
					cSkeleton->endMoveSelectedJoints();
				}
				pointedJoint = NULL;
			}
			if (pointedBone)
			{
				if (!dragging)
				{
					if (!Fl::event_state(FL_SHIFT | FL_CTRL))
						cSkeleton->clearSelection();
					if (!Fl::event_state(FL_CTRL))
						pointedBone->selected = true;
				}
				else
				{
					cSkeleton->endMoveSelectedBones();
				}
				pointedBone = NULL;
			}
			break;
		case ANIMATA_MODE_SKELETON_DELETE:
			break;
		case ANIMATA_MODE_NONE:
			glColor3f(1, 1, 1);
			break;
		default:
			glColor3f(1, 0, 0);
			break;
	}

	dragging = false;
}

void AnimataWindow::handleRightMouseRelease()
{
	dragging = false;
}

void AnimataWindow::handleMouseMotion(void)
{
	mouseX = Fl::event_x();
	mouseY = Fl::event_y();

	// transform the mouse coordinates based on the transformation of the current layer
	Vector2D p = transformMouseToWorld(mouseX, mouseY);
	transMouse.x = p.x;
	transMouse.y = p.y;
}

void AnimataWindow::handleMouseDrag(void)
{
	mouseX = Fl::event_x();
	mouseY = Fl::event_y();

	dragging = true;
	Vector2D dragTrans = transformMouseToWorld(dragMouseX, dragMouseY);

	// transform the mouse coordinates based on the transformation of the current layer
	Vector2D p1 = transformMouseToWorld(mouseX, mouseY);
	Vector2D p2 = transformMouseToWorld(prevMouseX, prevMouseY);

	transMouse.x = p1.x;
	transMouse.y = p1.y;

	Vector2D viewDist(mouseX - prevMouseX, mouseY - prevMouseY);
	Vector2D worldDist(p1.x - p2.x, p1.y - p2.y);

	if (!Fl::event_button1())
	{
		Vector3D *target = camera->getTarget();
		Vector3D cameraTarget(target->x - worldDist.x, target->y - worldDist.y, target->z);
		camera->setTarget(&cameraTarget);
		ui->playback->getCamera()->setTarget(&cameraTarget);
	}
	else
	switch (ui->settings.mode)
	{
		case ANIMATA_MODE_MESH_SELECT:
			if (pointedVertex && pointedVertex->selected)
			{
				cMesh->moveSelectedVertices(worldDist.x, worldDist.y);
			}
			else if (pointedFace)
			{
				pointedFace->move(worldDist.x, worldDist.y);
			}
			break;

		case ANIMATA_MODE_SKELETON_SELECT:
			/* if the joint below the cursor is selected drag the selected
			 * joints */
			if (pointedJoint && pointedJoint->selected)
			{
				cSkeleton->moveSelectedJoints(worldDist.x, worldDist.y);
				setJointUIPrefs(pointedJoint);
			}
			else
			if (pointedBone && pointedBone->selected)
			{
				cSkeleton->moveSelectedBones(worldDist.x, worldDist.y);
				setBoneUIPrefs(pointedBone);
			}
			break;

		case ANIMATA_MODE_TEXTURE_POSITION:
			/*
			if (selectedTexture)
			{
				selectedTexture->x += viewDist.x;
				selectedTexture->y += viewDist.y;
			}
			break;
			*/
			// TEXTURE_POSITION is the same as LAYER_MOVE now
		case ANIMATA_MODE_LAYER_MOVE:
			// FIXME: scale shouldn't alter distance when moving layers
			cLayer->move(worldDist.x * cLayer->getScale(), worldDist.y * cLayer->getScale());
			break;

		case ANIMATA_MODE_TEXTURE_SCALE:
			/*
			if(selectedTexture)
			{
				// the scale center has to be transformed into the texture coordinate space
				selectedTexture->scaleAroundPoint(selectedTexture->getScale() + (float)(viewDist.y) / 100.f,
													(dragMouseX - selectedTexture->x) / selectedTexture->getScale(),
													(dragMouseY - selectedTexture->y) / selectedTexture->getScale());
			}
			break;
			*/
			// TEXTURE_SCALE is the same as LAYER_SCALE now
		case ANIMATA_MODE_LAYER_SCALE:
			// the scale center has to be transformed into the layer coordinate space
			/*
			cLayer->scaleAroundPoint(cLayer->getScale() + (float)(viewDist.y) / 100.f,
											(dragMouseX - cLayer->getX()) / cLayer->getScale(),
											(dragMouseY - cLayer->getY()) / cLayer->getScale());
			*/
			cLayer->scaleAroundPoint(cLayer->getScale() + (float)(viewDist.y) / 100.f, dragTrans.x, dragTrans.y);
			break;

		case ANIMATA_MODE_LAYER_DEPTH:
			cLayer->depth(viewDist.y);
			sort(allLayers->begin(), allLayers->end(), Layer::zorder);
			break;

		default:
			break;
	}

	prevMouseX = mouseX;
	prevMouseY = mouseY;
}

void AnimataWindow::handleMouseWheel(void)
{
	Vector3D target(camera->getTarget());

	target.z += Fl::event_dy();
	camera->setTarget(&target);
	ui->playback->getCamera()->setTarget(&target);
}

/**
 * Event handler.
 **/
int AnimataWindow::handle(int event)
{
	switch (event)
	{
		case FL_MOVE:
			handleMouseMotion();
			return 1;
		case FL_DRAG:
			handleMouseDrag();
			return 1;
		case FL_PUSH:
			if(Fl::event_button() == FL_LEFT_MOUSE)
			{
				handleLeftMousePress();
				return 1;
			}
			if(Fl::event_button() == FL_RIGHT_MOUSE)
			{
				handleRightMousePress();
				return 1;
			}
			break;
		case FL_RELEASE:
			if(Fl::event_button() == FL_LEFT_MOUSE)
			{
				handleLeftMouseRelease();
				return 1;
			}
			if(Fl::event_button() == FL_RIGHT_MOUSE)
			{
				handleRightMouseRelease();
				return 1;
			}
			break;
		case FL_MOUSEWHEEL:
			handleMouseWheel();
			return 1;
		case FL_FOCUS:
			return 1;
		case FL_UNFOCUS:
			return 1;
		case FL_KEYDOWN:
			// FIXME: if window looses its focus, it wont get any keyboard event
			// should the main parent window check for keyboard events?
			if(Fl::event_key() == 'u') {
                cout << "UPDATING TEXTURES" << endl;
                updateTextures();
			}
			if(Fl::event_key() == FL_Escape) {
                cout << "ESC key pressed" << endl;
				exit(0);
            }
				//return 1;
			break;
		default:
			break;
	}

	return Fl_Gl_Window::handle(event);
}

/**
 * Sets joint preferences on the user interface.
 * \param j pointer to the joint below the cursor
 **/
void AnimataWindow::setJointUIPrefs(Joint *j)
{
	ui->jointName->value(j->getName());
	ui->jointX->value(j->x);
	ui->jointY->value(j->y);
	ui->jointFixed->value(j->fixed);
	ui->jointOSC->value(j->osc);
	// TODO: check if tab switching is needed
	ui->skeletonPrefTabs->value(ui->jointPrefs);
}

/**
 * Sets joint parameters from the user interface.
 * \param prefParam parameter to set
 * \param value parameter value cast to (void *)
 **/
void AnimataWindow::setJointPrefsFromUI(enum ANIMATA_PREFERENCES prefParam, void *value)
{
	cSkeleton->setSelectedJointParameters(prefParam, value);
}

/**
 * Sets bone preferences on the user interface.
 * \param b pointer to bone below the cursor
 **/
void AnimataWindow::setBoneUIPrefs(Bone *b)
{
	ui->boneStiffness->value(b->damp);
	ui->boneName->value(b->getName());
	ui->boneLengthMult->value(b->getLengthMult());
	ui->boneLengthMultMin->value(b->getLengthMultMin());
	ui->boneLengthMultMax->value(b->getLengthMultMax());
	ui->boneTempo->value(b->getTempo());
	// TODO: check if tab switching is needed
	ui->skeletonPrefTabs->value(ui->bonePrefs);
}

/**
 * Sets bone parameters from the user interface.
 * \param name name of bone
 * \param stiffness bone stiffness
 * \param lengthMult bone length multiplier
 **/
void AnimataWindow::setBonePrefsFromUI(const char *name, float stiffness, float lengthMult)
{
	cSkeleton->setSelectedBoneParameters(name, stiffness, lengthMult);
}

/**
 * Sets bone length multiplier minimum for selected bone.
 * \param p multiplier value
 **/
void AnimataWindow::setBoneLengthMultMin(float p)
{
	cSkeleton->setSelectedBoneLengthMultMin(p);
}

/**
 * Sets bone length multiplier maximum for selected bone.
 * \param p multiplier value
 **/
void AnimataWindow::setBoneLengthMultMax(float p)
{
	cSkeleton->setSelectedBoneLengthMultMax(p);
}

/**
 * Sets bone tempo for selected bone.
 * \param p tempo value
 **/
void AnimataWindow::setBoneTempo(float p)
{
	cSkeleton->setSelectedBoneTempo(p);
}

/**
 * Sets attach preferences from the user interface.
 * \param area bone range
 * \param falloff bone strength falloff for attached vertices
 **/
void AnimataWindow::setAttachPrefsFromUI(float area /* = FLT_MAX*/, float falloff /* = FLT_MAX */)
{
	cSkeleton->setSelectedBoneParameters(NULL, FLT_EPSILON, -1, area, falloff);
	cSkeleton->selectVerticesInRange(cMesh);
}

/**
 * Sets attach preferences on the user interface.
 * \param b pointer to the bone below the cursor
 **/
void AnimataWindow::setAttachUIPrefs(Bone *b)
{
	ui->attachArea->value(b->getRadiusMult());
	cSkeleton->selectVerticesInRange(cMesh);
	ui->attachFalloff->value(b->getFalloff());
	// TODO: check if tab switching is needed
	ui->skeletonPrefTabs->value(ui->attachVertices);
}

/**
 * Sets layer preferences from the user interface.
 * \param prefParam parameter to set
 * \param value parameter value cast to (void *)
 **/
void AnimataWindow::setLayerPrefsFromUI(enum ANIMATA_PREFERENCES prefParam, void *value)
{
	// vector<Layer *>::iterator l = selectedLayers.begin();

	switch (prefParam)
	{
		case PREFS_LAYER_NAME:
			/*
			for (; l < selectedLayers.end(); l++)
				(*l)->setName(*((const char **)value));
			*/
			cLayer->setName(*((const char **)value));
			break;
		case PREFS_LAYER_ALPHA:
			/*
			for (; l < selectedLayers.end(); l++)
				(*l)->setAlpha(*((float *)value));
			*/
			cLayer->setAlpha(*((float *)value));
			break;
		case PREFS_LAYER_VISIBILITY:
			/*
			for (; l < selectedLayers.end(); l++)
				(*l)->setVisibility(*((int *)value));
			*/
			cLayer->setVisibility(*((int *)value));
			break;
		default:
			break;
	}
}

/**
 * Sets layer preferences on the user interface.
 * \param l pointer to layer
 **/
void AnimataWindow::setLayerUIPrefs(Layer *l)
{
	// how is it possible that ui is NULL sometimes???
	if (ui)
	{
		ui->layerName->value(l->getName());
		ui->layerAlpha->value(l->getAlpha());
		ui->layerVisible->value(l->getVisibility());;
	}
}

/**
 * Locks shared resources.
 **/
void AnimataWindow::lock(void)
{
	pthread_mutex_lock(&mutex);
}

/**
 * Unlocks shared resources.
 **/
void AnimataWindow::unlock(void)
{
	pthread_mutex_unlock(&mutex);
}

void timerCallback(void *v)
{
	ui->editorBox->lock();
	ui->editorBox->redraw();
	ui->playback->redraw();
	ui->editorBox->unlock();

	usleep(1);
	Fl::repeat_timeout(1.0/ui->settings.fps, timerCallback);
}

void loadFileAtStartup(void *filename)
{
	ui->editorBox->loadScene((char *)filename);
}

void init(void)
{
	Fl::add_timeout(1.0, timerCallback);
}

int main(int argc, char **argv)
{
	ui = new AnimataUI();
	ui->editorBox->startup();
	ui->show();
	/* make window cover the whole screen */
	//ui->resize(0, 0, Fl::w(), Fl::h());

	init();

	/* load file if there's a command line argument */
	if (argc > 1)
	{
		/* FIXME: loading file after fltk's mainloop has started, so opengl
		 * gets initialised, otherwise textures can't be uploaded */
		Fl::add_timeout(0.1, loadFileAtStartup, argv[1]);
	}

	Fl::run();

	delete ui;

	return 0;
}

