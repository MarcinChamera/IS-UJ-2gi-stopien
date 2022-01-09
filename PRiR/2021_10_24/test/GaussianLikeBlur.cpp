/*
 * GaussianLikeBlur.cpp
 *
 *  Created on: 24 lis 2021
 *      Author: oramus
 */

#include "GaussianLikeBlur.h"
#include <math.h>

GaussianLikeBlur::GaussianLikeBlur( int dataPortionSize, double coef ) : MagicFuntion(dataPortionSize) {
	this->coef = coef;
	size = sqrt( dataPortionSize );
	size_half = size / 2;
}

double GaussianLikeBlur::calc( double *dataPortion ) {
	double result = 0.0;
	double exSum = 0.0;
	int r,c,dr,dc;
	double ex;

	for ( int i = 0; i < dataPortionSize; i++ ) {
		r = i / size;
		c = i - r * size;
		dr = r - size_half;
		dc = c - size_half;
		ex = exp( - coef * ( dr * dr + dc * dc ));
		exSum += ex;
		result += dataPortion[ i ] * ex;
	}
	calls++;

	return result / exSum;
}

