#include <math.h> 
#include <iostream>
#include <iomanip>
#include <limits>
#include <fstream>
#include "mpi.h" 
#include "DataProcessor.h"
#include "MagicFuntion.h"
#include "ArithmeticMeanFunction.h"
#include "SimpleInitialDataGenerator.h"
#include "SequentialDataProcessor.h"
#include "Alloc.h"
#include "GaussianLikeBlur.h"
#include "SimulationData.h"
#include "MPIDataProcessor.h"

using namespace std;

int calcDataPortion(int margin) {
	int dataPortion = margin * 2 + 1;
	dataPortion *= dataPortion;
	return dataPortion;
}

void showTableIfSizeSmallerThan30(double **table, int dataSize) {
	if (dataSize > 29)
		return;
	cout << "----------------------------------" << endl;
	for (int i = 0; i < dataSize; i++) {
		cout << setw(3) << i << " -> ";
		for (int j = 0; j < dataSize; j++)
			cout << " " << showpoint << setw(4) << setprecision(3)
					<< table[i][j];
		cout << endl;
	}
	cout << "----------------------------------" << endl;
}

void showBasicSimulationInfo(SimulationData *simInfo) {
	cout << "------------------------------" << endl;
	cout << "data size          " << simInfo->dataSize << endl;
	cout << "margin             " << simInfo->margin << endl;
	cout << "repetitions        " << simInfo->repetitions << endl;
	cout << "wallTime           " << simInfo->wallTime << endl;
	cout << "magicFunctionCalls " << simInfo->magicFunctionCalls << endl;
	cout << "------------------------------" << endl;
}

void simInfoComparator() {
}

void getNumberOfMagicFunctionCalls(long *callsPerProcess, int size,
		MagicFuntion *mf) {
	long toSend = mf->callsNumber();
	cout << "toSend : " << toSend << endl;
	// https://mpitutorial.com/tutorials/mpi-scatter-gather-and-allgather/
	MPI_Gather(&toSend, 1, MPI_LONG, callsPerProcess, 1, MPI_LONG, 0,
	MPI_COMM_WORLD);
}

void showNumberOfMagicFunctionCalls(long *callsPerProcess, int size) {
	for (int i = 0; i < size; i++) {
		cout << "Process " << i << " MagicFuntion calls " << callsPerProcess[i]
				<< endl;
	}
}

long countTotalNumberOfMagicFunctionCalls(long *callsPerProcess, int size) {
	long result = 0;
	for (int i = 0; i < size; i++) {
		result += callsPerProcess[i];
	}
	return result;
}

double** createInitialData(InitialDataGenerator *generator,
		SimulationData *simInfo) {
	double **initialData = tableAlloc(simInfo->dataSize);
	generator->fillWithData(initialData, simInfo->dataSize, simInfo->margin);
	return initialData;
}

void executeSimulationAndSaveResult(SimulationData *simInfo, MagicFuntion *mf,
		double **initialData) {

	showTableIfSizeSmallerThan30(initialData, simInfo->dataSize);

	DataProcessor *dp = new SequentialDataProcessor();
	dp->setMagicFunction(mf);
	dp->setInitialData(initialData, simInfo->dataSize);

	double workTime = MPI_Wtime();
	dp->execute(simInfo->repetitions);
	workTime = MPI_Wtime() - workTime;

	long callsPerProcess;
	getNumberOfMagicFunctionCalls(&callsPerProcess, 1, mf);

	double **result = dp->getResult();
	showTableIfSizeSmallerThan30(result, simInfo->dataSize);
	simInfo->result = result;
	simInfo->wallTime = workTime;
	simInfo->magicFunctionCalls = callsPerProcess;
	showBasicSimulationInfo(simInfo);
	saveSimulationDataInFile(simInfo);
}

int resultsTheSame(SimulationData *simInfo, SimulationData *expectedData) {
	cout << "Expected size " << expectedData->dataSize << endl;
	for (int r = 0; r < expectedData->dataSize; r++)
		for (int c = 0; c < expectedData->dataSize; c++) {
			if (fabs(simInfo->result[r][c] - expectedData->result[r][c])
					> 0.001) {
				cerr << "Result are different. " << endl;
				cerr << "Row      " << r << endl;
				cerr << "Col      " << c << endl;
				cerr << "Expected " << expectedData->result[r][c] << endl;
				cerr << "Accual   " << simInfo->result[r][c] << endl;
				return 0;
			}
		}
	cout << "Result OK!" << endl;
	return 1;
}

int efficiencyAcceptable(SimulationData *simInfo, SimulationData *expectedData,
		int size) {
	double speedup = expectedData->wallTime / simInfo->wallTime;
	double efficiency = 100.0 * speedup / size;

	cout << "Speedup    " << speedup << endl;
	cout << "Efficiency " << efficiency << endl;

	if (efficiency > 60.0) {
		cout << "Efficiency is acceptable!!!" << endl;
		return 1;
	} else {
		cerr << "Low efficiency" << endl;
		return 0;
	}
}

