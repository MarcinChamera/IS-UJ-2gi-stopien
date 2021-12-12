/*
 * MPIDataProcessor.cpp
 *
 *  Created on: 12.12.2021
 *      Author: chamera
 */

#include "MPIDataProcessor.h"
#include "Alloc.h"
#include "mpi.h"

MPIDataProcessor::MPIDataProcessor() {
	MPI_Comm_size( MPI_COMM_WORLD, &numOfProcesses );
}

void MPIDataProcessor::shareData() {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	if (rank == 0) {
		nextData = tableAlloc(dataSize);
		for (int i = 0; i < dataSize; i++)
			for (int j = 0; j < dataSize; j++)
				nextData[i][j] = data[i][j];
	}
}

void MPIDataProcessor::singleExecution() {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status status;
	if (rank == 0) {
		int *dataSizeBuffer = new int[1];
		dataSizeBuffer[0] = dataSize;
		for (int i = 1; i < numOfProcesses; i++)
			MPI_Send(dataSizeBuffer, sizeof(int), MPI_INT, i, 0, MPI_COMM_WORLD);
		delete[] dataSizeBuffer;
	}
	else {
		int *dataSizeBuffer = new int[1];
		MPI_Recv(dataSizeBuffer, sizeof(int), MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		dataSize = dataSizeBuffer[0];
		delete[] dataSizeBuffer;
	}
	int columnsNotLastProcess = (dataSize % numOfProcesses != 0 ? dataSize / numOfProcesses + 1 : dataSize / numOfProcesses);
	int columnsLastProcess = dataSize - (numOfProcesses - 1) * columnsNotLastProcess;
	int columnsInCurrentProcess = (rank != numOfProcesses - 1 ? columnsNotLastProcess : columnsLastProcess);
	int currentProcessStartingColumn = rank * columnsNotLastProcess;
	double *dataPortionBuffer = new double[dataPortionSize]; 
	double *columnsFromPreviousProcess = new double[margin * dataSize]; 
	double *columnsFromNextProcess = new double[margin * dataSize]; 
	double *columnsForNextProcess = new double[margin * dataSize]; 
	double *columnsForPreviousProcess = new double[margin * dataSize];
	double *calculatedDataPortionBuffer = new double[(dataSize - 2 * margin) * columnsInCurrentProcess];
	double *processTablePortion = new double[columnsInCurrentProcess * dataSize];
	double **arr;
	if (rank == 0) {
		int sizeToSend;
		for (int i = 1; i < numOfProcesses; i++) {
			bool isLastProcess = i == numOfProcesses - 1;
			int counter = 0;
			int columnsInReceivingProcess = !isLastProcess ? columnsNotLastProcess : columnsLastProcess;
			double *tablePortionSend = new double[columnsInReceivingProcess * dataSize];
			for (int k = i * columnsNotLastProcess; k < i * columnsNotLastProcess + columnsInReceivingProcess; k++) {
				for (int j = 0; j < dataSize; j++) {
					tablePortionSend[counter++] = data[k][j];
				}
			}
			MPI_Send(tablePortionSend, columnsInReceivingProcess * dataSize, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
			delete[] tablePortionSend;
		}
		int counter = 0;
		for (int col = 0; col < columnsInCurrentProcess; col++) {
			for (int row = 0; row < dataSize; row++) {
				processTablePortion[counter++] = data[row][col];
			}
		} 
	}
	else {
		bool isLastProcess = rank == numOfProcesses - 1;
		int sizeToRecv = (!isLastProcess ? columnsNotLastProcess * dataSize : columnsLastProcess * dataSize);
		MPI_Recv(processTablePortion, sizeToRecv, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD, &status);
	}
	if (rank == 0 && numOfProcesses > 1) {
		int counter = 0;
		for (int i = columnsInCurrentProcess - margin; i < columnsInCurrentProcess; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForNextProcess[counter++] = processTablePortion[i * dataSize + j];
			}
		}
		MPI_Send(columnsForNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank + 1, MPI_COMM_WORLD);

		MPI_Recv(columnsFromNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, &status);
	}
	else if (rank != 0 && rank != numOfProcesses - 1) {
		int counter = 0;
		for (int i = columnsInCurrentProcess - margin; i < columnsInCurrentProcess; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForNextProcess[counter++] = processTablePortion[i * dataSize + j];
			}
		}
		
		MPI_Send(columnsForNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank + 1, MPI_COMM_WORLD);

		MPI_Recv(columnsFromNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, &status);
	
		counter = 0;
		for (int i = 0; i < margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForPreviousProcess[counter++] = processTablePortion[i * dataSize + j];
			}
		}
		MPI_Send(columnsForPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD);
		
		MPI_Recv(columnsFromPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank, MPI_COMM_WORLD, &status);
	}
	else {
		int counter = 0;
		for (int i = 0; i < margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForPreviousProcess[counter++] = processTablePortion[i * dataSize + j];
			}
		}
		MPI_Send(columnsForPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD);
		
		MPI_Recv(columnsFromPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank, MPI_COMM_WORLD, &status);
	}
	delete[] columnsForNextProcess;
	delete[] columnsForPreviousProcess;

	arr = (rank == 0 || rank == numOfProcesses - 1) ? tablePortionAlloc(dataSize, columnsInCurrentProcess + margin) : tablePortionAlloc(dataSize, columnsInCurrentProcess + 2 * margin);
	if (rank != 0 && rank < numOfProcesses - 1) {
		for (int i = 0; i < columnsInCurrentProcess + 2 * margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				if (i < margin) {
					arr[j][i] = columnsFromPreviousProcess[i * dataSize + j];
				}
				else if (i < margin + columnsInCurrentProcess) {
					arr[j][i] = processTablePortion[(i - margin) * dataSize + j];
				}
				else {
					arr[j][i] = columnsFromNextProcess[(i - margin - columnsInCurrentProcess) * dataSize + j];
				}
			}
		}
	}
	else {
		if (rank == 0) {
			for (int i = 0; i < columnsInCurrentProcess + margin; i++) {
				for (int j = 0; j < dataSize; j++) {
					if (i < columnsInCurrentProcess) {
						arr[j][i] = processTablePortion[i * dataSize + j];
					}
					else {
						arr[j][i] = columnsFromNextProcess[(i - columnsInCurrentProcess) * dataSize + j];
					}
				}
			}
		}
		else {
			for (int i = 0; i < columnsInCurrentProcess + margin; i++) {
				for (int j = 0; j < dataSize; j++) {
					if (i < margin) {
						arr[j][i] = columnsFromPreviousProcess[i * dataSize + j];
					}
					else {
						arr[j][i] = processTablePortion[(i - margin) * dataSize + j];
					}
				}
			}
		}
	}
	delete[] columnsFromPreviousProcess;
	delete[] columnsFromNextProcess;
	delete[] processTablePortion;

	int counter = 0;
	int columnStop;
	if (rank == 0) {
		columnStop = columnsInCurrentProcess;
	} else if (rank < numOfProcesses - 1) {
		columnStop = columnsInCurrentProcess + margin;
		if (rank == numOfProcesses - 2 && margin > columnsLastProcess)
			columnStop = columnsInCurrentProcess + margin - (margin - columnsLastProcess);
	}
	else {
		columnStop = columnsLastProcess;
		if (margin > columnsLastProcess)
			columnStop = 0;
	}
	for (int col = margin; col < columnStop; col++) {
		for (int row = margin; row < dataSize - margin; row++) {
			createDataPortion(row, col, dataPortionBuffer, arr, rank);
			calculatedDataPortionBuffer[counter++] = function->calc(dataPortionBuffer);
		}
	}
	delete[] dataPortionBuffer;
	int sizeToSend = counter;
	if (rank != 0 && sizeToSend > 0) {
		MPI_Send(calculatedDataPortionBuffer, sizeToSend, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD);
	}
	else if (rank == 0) {
		int counter = 0;
		for (int j = margin; j < columnStop; j++) {
			for (int i = margin; i < dataSize - margin; i++) {
				nextData[i][j] = calculatedDataPortionBuffer[counter++];
			}
		}
		int dataFromOtherProcessesSize = (dataSize - 2 * margin) * (dataSize - 2 * margin) - ((dataSize - 2 * margin) * (columnsInCurrentProcess - margin));
		double *calculatedDataFromOtherProcesses = new double[dataFromOtherProcessesSize];
		MPI_Status status;
		int sizeToReceive;
		double *p = calculatedDataFromOtherProcesses;
		counter = 0;
		for (int i = 1; i < numOfProcesses; i++) {
			sizeToReceive = i != numOfProcesses - 1 ? (dataSize - 2 * margin) * columnsNotLastProcess : (dataSize - 2 * margin) * (columnsLastProcess - margin);
			if (sizeToReceive > 0) {
				MPI_Recv(p, sizeToReceive, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &status);
				if (i < numOfProcesses - 1) {
					p += sizeToReceive;
				}
			}
		}
		counter = 0;
		for (int j = columnsNotLastProcess; j < dataSize - margin; j++)
			for (int i = margin; i < dataSize - margin; i++) {
				nextData[i][j] = calculatedDataFromOtherProcesses[counter++];
		}
		double **tmp = data;
		data = nextData;
		nextData = tmp;
		delete[] calculatedDataFromOtherProcesses;
	}
	delete[] calculatedDataPortionBuffer;
}

void MPIDataProcessor::createDataPortion(int row, int col, double *buffer, double **arr, int rank) {
	int counter = 0;
	for (int i = row - margin; i <= row + margin; i++) {
		for (int j = col - margin; j <= col + margin; j++) {
			buffer[counter++] = arr[i][j];
		}
	}	
}

double **MPIDataProcessor::tablePortionAlloc( int rows, int cols ) {
	double **result;
	result = new double* [ rows ];
	for ( int i = 0; i < rows; i++ )
		result[ i ] = new double[ cols ];
	return result;
}