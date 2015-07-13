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
#include <math.h>
#include "Joint.h"
#include "Primitives.h"
#include "animata.h"
#include "animataUI.h"

using namespace Animata;

/**
 * Creates a joint at the (x, y) coordinate.
 **/
Joint::Joint(float x, float y)
{
	this->x = x;
	this->y = y;
	fixed = false;
	dragged = false;
	selected = false;
	dragTS = -1;
	osc = false;

	setName("");
}

/**
 * Returns the name of the joint.
 * \return pointer to name
 **/
const char *Joint::getName(void)
{
	return name;
}

/**
 * Sets the name of the joint.
 * \param str pointer to name string
 * \remark Only the first 15 characters will be used.
 **/
void Joint::setName(const char *str)
{
	strncpy(name, str, 15);
	name[15] = 0;
}

/**
 * Runs the spring simulation on the joint.
 **/
void Joint::simulate(void)
{
	if ((ui->settings.gravity == 1) & (!fixed) & (!dragged))
	{
		x += ui->settings.gravityForce * ui->settings.gravityX;
		y += ui->settings.gravityForce * ui->settings.gravityY;
	}
}

/**
 * Draws joint.
 * \param mouseOver 1 if the mouse is over the bone
 * \param active 1 if it is active
 **/
void Joint::draw(int mouseOver, int active)
{
	Primitives::drawJoint(this, mouseOver, active);
}

/**
 * Inverts the joint's state of selection.
 **/
void Joint::flipSelection(void)
{
	selected = !selected;
}

/**
 * Moves joint by the given vector.
 * \param dx x distance
 * \param dy y distance
 * \param timeStamp timestamp to prevent moving joints multiple times when
 *		they belong to several bones
 **/
void Joint::drag(float dx, float dy, int timeStamp /* = 0*/)
{
	if ((dragTS != timeStamp) || (timeStamp == 0))
	{
		x += dx;
		y += dy;
		dragged = true;
		dragTS = timeStamp;
	}
}

