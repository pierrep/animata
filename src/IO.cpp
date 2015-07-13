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

#include <libgen.h> // basename, dirname
#include <algorithm>
#include <iterator>

#include "animata.h"
#include "animataUI.h"
#include "IO.h"

using namespace Animata;

/* template to find the index of a given element in a vector
 * returns the index or -1 if not a member
 */
template <typename InputIterator, typename EqualityComparable>
int index(const InputIterator& begin, const InputIterator& end,
		const EqualityComparable& item)
{
	InputIterator r = find(begin, end, item);
	return (r == end) ? -1 : r - begin;
}

/**
 * Creates XML objects for all the bones in a skeleton.
 **/
void IO::saveBones(TiXmlElement *parent, vector<Bone *> *bones,
	vector<Joint *> *joints, vector<Vertex *> *vertices)
{
	TiXmlElement *bonesXML = new TiXmlElement("bones");
	parent->LinkEndChild(bonesXML);

	vector<Bone *>::iterator i = bones->begin();
	for (; i < bones->end(); i++)
	{
		Bone *b = *i;
		TiXmlElement *boneXML = new TiXmlElement("bone");
		const char *name = b->getName();
		if (name[0] != 0) // save name only for named bones
			boneXML->SetAttribute("name", name);
		// find vertex indices in mesh and set attributes
		boneXML->SetAttribute("j0",
			index(joints->begin(), joints->end(), b->j0));
		boneXML->SetAttribute("j1",
			index(joints->begin(), joints->end(), b->j1));
		boneXML->SetDoubleAttribute("stiffness", b->damp);
		boneXML->SetDoubleAttribute("lm", b->getLengthMult());
		boneXML->SetDoubleAttribute("lmmin", b->getLengthMultMin());
		boneXML->SetDoubleAttribute("lmmax", b->getLengthMultMax());
		boneXML->SetDoubleAttribute("tempo", b->getTempo());
		boneXML->SetDoubleAttribute("time", b->getTime());
		boneXML->SetDoubleAttribute("size", b->getOrigSize());
		boneXML->SetAttribute("selected", b->selected);
		boneXML->SetDoubleAttribute("radius", b->getRadiusMult());

		bonesXML->LinkEndChild(boneXML);

		// save attached vertices
		float *dsts, *weights, *ca, *sa;
		vector<Vertex *> *attachedVertices =
			b->getAttachedVertices(&dsts, &weights, &ca, &sa);
		int count = attachedVertices->size();
		if (!count) // no vertices attached
			continue;
		TiXmlElement *attachedXML = new TiXmlElement("attached");
		boneXML->LinkEndChild(attachedXML);
		for (int j = 0; j < count; j++)
		{
			Vertex *v = (*attachedVertices)[j];
			TiXmlElement *vertexXML = new TiXmlElement("vertex");
			// find and set vertex index
			vertexXML->SetAttribute("id",
			 index(vertices->begin(), vertices->end(), v));
			vertexXML->SetDoubleAttribute("d", dsts[j]);	// distance
			vertexXML->SetDoubleAttribute("w", weights[j]); // weight
			vertexXML->SetDoubleAttribute("ca", ca[j]); // cosinus
			vertexXML->SetDoubleAttribute("sa", sa[j]); // sinus
			attachedXML->LinkEndChild(vertexXML);
		}
	}
}

/**
 * Creates skeleton object in XML.
 **/
void IO::saveSkeleton(TiXmlElement *parent, Skeleton *s, Mesh *m)
{
	vector<Joint *> *joints = s->getJoints();
	vector<Bone *> *bones = s->getBones();

	if (joints->empty() && bones->empty())
		return;

	TiXmlElement *skeletonXML = new TiXmlElement("skeleton");
	parent->LinkEndChild(skeletonXML);

	if (!joints->empty())
	{
		TiXmlElement *jointsXML = new TiXmlElement("joints");
		skeletonXML->LinkEndChild(jointsXML);

		vector<Joint *>::iterator i = joints->begin();
		for(; i < joints->end(); i++)
		{
			Joint *j = *i;
			TiXmlElement *jointXML = new TiXmlElement("joint");
			const char *name = j->getName();
			if (name[0] != 0) // save name only for named joints
				jointXML->SetAttribute("name", name);
			jointXML->SetDoubleAttribute("x", j->x);
			jointXML->SetDoubleAttribute("y", j->y);
			jointXML->SetAttribute("fixed", j->fixed);
			jointXML->SetAttribute("selected", j->selected);
			jointXML->SetAttribute("osc", j->osc);

			jointsXML->LinkEndChild(jointXML);
		}
	}

	if (!bones->empty())
	{
		saveBones(skeletonXML, bones, joints, m->getVertices());
	}
}

