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
#include "animata.h"
#include "Primitives.h"
#include "Bone.h"
#include "Joint.h"

using namespace Animata;

float Primitives::fillColorR = 0.5f;
float Primitives::fillColorG = 0.5f;
float Primitives::fillColorB = 0.5f;
float Primitives::fillColorA = 0.7f;

float Primitives::strokeColorR = 1.0f;
float Primitives::strokeColorG = 1.0f;
float Primitives::strokeColorB = 1.0f;
float Primitives::strokeColorA = 1.0F;

bool Primitives::doStroke = true;
bool Primitives::doFill = false;
float Primitives::strokeW = 1;

float Primitives::boneSize = 5;
float Primitives::jointSize = 5;
float Primitives::vertexSize = 5;
float Primitives::border = 4;
int Primitives::dAlpha = 100;


///////////////////////////////    BONE     //////////////////////////////////

void Primitives::drawBone(Bone *b, int mouseOver, int active)
{
/*
	float x1 = b->j0->x;
	float y1 = b->j0->y;
	float x2 = b->j1->x;
	float y2 = b->j1->y;
*/

	float x1 = b->j0->vx;
	float y1 = b->j0->vy;
	float x2 = b->j1->vx;
	float y2 = b->j1->vy;

	float dx = (x2 - x1); // to prevent bone joint overlaping
	float dy = (y2 - y1);
	float dCurrent = sqrt(dx*dx + dy*dy);

	dx /= dCurrent;
	dy /= dCurrent;

	x1 += dx * boneSize;
	y1 += dy * boneSize;

	x2 -= dx * boneSize;
	y2 -= dy * boneSize; // move the endpoints towards the center boneSize

	float size = b->damp*3;

	int alpha = active ? 0 : dAlpha;

	stroke(true);
	strokeWeight(boneSize + border);
	if (mouseOver)
	{
		strokeColor(0, 255, 0, 128 - alpha);
	}
	else
	{
		strokeColor(0, 0, 0, 128 - alpha);
	}
	drawLine(x1, y1, x2, y2);
	strokeWeight(boneSize * size);
	if (mouseOver)
	{
		strokeColor(255, 255, 0, 128 - alpha);
	}
	else if (b->selected)
	{
		strokeColor(255, 255, 0, 200 - alpha);
	}
	else if (b->getTempo() > 0)
	{
		strokeColor(176, 0, 0, 200 - alpha);
	}
	else
	{
		strokeColor(255, 255, 255, 128 - alpha);
	}
	drawLine(x1, y1, x2, y2);
	if (!(b->selected))
	{
		strokeWeight(boneSize / 3 * size);
		strokeColor(255, 255, 255, 128 - alpha);
		drawLine(x1, y1, x2, y2);
	}
}

void Primitives::drawBoneWhileConnecting(float x1, float y1, float x2, float y2)
{
	stroke(true);
	strokeWeight((boneSize + border) );
	strokeColor(0, 0, 0, 128);
	drawLine(x1, y1, x2, y2);
	strokeWeight(boneSize);
	strokeColor(0, 255, 0, 128);
	drawLine(x1, y1, x2, y2);
}

///////////////////////////////    JOINT    //////////////////////////////////

void Primitives::drawJoint(Joint *j, int mouseOver, int active)
{
/*
	float x = j->x;
	float y = j->y;
*/

	float x = j->vx;
	float y = j->vy;

	int alpha = active ? 0 : dAlpha;

	stroke(false);
	fill(true);
	fillColor(0, 0, 0, 128 - alpha);
	drawCircle(x, y, jointSize + border);

	if (mouseOver)
	{
		stroke(true);
		fill(true);
		strokeColor(0, 255, 0, 200 - alpha);
		fillColor(0, 255, 0, 200 - alpha);
	}
	else if (j->selected)
	{
		stroke(true);
		fill(true);
		strokeColor(255, 255, 255, 200 - alpha);
		fillColor(255, 255, 0, 200 - alpha);
	}
	else if (j->fixed)
	{
		stroke(true);
		fill(true);
		strokeColor(255, 255, 255, 200 - alpha);
		fillColor(255, 0, 255, 200 - alpha);
	}
	else
	{
		stroke(true);
		fill(false);
		strokeColor(255, 255, 255, 200 - alpha);
	}
	strokeWeight(2);
	drawCircle(x, y, jointSize);
}

///////////////////////////////    VERTEX   //////////////////////////////////

void Primitives::drawVertex(Vertex *v, int mouseOver, int active)
{
/*
	float x = v->coord.x;
	float y = v->coord.y;
*/

	float x = v->view.x;
	float y = v->view.y;

	int alpha = active ? 0 : dAlpha;

	fill(true);
	stroke(false);
	fillColor(0,0,0,128 - alpha);
	drawRect(x, y, vertexSize + border);

	if (mouseOver)
	{
		stroke(true);
		fill(true);
		strokeColor(0, 255, 0, 200 - alpha);
		fillColor(0, 255, 0, 200 - alpha);
	}
	else if (v->selected)
	{
		stroke(true);
		fill(true);
		strokeColor(255, 255, 0, 200 - alpha);
		fillColor(255, 255, 0, 200 - alpha);
	}
	else
	{
		stroke(true);
		fill(false);
		strokeColor(255, 255, 255, 200 - alpha);
	}

	strokeWeight(1);
	drawRect(x, y, vertexSize);
}

