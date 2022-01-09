/*
 * GaussianLikeBlur.h
 *
 *  Created on: 24 lis 2021
 *      Author: oramus
 */

#ifndef GAUSSIANLIKEBLUR_H_
#define GAUSSIANLIKEBLUR_H_

#include "MagicFuntion.h"

class GaussianLikeBlur: public MagicFuntion {
private:
	int size;
	int size_half;
	double coef;
public:
	GaussianLikeBlur(int dataPortionSize, double coef);
	double calc(double *dataPortion);
};

#endif /* GAUSSIANLIKEBLUR_H_ */