/**
 * Creates XML objects for all the faces in a mesh.
 **/
void IO::saveFaces(TiXmlElement *parent, vector<Face *> *faces, vector<Vertex *> *vertices)
{
	vector<Face *>::iterator i = faces->begin();
	for (; i < faces->end(); i++)
	{
		Face *f = *i;
		TiXmlElement *face = new TiXmlElement("face");
		// find vertex indices in mesh and set attributes
		face->SetAttribute("v0",
			index(vertices->begin(), vertices->end(), f->v[0]));
		face->SetAttribute("v1",
			index(vertices->begin(), vertices->end(), f->v[1]));
		face->SetAttribute("v2",
			index(vertices->begin(), vertices->end(), f->v[2]));
		parent->LinkEndChild(face);
	}
}

/**
 * Creates a mesh object in the XML structure.
 **/
void IO::saveMesh(TiXmlElement *parent, Mesh *m)
{
	vector<Vertex *> *vertices = m->getVertices();
	vector<Face *> *faces = m->getFaces();

	if (vertices->empty() && faces->empty())
		return;

	TiXmlElement *meshXML = new TiXmlElement("mesh");
	parent->LinkEndChild(meshXML);

	if (!vertices->empty())
	{
		TiXmlElement *vertXML = new TiXmlElement("vertices");
		meshXML->LinkEndChild(vertXML);
		unsigned n = vertices->size();

		for (unsigned i = 0; i < n; i++)
		{
			Vertex *v = (*vertices)[i];

			TiXmlElement *vertex = new TiXmlElement("vertex");
			vertex->SetDoubleAttribute("x", v->coord.x);
			vertex->SetDoubleAttribute("y", v->coord.y);
			vertex->SetDoubleAttribute("u", v->texCoord.x);
			vertex->SetDoubleAttribute("v", v->texCoord.y);
			vertex->SetAttribute("selected", v->selected);

			vertXML->LinkEndChild(vertex);
		}
	}

	if (!faces->empty())
	{
		TiXmlElement *facesXML = new TiXmlElement("faces");
		meshXML->LinkEndChild(facesXML);

		saveFaces(facesXML, faces, vertices);
	}
}

/**
 * Creates the object of the layer's texture.
 **/
void IO::saveTexture(TiXmlElement *parent, Texture *t)
{
	if (t == NULL)
		return;

	TiXmlElement *texture = new TiXmlElement("texture");

	const char *filename = basename((char *)(t->getFilename()));
	if (filename)
		texture->SetAttribute("location", filename);

	texture->SetDoubleAttribute("x", t->x);
	texture->SetDoubleAttribute("y", t->y);
	texture->SetDoubleAttribute("scale", t->getScale());

	parent->LinkEndChild(texture);
}

/**
 * Creates the object of one layer in the XML structure.
 **/
void IO::saveLayer(TiXmlElement *parent, Layer *layer)
{
	TiXmlElement *layerXML = new TiXmlElement("layer");
	layerXML->SetAttribute("name", layer->getName());
	layerXML->SetDoubleAttribute("x", layer->getX());
	layerXML->SetDoubleAttribute("y", layer->getY());
	layerXML->SetDoubleAttribute("z", layer->getZ());
	layerXML->SetDoubleAttribute("alpha", layer->getAlpha());
	layerXML->SetDoubleAttribute("scale", layer->getScale());
	layerXML->SetAttribute("vis", layer->getVisibility());

	Mesh *m = layer->getMesh();
	Texture *t = m->getAttachedTexture();
	saveTexture(layerXML, t);
	saveMesh(layerXML, m);

	Skeleton *s = layer->getSkeleton();
	saveSkeleton(layerXML, s, m);

	parent->LinkEndChild(layerXML);

	vector<Layer *> *sublayers = layer->getLayers();
	if (!(sublayers->empty()))
	{
		saveLayers(layerXML, sublayers);
	}
}

/**
 * Creates the objects of layers in the XML structure.
 **/
void IO::saveLayers(TiXmlElement *parent, vector<Layer *> *layers)
{
	vector<Layer *>::iterator l = layers->begin();
	for (; l < layers->end(); l++)
	{
		saveLayer(parent, (*l));
	}
}

/**
 * Creates the settings object in the XML structure.
 **/
