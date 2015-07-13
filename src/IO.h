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

#ifndef __IO_H__
#define __IO_H__

#include <vector>
#include "tinyxml.h"
#include "Layer.h"

#define QUERY_CRITICAL_ATTR(t, name, outValue) \
		if (t->QueryValueAttribute(name, &outValue) != TIXML_SUCCESS) \
			continue;

#define QUERY_ATTR(t, name, outValue, defaultValue) \
		if (t->QueryValueAttribute(name, &outValue) != TIXML_SUCCESS) \
			outValue = defaultValue;

using namespace std;

namespace Animata
{

/// Class to save and load animata scenes.
class IO
{
	private:
		void saveLayer(TiXmlElement *parent, Layer *layer);
		void saveLayers(TiXmlElement *parent, vector<Layer *> *layers);
		void saveTexture(TiXmlElement *parent, Texture *t);
		void saveMesh(TiXmlElement *parent, Mesh *m);
		void saveFaces(TiXmlElement *parent, vector<Face *> *faces,
				vector<Vertex *> *vertices);
		void saveSkeleton(TiXmlElement *parent, Skeleton *s, Mesh *m);
		void saveBones(TiXmlElement *parent, vector<Bone *> *bones,
				vector<Joint *> *joints, vector<Vertex *> *vertices);
		void saveSettings(TiXmlElement *parent);

		Layer *loadLayer(TiXmlNode *layerNode, Layer *layerParent = NULL);
		void loadLayers(TiXmlNode *parent, vector<Layer *> *layers,
					Layer *layerParent);
		void loadMesh(TiXmlNode *parent, Mesh *mesh);
		void loadTexture(TiXmlNode *textureNode, Mesh *mesh);
		void loadSkeleton(TiXmlNode *parent, Skeleton *skeleton, Mesh *m);

		const char *filepath; ///< absolute filename to load from

	public:
		/// Saves scene specified by its root layer using the given filename.
		void save(const char *filename, Layer *rootLayer);

		/// Loads scene.
		Layer *load(const char *filename);
};

} /* namespace Animata */

#endif

