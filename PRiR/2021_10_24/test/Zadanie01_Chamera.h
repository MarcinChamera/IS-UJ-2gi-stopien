/*
 * MPIDataProcessor.h
 *
 *  Created on: 10.01.2021
 *      Author: chamera
 */

#ifndef MPIDATAPROCESSOR_H_
#define MPIDATAPROCESSOR_H_

#include "DataProcessor.h"

class MPIDataProcessor : public DataProcessor {
private:
	void createDataPortion( int row, int col, double *buffer);
	double **tablePortionAlloc(int rows, int cols);
	int numOfProcesses;
	int columnsFirstProcess;
	int columnsNotFirstProcess;
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