void IO::saveSettings(TiXmlElement *parent)
{
	TiXmlElement *settingsXML = new TiXmlElement("settings");

	parent->LinkEndChild(settingsXML);
}

/**
 * Saves scene in XML.
 * \param filename filename to save to
 * \param rootLayer pointer to the main vector of layers
 **/
void IO::save(const char *filename, Layer *rootLayer)
{
	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);

	TiXmlElement *animata = new TiXmlElement("animata");

	char versionstr[128];
	snprintf(versionstr, 128, "%d.%03d", ANIMATA_MAJOR_VERSION, ANIMATA_MINOR_VERSION);
	animata->SetAttribute("version", versionstr);
	doc.LinkEndChild(animata);

	saveLayer(animata, rootLayer);

	doc.SaveFile(filename);
}

/// Loads skeleton.
void IO::loadSkeleton(TiXmlNode *parent, Skeleton *skeleton, Mesh *m)
{
	// skeleton object is missing
	if (parent == NULL)
		return;

	// load joints
	TiXmlNode *jointsNode = parent->FirstChild("joints");
	if (jointsNode == NULL)
		return;
	TiXmlNode *jointNode = NULL;
	int jointCount = 0;
	int loadedJointCount = 0;
	while ((jointNode = jointsNode->IterateChildren(jointNode)))
	{
		TiXmlElement *j = jointNode->ToElement();

		const char *name;
		float x, y;
		int selected;
		int osc;
		int fixed;
		jointCount++;
		/* if there's a critical error the joint is skipped */
		QUERY_CRITICAL_ATTR(j, "x", x);
		QUERY_CRITICAL_ATTR(j, "y", y);
		/* loadedJointCount holds the number of actually loaded joints */
		loadedJointCount++;
		QUERY_ATTR(j, "selected", selected, 0);
		QUERY_ATTR(j, "osc", osc, 0);
		QUERY_ATTR(j, "fixed", fixed, 0);
		name = j->Attribute("name"); // can be NULL
		Joint *joint = skeleton->addJoint(x, y);
		joint->selected = selected;
		joint->osc = osc;
		joint->fixed = fixed;
		if (name)
			joint->setName(name);
		// add or remove the joint from the vector of joints
		// needed to be sent via OSC
		if (osc)
		{
		    cout << "add to OSC joints" << endl;
			ui->editorBox->addToOSCJoints(joint);
		}
	}
	// skip the loading of bones if there was a problematic joint
	if (jointCount != loadedJointCount)
		return;

	// load bones
	TiXmlNode *bonesNode = parent->FirstChild("bones");
	if (bonesNode == NULL)
		return;
	TiXmlNode *boneNode = NULL;
	vector<Joint*> *joints = skeleton->getJoints();
	while ((boneNode = bonesNode->IterateChildren(boneNode)))
	{
		TiXmlElement *b = boneNode->ToElement();

		const char *name;
		int j0, j1;
		float size, stiffness, lengthMult;
		float lengthMultMin, lengthMultMax, time, tempo;
		int selected;
		float radius;

		name = b->Attribute("name"); // can be NULL
		QUERY_CRITICAL_ATTR(b, "j0", j0);
		QUERY_CRITICAL_ATTR(b, "j1", j1);
		QUERY_CRITICAL_ATTR(b, "size", size);
		QUERY_ATTR(b, "stiffness", stiffness, BONE_DEFAULT_DAMP);
		QUERY_ATTR(b, "lm", lengthMult, BONE_DEFAULT_LENGTH_MULT);
		QUERY_ATTR(b, "lmmin", lengthMultMin, BONE_DEFAULT_LENGTH_MULT_MIN);
		QUERY_ATTR(b, "lmmax", lengthMultMax, BONE_DEFAULT_LENGTH_MULT_MAX);
		QUERY_ATTR(b, "tempo", tempo, 0);
		QUERY_ATTR(b, "time", time, 0);
		QUERY_ATTR(b, "selected", selected, 0);
		QUERY_ATTR(b, "radius", radius, 1);

		if ((j0 >= jointCount) || (j1 >= jointCount))
			continue;
		Joint *j0p = (*joints)[j0];
		Joint *j1p = (*joints)[j1];
		Bone *bone = skeleton->addBone(j0p, j1p);
		if (name)
			bone->setName(name);
		bone->setOrigSize(size);
		bone->damp = stiffness;
		bone->setLengthMult(lengthMult);
		bone->setLengthMultMin(lengthMultMin);
		bone->setLengthMultMax(lengthMultMax);
		bone->setTempo(tempo);
		bone->setTime(time);
		bone->selected = selected;
		bone->setRadiusMult(radius);

		// load attached vertices
		TiXmlNode *attachedNode = boneNode->FirstChild("attached");
		if (attachedNode == NULL)
			continue;
		TiXmlNode *vertexNode = NULL;
		// vector to hold vertices to be attached
		vector<Vertex *> *vertsToAttach = new vector<Vertex *>;
		// all vertices in mesh
		vector<Vertex *> *vertices = m->getVertices();
		// iterate over children to find all vertices
		while ((vertexNode = attachedNode->IterateChildren(vertexNode)))
		{
			TiXmlElement *vertexXML = vertexNode->ToElement();

			int id;
			QUERY_CRITICAL_ATTR(vertexXML, "id", id);

			if ((id >= (int)(vertices->size())) || (id < 0))
				continue;

			Vertex *v = (*vertices)[id];
			vertsToAttach->push_back(v);
		}

		// setup parameter arrays
		vertexNode = NULL;
		float *dsts, *weights, *ca, *sa;
		int count = vertsToAttach->size();
		dsts = new float[count];
		weights = new float[count];
		ca = new float[count];
		sa = new float[count];

		// load parameters of attached vertices
		int i = 0;
		while ((vertexNode = attachedNode->IterateChildren(vertexNode)))
		{
			TiXmlElement *vertexXML = vertexNode->ToElement();

			int id;
			float d, w;
			float s, c;
			QUERY_CRITICAL_ATTR(vertexXML, "id", id);
			QUERY_CRITICAL_ATTR(vertexXML, "d", d);
			QUERY_CRITICAL_ATTR(vertexXML, "w", w);
			QUERY_CRITICAL_ATTR(vertexXML, "sa", s);
			QUERY_CRITICAL_ATTR(vertexXML, "ca", c);

			if ((id >= (int)(vertices->size())) || (id < 0))
				continue;

			dsts[i] = d;
			weights[i] = w;
			ca[i] = c;
			sa[i] = s;
			i++;
		}
		bone->attachVertices(vertsToAttach, dsts, weights, ca, sa);
		vertsToAttach->clear();
		delete vertsToAttach;
	}
}