int executeParallelSimulationAndTestResult(SimulationData *simInfo,
		SimulationData *expectedData, MagicFuntion *mf, DataProcessor *dp,
		double **initialData, int size, int rank) {
	long *callsPerProcess;
	double workTime;

	dp->setMagicFunction(mf);

	if (rank == 0) {
		callsPerProcess = new long[size];

		showTableIfSizeSmallerThan30(initialData, simInfo->dataSize);

		dp->setInitialData(initialData, simInfo->dataSize);
		workTime = MPI_Wtime();
	}

	dp->execute(simInfo->repetitions);

	if (rank == 0) {
		workTime = MPI_Wtime() - workTime;
	}

	getNumberOfMagicFunctionCalls(callsPerProcess, size, mf);

	int testOK;
	if (rank == 0) {
		showNumberOfMagicFunctionCalls(callsPerProcess, size);
		double **result = dp->getResult();
		showTableIfSizeSmallerThan30(result, simInfo->dataSize);
		simInfo->result = result;
		simInfo->wallTime = workTime;
		simInfo->magicFunctionCalls = countTotalNumberOfMagicFunctionCalls(
				callsPerProcess, size);

		cout << endl << "Parallel simulation info" << endl;
		showBasicSimulationInfo(simInfo);
		cout << endl << "resultsTheSame?" << endl;

		testOK = resultsTheSame(simInfo, expectedData);
		if (testOK)
			if (simInfo->dataSize > 1000) {
				testOK = efficiencyAcceptable(simInfo, expectedData, size);
			}
	}

	MPI_Bcast(&testOK, 1, MPI_INT, 0, MPI_COMM_WORLD);

	return testOK;
}

void serialSimulation(SimulationData *simData) {

	int dataPortion = calcDataPortion(simData->margin);
	MagicFuntion *mf = new GaussianLikeBlur(dataPortion, 0.1);
	InitialDataGenerator *generator = new SimpleInitialDataGenerator(1, 10);
	double **initialData = createInitialData(generator, simData);

	executeSimulationAndSaveResult(simData, mf, initialData);
}

int parallelSimulation(SimulationData *simData, SimulationData *expectedResult,
		int size, int rank) {
	int dataPortion = calcDataPortion(simData->margin);
	MagicFuntion *mf = new GaussianLikeBlur(dataPortion, 0.1);
	InitialDataGenerator *generator = new SimpleInitialDataGenerator(1, 10);
	double **initialData = createInitialData(generator, simData);
	DataProcessor *dp = new MPIDataProcessor();

	return executeParallelSimulationAndTestResult(simData, expectedResult, mf,
			dp, initialData, size, rank);
}

SimulationData* loadSimulationData(string fileName) {
	SimulationData *tmp = loadDataFromFile(fileName);
	if (tmp->dataSize < 0) {
		cerr << endl << "File " << fileName << " not found" << endl;
	}
	showBasicSimulationInfo(tmp);
	return tmp;
}

int main(int argc, char *argv[]) {

	MPI_Init(&argc, &argv);
	int clusterSize;
	MPI_Comm_size(MPI_COMM_WORLD, &clusterSize);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if (clusterSize == 1) {
		cout << "Serial simulation - short" << endl;
		serialSimulation(getShortSimulationData());
		cout << "Serial simulation - long" << endl;
		serialSimulation(getLongSimulationData());
		MPI_Finalize();
		return 0;
	}

	SimulationData *simShortData = getShortSimulationData();
	SimulationData *simLongData = getLongSimulationData();
	SimulationData *simShortExpectedResult;
	SimulationData *simLongExpectedResult;

	int canContinue;
	if (rank == 0) {
		cout << endl << "Start. Processes #" << clusterSize << endl;
		simShortExpectedResult = loadSimulationData(simShortData->fileName);
		simLongExpectedResult = loadSimulationData(simLongData->fileName);
		if ((simShortExpectedResult->dataSize < 0)
				|| (simLongExpectedResult->dataSize < 0)) {
			cerr << endl;
			cerr << "Shutdown..." << endl;
			cerr << endl;
			cerr << "Execute sequential version first: mpirun -np 1" << endl;
			cerr << endl;
			canContinue = 0;
		} else {
			canContinue = 1;
		}
	}

	// rozeslanie informacji z proc 0 czy bedziemy kontynuowac wykonanie programu
	MPI_Bcast(&canContinue, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (!canContinue) {
		// Natychmiast wylaczamy program
		MPI_Finalize();
		return 0;
	}

	canContinue = parallelSimulation(simShortData, simShortExpectedResult,
			clusterSize, rank);

	MPI_Bcast(&canContinue, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if (!canContinue) {
		// Natychmiast wylaczamy program
		MPI_Finalize();
		return 0;
	}

	int resultOK = parallelSimulation(simLongData, simLongExpectedResult,
			clusterSize, rank);

	if (rank == 0) {
		if (resultOK) {
			cout << endl;
			cout << endl;
			cout << "*************************" << endl;
			cout << "TEST ZAKONCZONY POPRAWNIE" << endl;
			cout << "TEST ZAKONCZONY POPRAWNIE" << endl;
			cout << "TEST ZAKONCZONY POPRAWNIE" << endl;
			cout << "TEST ZAKONCZONY POPRAWNIE" << endl;
			cout << "*************************" << endl;
			cout << endl;
			cout << endl;
		} else {
			cout << endl;
			cout << endl;
			cout << "*************************" << endl;
			cout << " _____     _ _ " << endl;
			cout << "|  ___|_ _(_) |" << endl;
			cout << "| |_ / _` | | |" << endl;
			cout << "|  _| (_| | | |" << endl;
			cout << "|_|  \\__,_|_|_|" << endl;
			cout << endl;
			cout << "*************************" << endl;
			cout << endl;
			cout << endl;
		}
	}

	MPI_Finalize();

	return 0;
}

