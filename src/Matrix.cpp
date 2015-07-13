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

#include "Matrix.h"
#include "Vector3D.h"
#include <stdio.h>
#include <math.h>

using namespace Animata;

Matrix::Matrix()
{
	clear();
}

Matrix::Matrix(double e[16])
{
	for(int i = 0; i < 16; i++)
		f[i] = e[i];
}

/**
 * sets to zero every item of the matrix
 **/
void Matrix::clear()
{
	for(int i = 0; i < 16; i++)
		f[i] = 0.f;
}

/**
 * constructs an identity matrix
 **/
void Matrix::loadIdentity()
{
	clear();

	f[0] = f[5] = f[10] = f[15] = 1.f;
}

/**
 * Computes the inverse rotation portion of this matrix and returns it as a new object.
 * In case of OpenGL, the inverse of the rotation is the transpose of the rotation.
 **/
Matrix& Matrix::inverseRotation()
{
	Matrix *invrot = new Matrix();
	invrot = this;

	invrot->f[1] = f[4];
	invrot->f[4] = f[1];
	invrot->f[2] = f[8];
	invrot->f[8] = f[2];
	invrot->f[6] = f[9];
	invrot->f[9] = f[6];

	return *invrot;
}

/**
 * Compute the inverse only from the first three rows of this matrix and return as a new object.
 * If the matrix is not invertible, null is returned.
 **/
Matrix& Matrix::inverse()
{

    float A0 = f[0] * f[5] - f[4] * f[1];
    float A1 = f[0] * f[9] - f[8] * f[1];
    float A3 = f[4] * f[9] - f[8] * f[5];
    float det = A0 * f[10] - A1 * f[6] + A3 * f[2];

//    if (fabs(det) < 1e-12f)
//        return NULL; // matrix is not invertible

    float invDet = 1 / det;
    float A2 = f[0] * f[13] - f[12] * f[1];
    float A4 = f[4] * f[13] - f[12] * f[5];
    float A5 = f[8] * f[13] - f[12] * f[9];

    Matrix *inv = new Matrix();

    inv->f[0] = (+f[5] * f[10] - f[9] * f[6]) * invDet;
    inv->f[1] = (-f[1] * f[10] + f[9] * f[2]) * invDet;
    inv->f[2] = (+f[1] * f[6] - f[5] * f[2]) * invDet;
    inv->f[4] = (-f[4] * f[10] + f[8] * f[6]) * invDet;
    inv->f[5] = (+f[0] * f[10] - f[8] * f[2]) * invDet;
    inv->f[6] = (-f[0] * f[6] + f[4] * f[2]) * invDet;
    inv->f[8] = +A3 * invDet;
    inv->f[9] = -A1 * invDet;
    inv->f[10] = +A0 * invDet;
    inv->f[12] = (-f[6] * A5 + f[10] * A4 - f[14] * A3) * invDet;
    inv->f[13] = (+f[2] * A5 - f[10] * A2 + f[14] * A1) * invDet;
    inv->f[14] = (-f[2] * A4 + f[6] * A2 - f[14] * A0) * invDet;

    return *inv;

/*
	Matrix *inv = new Matrix();

	inv->f[0] = f[5] * f[10] - f[9] * f[6];
	inv->f[1] = f[9] * f[2] - f[1] * f[10];
	inv->f[2] = f[1] * f[6] - f[5] * f[2];

	float det = f[0] * inv->f[0] + f[1] * inv->f[1] + f[2] * inv->f[2];

//	if(fabs(det) < 1e-12f)
//		return NULL;

	float invdet = 1.f / det;

	inv->f[0] = inv->f[0] * invdet;
	inv->f[1] = inv->f[1] * invdet;
	inv->f[2] = inv->f[2] * invdet;
	inv->f[3] = 0.f;
*/
}

Matrix& Matrix::operator = (Matrix& m)
{
	for(int i = 0; i < 16; i++)
		f[i] = m[i];

	return *this;
}

Matrix& Matrix::operator = (float e[16])
{
	for(int i = 0; i < 16; i++)
		f[i] = e[i];

	return *this;
}

/**
 * right multiply operator
 * the current matrix gets multiplied with the given one from right
 * /param m the given matrix to which the current is multiplied
 **/
Matrix& Matrix::operator *= (Matrix& m)
{
	Matrix *temp = new Matrix();
	*temp = *this;

	this->clear();

	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 4; j++)
			for(int k = 0; k < 4; k++)
//				f[i*4 + j] += m[i*4 + k] * (*temp)[k*4 + j];
				f[i*4 + j] += (*temp)[i*4 + k] * m[k*4 + j];

	delete temp;
	return *this;
}

/**
 * constructs a translate matrix from the given translation vector
 * and multiplies it to the current matrix
 * /param x x coordinate of the translation
 * /param y y coordinate of the translation
 * /param z z coordinate of the translation
 **/
Matrix& Matrix::translate(float x, float y, float z)
{
	Matrix t;
	t.loadIdentity();

	t[12] = x;
	t[13] = y;
	t[14] = z;

	// left or right?
	*this *= t;
	return *this;
}

/**
 * constructs a scale matrix from the given scale vector
 * and multiplies it to the current matrix
 * /param x x coordinate of the scale
 * /param y y coordinate of the scale
 * /param z z coordinate of the scale
 **/
Matrix& Matrix::scale(float x, float y, float z)
{
	Matrix t;
	t.loadIdentity();

	t[0] = x;
	t[5] = y;
	t[10] = z;

	// left or right?
	*this *= t;
	return *this;
}

void Matrix::print()
{
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			printf("%f ", f[j*4 + i]);
		}
		printf("\n");
	}
	printf("\n");
}

