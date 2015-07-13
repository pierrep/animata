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

#ifndef __MATRIX_H__
#define __MATRIX_H__

namespace Animata
{

// class storing 4x4 float matrix
class Matrix
{
	public:

		float f[16];			///< items of the matrix in one dimension to adapt to the openGL convencion

		Matrix();
		Matrix(double e[16]);

		void clear();			///< zero matrix
		void loadIdentity();	///< indentity matrix

		inline float& operator [] (int i) { return f[i]; }

		Matrix& operator = (Matrix& m);		///< assignment operator
		Matrix& operator = (float e[16]);	///< float assignment operator
		Matrix& operator *= (Matrix& m);	///< right multiply operator

		Matrix& inverseRotation();						///< computes the inverse rotation from the 3x3 rotation portion of this matrix
		Matrix& inverse();								///< computes and returns the inverse of this matrix
		Matrix& translate(float x, float y, float z);	///< mutiplies the current matrix with the given translate matrix
		Matrix& scale(float x, float y, float z);		///< mutiplies the current matrix with the given scale matrix

		void print();
};

} /* namespace Animata */

#endif