void Primitives::drawVertexAttached(Vertex *v)
{
/*
	float x = v->coord.x;
	float y = v->coord.y;
*/

	float x = v->view.x;
	float y = v->view.y;

	fill(false);
	stroke(true);
	strokeColor(255,0,0,128);
	strokeWeight(1);
	drawRect(x, y, vertexSize + border*2);
}


///////////////////////////////    FACE     //////////////////////////////////

void Primitives::drawFace(Face *face, int mouseOver /* = 0 */, int active)
{
/*
	float x1 = face->v[0]->coord.x;
	float y1 = face->v[0]->coord.y;
	float x2 = face->v[1]->coord.x;
	float y2 = face->v[1]->coord.y;
	float x3 = face->v[2]->coord.x;
	float y3 = face->v[2]->coord.y;
*/
	float x1 = face->v[0]->view.x;
	float y1 = face->v[0]->view.y;
	float x2 = face->v[1]->view.x;
	float y2 = face->v[1]->view.y;
	float x3 = face->v[2]->view.x;
	float y3 = face->v[2]->view.y;

	int alpha = active ? 0 : dAlpha;

	fill(true);
	stroke(true);
	strokeWeight(1);
	if (mouseOver)
	{
		strokeColor(255, 255, 0, 200 - alpha);
		fillColor(255, 255, 0, 50);
	}
	else
	{
		strokeColor(255, 255, 255, 200 - alpha);
		fillColor(0, 0, 0, 42);
	}
	drawTriangle(x1, y1, x2, y2, x3, y3);
}

void Primitives::drawFaceWhileConnecting(float x1, float y1, float x2, float y2)
{
	stroke(true);
	strokeWeight(1);
	strokeColor(0, 255, 0, 200);
	drawLine(x1, y1, x2, y2);
}

void Primitives::fill(bool b)
{
	doFill = b;
}

void Primitives::stroke(bool b)
{
	doStroke = b;
}

void Primitives::strokeWeight(float w)
{
	strokeW=w;
}

void Primitives::drawRect(float x, float y, float size)
{
	float x1=x - size/2;
	float y1=y - size/2;
	float x2=x + size/2;
	float y2=y + size/2;
	if (doFill)
	{
		glColor4f(fillColorR, fillColorG, fillColorB, fillColorA);
		glRectf(x1,y1,x2,y2);
	}
	if (doStroke)
	{
		drawLine(x1, y1, x2, y1);
		drawLine(x2, y1, x2, y2);
		drawLine(x2, y2, x1, y2);
		drawLine(x1, y2, x1, y1);
	}
}

void Primitives::drawSelectionBox(float x1, float y1, float x2, float y2)
{
	glLineWidth(1);
	glLineStipple(1, 0xAAAA);
	glEnable(GL_LINE_STIPPLE);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glBegin(GL_LINE_LOOP);
		glVertex2f(x1,y1);
		glVertex2f(x2,y1);
		glVertex2f(x2,y2);
		glVertex2f(x1,y2);
	glEnd();
	glDisable(GL_LINE_STIPPLE);
}

void Primitives::drawSelectionCircle(float x, float y, float r)
{
	glLineWidth(1.5);
	glLineStipple(1, 0xAAAA);
	glEnable(GL_LINE_STIPPLE);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	int segments = (int)(2 * M_PI * r / 10);
	float inc = 2 * M_PI/segments;
	float angle = 0;
	glBegin(GL_LINE_LOOP);
		for(int i = 0; i < segments; i++)
		{
			glVertex2f(x + r*cos(angle), y + r*sin(angle));
			angle += inc;
		}
	glEnd();
	glDisable(GL_LINE_STIPPLE);
}

void Primitives::drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
	if (doFill)
	{
		glColor4f(fillColorR, fillColorG, fillColorB, fillColorA);
		glBegin(GL_TRIANGLES);
		glVertex2f(x1,y1);
		glVertex2f(x2,y2);
		glVertex2f(x3,y3);
		glEnd();
	}
	if (doStroke)
	{
		drawLine(x1, y1, x2, y2);
		drawLine(x2, y2, x3, y3);
		drawLine(x3, y3, x1, y1);
	}
}

void Primitives::drawLine(float x1, float y1, float x2, float y2)
{
	if (doStroke)
	{
		glLineWidth(strokeW);
		glColor4f(strokeColorR, strokeColorG, strokeColorB, strokeColorA);
		glBegin(GL_LINES);
		glVertex2f(x1,y1);
		glVertex2f(x2,y2);
		glEnd();
	}
}

void Primitives::drawCircle(float x, float y, float r)
{
	const int num = 8;
	static float ca[num+1], sa[num+1];
	static bool trigInited = false;

	if (!trigInited)
	{
		float inc = 2 * M_PI/num;
		float angle = 0;
		for (int i = 0; i <= num; i++)
		{
			ca[i] = cos(angle);
			sa[i] = sin(angle);
			angle += inc;
		}
		trigInited = true;
	}

	if (doFill)
	{
		glColor4f(fillColorR, fillColorG, fillColorB, fillColorA);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(x, y);
		for(int i = 0; i < num; i++)
		{
			glVertex2f(x + r*ca[i], y + r*sa[i]);
		}
		glVertex2f(x+r, y);
		glEnd();
	}
	if (doStroke)
	{
		for(int i = 0; i < num; i++)
		{
			Primitives::drawLine(x + r*ca[i], y + r*sa[i],x + r*ca[i+1], y + r*sa[i+1]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////