/// Loads mesh.
void IO::loadMesh(TiXmlNode *parent, Mesh *mesh)
{
	// xml element is missing
	if (parent == NULL)
		return;

	// load vertices
	TiXmlNode *verticesNode = parent->FirstChild("vertices");
	if (verticesNode == NULL)
		return;
	TiXmlNode *vertexNode = NULL;
	int vertexCount = 0;
	int loadedVertexCount = 0;
	while ((vertexNode = verticesNode->IterateChildren(vertexNode)))
	{
		TiXmlElement *vert = vertexNode->ToElement();

		float x, y, u, v;
		int selected;
		vertexCount++;
		/* if there's a critical error the vertex is skipped */
		QUERY_CRITICAL_ATTR(vert, "x", x);
		QUERY_CRITICAL_ATTR(vert, "y", y);
		/* loadedVertexCount is incremented only if all the necessary
		 * attributes are set */
		loadedVertexCount++;
		QUERY_ATTR(vert, "u", u, 0);
		QUERY_ATTR(vert, "v", v, 0);
		QUERY_ATTR(vert, "selected", selected, 0);

		Vertex *vertex = mesh->addVertex(x, y);
		vertex->texCoord.x = u;
		vertex->texCoord.y = v;
		vertex->selected = selected;
	}

	// skip the loading of faces if there was an error during vertex loading
	if (vertexCount != loadedVertexCount)
		return;

	// load faces
	TiXmlNode *facesNode = parent->FirstChild("faces");
	if (facesNode == NULL)
		return;
	TiXmlNode *faceNode = NULL;
	while ((faceNode = facesNode->IterateChildren(faceNode)))
	{
		TiXmlElement *f = faceNode->ToElement();

		int v0, v1, v2;
		QUERY_CRITICAL_ATTR(f, "v0", v0);
		QUERY_CRITICAL_ATTR(f, "v1", v1);
		QUERY_CRITICAL_ATTR(f, "v2", v2);

		vector<Vertex*> *vertices = mesh->getVertices();
		int vertexCount = vertices->size();
		if ((v0 >= vertexCount) || (v1 >= vertexCount) || (v2 >= vertexCount))
			continue;
		Vertex *v0p = (*vertices)[v0];
		Vertex *v1p = (*vertices)[v1];
		Vertex *v2p = (*vertices)[v2];
		mesh->addFace(v0p, v1p, v2p);
	}
}

