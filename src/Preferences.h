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

#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

namespace Animata
{

/**
 * Preferences set from the UI.
 **/
enum ANIMATA_PREFERENCES
{
	PREFS_JOINT_NAME,
	PREFS_JOINT_X,
	PREFS_JOINT_Y,
	PREFS_JOINT_FIXED,
	PREFS_JOINT_OSC,
	PREFS_LAYER_NAME,
	PREFS_LAYER_ALPHA,
	PREFS_LAYER_VISIBILITY
};

}

#endif

