/*
 * MPIDataProcessor.h
 *
 *  Created on: 12.12.2021
 *      Author: chamera
 */

#ifndef MPIDATAPROCESSOR_H_
#define MPIDATAPROCESSOR_H_

#include "DataProcessor.h"

class MPIDataProcessor : public DataProcessor {
private:
	double **arr;
	void createDataPortion( int row, int col, double *buffer , double **arr, int rank);
	double **tablePortionAlloc(int rows, int cols);
	int numOfProcesses;
protected:
	void singleExecution();
	void collectData() {}
	void shareData();
public:
	MPIDataProcessor();
	double** getResult() {
		return data;
	}
};

#endif /* MPIDATAPROCESSOR_H_ */