/// Loads texture.
void IO::loadTexture(TiXmlNode *textureNode, Mesh *mesh)
{
	if (textureNode == NULL)
		return;

	TiXmlElement *t = textureNode->ToElement();
	const char *location;
	float x, y, scale;

	location = t->Attribute("location");
	if (location == NULL)
		return;
	char filepath[PATH_MAX+1], fullpath[PATH_MAX+1];
	// dirname may modify the content of filepath, so making a copy
	strncpy(filepath, this->filepath, PATH_MAX);
	const char *dir = dirname(filepath);
	snprintf(fullpath, PATH_MAX, "%s/%s", dir, location);
	//printf("loading texture %s, path %s, file %s\n", fullpath, filepath, location);

	QUERY_ATTR(t, "x", x, 0);
	QUERY_ATTR(t, "y", y, 0);
	QUERY_ATTR(t, "scale", scale, 1.0);

	// load image using fltk and add it to the texture manager
	ImageBox *box = ui->loadImage(fullpath);
	// TODO: error message box
	if (box == NULL)
	{
		fprintf(stderr, "error loading texture %s\n", fullpath);
		return;
	}

	// add the texture to the texturemanager
	Texture *texture = ui->editorBox->getTextureManager()->createTexture(box);

	// set texture parameters
	texture->x = x;
	texture->y = y;
	texture->setScale(scale);

	mesh->setAttachedTexture(texture);
}

/**
 * Loads one layer.
 * \param layerNode layer node in XML structure
 * \param layerParent pointer to the parent of current level layers
 * \return stores the layer structure in the layerParent's children
 **/
Layer *IO::loadLayer(TiXmlNode *layerNode, Layer *layerParent /* = NULL */)
{
	// load layer data
	TiXmlElement *l = layerNode->ToElement();

	const char *name;
	float x, y, z, scale, alpha;
	int vis;

	name = l->Attribute("name");
	if (name == NULL)
	{
		fprintf(stderr, "layer name is NULL %x\n", layerNode);
		return NULL;
	}

	QUERY_ATTR(l, "x", x, 0);
	QUERY_ATTR(l, "y", y, 0);
	QUERY_ATTR(l, "z", z, 0);
	QUERY_ATTR(l, "scale", scale, 1.0);
	QUERY_ATTR(l, "alpha", alpha, 1.0);
	QUERY_ATTR(l, "vis", vis, 1);

	Layer *layer;
	if (layerParent)
	{
		layer = layerParent->makeLayer();
	}
	else
	{
		/* layer is the root layer, it has no parent */
		layer = new Layer();
	}

	layer->setName(name);
	layer->setup(x, y, z, alpha, scale);
	layer->setVisibility(vis);

	// load mesh
	Mesh *mesh = layer->getMesh();
	TiXmlNode *meshNode = layerNode->FirstChild("mesh");
	loadMesh(meshNode, mesh);

	// load texture
	TiXmlNode *textureNode = layerNode->FirstChild("texture");
	loadTexture(textureNode, mesh);

	// load skeleton
	TiXmlNode *skeletonNode = layerNode->FirstChild("skeleton");
	loadSkeleton(skeletonNode, layer->getSkeleton(), layer->getMesh());

	// load sublayers
	if (layerNode->FirstChild("layer"))
	{
		loadLayers(layerNode, layer->getLayers(), layer);
	}

	return layer;
}

/**
 * Loads layers.
 * \param parent parent node in XML structure
 * \param layers vector of current level layers
 * \param layerParent pointer to the parent of current level layers
 * \return stores the layers structure in the layers vector
 **/
void IO::loadLayers(TiXmlNode *parent, vector<Layer *> *layers,
	Layer *layerParent)
{
	TiXmlNode *layerNode  = parent->FirstChild("layer");

	for ( ; layerNode; layerNode = layerNode->NextSibling())
	{
		loadLayer(layerNode, layerParent);
	}
}

/**
 * Loads XML scene.
 * \param filename filename to load the scene from
 * \return layers pointer,
 * the returned pointer must be freed by the caller
 **/
Layer *IO::load(const char *filename)
{
	filepath = filename;

	TiXmlDocument doc(filename);
	if (!doc.LoadFile())
		return NULL;

	TiXmlHandle docHandle(&doc);

	TiXmlNode *animataNode = docHandle.FirstChild("animata").ToNode();
	TiXmlNode *rootLayerNode  = animataNode->FirstChild("layer");
	Layer *rootLayer = loadLayer(rootLayerNode);

	return rootLayer;
}

