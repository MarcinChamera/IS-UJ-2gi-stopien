/*
 * SequentialDataProcessor.h
 *
 *  Created on: 20 paź 2021
 *      Author: oramus
 */

#ifndef MPIDATAPROCESSOR_H_
#define MPIDATAPROCESSOR_H_

#include "DataProcessor.h"

class MPIDataProcessor : public DataProcessor {
private:
	void createDataPortion( int row, int col, double *buffer );
protected:
	void singleExecution();
	void collectData();
	void shareData();
	void processZeroJob(int numberOfProcesses);
	void calculate();
	void sendData();
	void receiveData();
public:
	double** getResult() {
		return data;
	}
};

#endif /* SEQUENTIALDATAPROCESSOR_H_ */
