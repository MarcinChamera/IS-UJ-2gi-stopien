/*
 * MPIDataProcessor.cpp
 *
 *  Created on: xxx
 *      Author: chamera
 */

#include "MPIDataProcessor.h"
#include "Alloc.h"
#include "mpi.h"
#include <string>

MPIDataProcessor::MPIDataProcessor() {
	MPI_Comm_size( MPI_COMM_WORLD, &numOfProcesses );
}

// tylko proces 0 ma dostęp do tablicy - dlatego wysyła on części tablicy każdemu z procesów, a potem odbiera od nich obliczone wartości
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

// ilosc kolumn dla pierwszych n-1 procesow: x = dataSize / numOfProcesses
// ilosc kolumn dla ostatniego procesu : y = dataSize - (numOfProcesses - 1) * x
void MPIDataProcessor::singleExecution() {
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Status status;
	// jako proces 0 udostepnij innym procesom informacje o dataSize
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
	// std::cout << "columnsNotLastProcess " << columnsNotLastProcess << ", columnsLastProcess " << columnsLastProcess << std::endl;
	// std::cout << "rank " << rank << ", columnsInCurrentProcess " << columnsInCurrentProcess << std::endl;
	int currentProcessStartingColumn = rank * columnsNotLastProcess;
	double *dataPortionBuffer = new double[dataPortionSize]; // dataPortionSize rowne 25
	// maksymalnie potrzebne będzie x kolumn z poprzedniego/następnego procesu, gdzie x = margin
	double *columnsFromPreviousProcess = new double[margin * dataSize]; 
	double *columnsFromNextProcess = new double[margin * dataSize]; 
	double *columnsForNextProcess = new double[margin * dataSize]; 
	double *columnsForPreviousProcess = new double[margin * dataSize];
	double *calculatedDataPortionBuffer = new double[(dataSize - 2 * margin) * (columnsInCurrentProcess - margin)];
	double *processTablePortion = new double[columnsInCurrentProcess * dataSize];
	// jako proces zerowy udostępnij każdemu z innych procesów część tablicy
	if (rank == 0) {
		int sizeToSend;
		// udostepnij czesc tablicy procesowi i
		for (int i = 1; i < numOfProcesses; i++) {
			bool isLastProcess = i == numOfProcesses - 1;
			sizeToSend = (!isLastProcess ? columnsNotLastProcess * dataSize : columnsLastProcess * dataSize);
			int counter = 0;
			int columnsInReceivingProcess = !isLastProcess ? columnsNotLastProcess : columnsLastProcess;
			double *tablePortionSend = new double[columnsInReceivingProcess * dataSize];
			for (int k = i * columnsNotLastProcess; k < i * columnsNotLastProcess + columnsInReceivingProcess; k++) {
				for (int j = 0; j < dataSize; j++) {
					tablePortionSend[counter++] = data[k][j];
				}
			}
			std::cout << "Proces 0 wysyla do procesu " << i << " " << sizeToSend << " komorek:" << std::endl;
			for (int i = 0; i < sizeToSend; i++)
				std::cout << tablePortionSend[i]<< " ";
			std::cout << "\n\n";
			MPI_Send(tablePortionSend, sizeof(tablePortionSend), MPI_INT, i, i, MPI_COMM_WORLD);
			delete[] tablePortionSend;
		}
		// a dla siebie weź pierwszą część tablicy
		int counter = 0;
		for (int col = 0; col < columnsInCurrentProcess; col++) {
			for (int row = 0; row < dataSize; row++) {
				processTablePortion[counter++] = data[row][col];
			}
		} 
		std::cout << "Proces 0 wzial komorek:" << std::endl;
		for (int i = 0; i < counter; i++)
			std::cout << processTablePortion[i]<< " ";
		std::cout << "\n\n";
	}
	// jako proces różny od zerowego, odbierz swoją część tablicy
	else {
		bool isLastProcess = rank == numOfProcesses - 1;
		int sizeToRecv = (!isLastProcess ? columnsNotLastProcess * dataSize : columnsLastProcess * dataSize);
		MPI_Recv(processTablePortion, sizeToRecv, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		std::cout << "Proces " << rank << " odebral nastepujace komorki:" << std::endl;
		for (int i = 0; i < sizeToRecv; i++)
			std::cout << processTablePortion[i]<< " ";
		std::cout << "\n\n";
	}
	// // niech każdy proces każdorazowo przy repetition odbierze od i wyśle sąsiadom po x kolumn, gdzie x = margin
	// // (oprócz procesu pierwszego, który nie ma wcześniejszego sąsiada, i procesu ostatniego, który nie ma następnego sąsiada)
	// if (rank == 0 && numOfProcesses > 1) {
	// 	int counter = 0;
	// 	for (int i = 0; i < columnsInCurrentProcess; i++) {
	// 		for (int j = 0; j < dataSize; j++) {
	// 			columnsForNextProcess[counter++] = data[i][j];
	// 		}
	// 	}
	// 	MPI_Send(columnsForNextProcess, sizeof(columnsForNextProcess), MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
	// 	MPI_Recv(columnsFromNextProcess, sizeof(columnsFromNextProcess), MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &status);
	// }
	// else if (rank != 0 && rank != numOfProcesses - 1) {
	// 	int counter = 0;
	// 	for (int i = currentProcessStartingColumn; i < currentProcessStartingColumn + columnsInCurrentProcess; i++) {
	// 		for (int j = 0; j < dataSize; j++) {
	// 			columnsForNextProcess[counter++] = data[i][j];
	// 		}
	// 	}
	// 	MPI_Send(columnsForNextProcess, sizeof(columnsForNextProcess), MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
	// 	MPI_Recv(columnsFromNextProcess, sizeof(columnsFromNextProcess), MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &status);
	// 	counter = 0;
	// 	for (int i = currentProcessStartingColumn; i < currentProcessStartingColumn - columnsInCurrentProcess; i++) {
	// 		for (int j = 0; j < dataSize; j++) {
	// 			columnsForPreviousProcess[counter++] = data[i][j];
	// 		}
	// 	}
	// 	MPI_Send(columnsForPreviousProcess, sizeof(columnsForPreviousProcess), MPI_INT, rank - 1, 0, MPI_COMM_WORLD);
	// 	MPI_Recv(columnsFromPreviousProcess, sizeof(columnsFromPreviousProcess), MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
	// }
	// else {
	// 	int counter = 0;
	// 	for (int i = currentProcessStartingColumn; i < currentProcessStartingColumn - columnsInCurrentProcess; i++) {
	// 		for (int j = 0; j < dataSize; j++) {
	// 			columnsForPreviousProcess[counter++] = data[i][j];
	// 		}
	// 	}
	// 	MPI_Send(columnsForPreviousProcess, sizeof(columnsForPreviousProcess), MPI_INT, rank - 1, 0, MPI_COMM_WORLD);

	// 	MPI_Recv(columnsFromPreviousProcess, sizeof(columnsFromPreviousProcess), MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &status);
	// }
	delete[] columnsForNextProcess;
	delete[] columnsForPreviousProcess;
	// // stworz tablice skladajace sie z:
	// // dla procesu 0: swojej tablicy i tablicy nastepnego sasiada
	// // dla procesow (0, n - 1): tablicy poprzedniego sasiada, swojej tablicy i tablicy nastepnego sasiada
	// // dla procesu n - 1: tablicy poprzedniego sasiada i swojej tablicy
	// if (rank != 0 && rank < 1) {
	// 	arr = tablePortionAlloc(dataSize, columnsInCurrentProcess + 2 * margin);
	// 	for (int j = 0; j < columnsInCurrentProcess + 2 * margin; j++) {
	// 		for (int i = 0; i < dataSize; i ++) {
	// 			if (j < margin) {
	// 				arr[i][j] = columnsFromPreviousProcess[j * dataSize + i];
	// 			}
	// 			else if (j < margin + columnsInCurrentProcess) {
	// 				arr[i][j] = processTablePortion[j * dataSize + i];
	// 			}
	// 			else {
	// 				arr[i][j] = columnsFromNextProcess[j * dataSize + i];
	// 			}
	// 		}
	// 	}
	// }
	// else {
	// 	arr = tablePortionAlloc(dataSize, columnsInCurrentProcess + margin);
	// 	if (rank == 0) {
	// 		for (int j = 0; j < columnsInCurrentProcess + margin; j++) {
	// 			for (int i = 0; i < dataSize; i ++) {
	// 				if (j < columnsInCurrentProcess) {
	// 					arr[i][j] = processTablePortion[j * dataSize + i];
	// 				}
	// 				else {
	// 					arr[i][j] = columnsFromNextProcess[j * dataSize + i];
	// 				}
	// 			}
	// 		}
	// 	}
	// 	else {
	// 		for (int j = 0; j < columnsInCurrentProcess + margin; j++) {
	// 			for (int i = 0; i < dataSize; i ++) {
	// 				if (j < margin) {
	// 					arr[i][j] = columnsFromPreviousProcess[j * dataSize + i];
	// 				}
	// 				else {
	// 					arr[i][j] = processTablePortion[j * dataSize + i];
	// 				}
	// 			}
	// 		}
	// 	}
	// }
	delete[] columnsFromPreviousProcess;
	delete[] columnsFromNextProcess;
	delete[] processTablePortion;
	// // oblicz komorki
	// int counter = 0;
	// for (int row = margin; row < dataSize - margin; row++) {
	// 	for (int col = margin; col < columnsInCurrentProcess; col++) {
	// 		createDataPortion(row, col, dataPortionBuffer);
	// 		calculatedDataPortionBuffer[counter++] = function->calc(dataPortionBuffer);
	// 	}
	// }
	delete[] dataPortionBuffer;
	// // wyslij obliczone komorki do procesu 0
	// if (rank != 0) {
	// 	MPI_Send(calculatedDataPortionBuffer, sizeof(calculatedDataPortionBuffer), MPI_INT, 0, 0, MPI_COMM_WORLD);
	// }
	// // jako proces 0: 
	// else {
	// 	// zapisz obliczone przez siebie komorki
	// 	for (int i = 0; i < dataSize; i++) {
	// 		for (int j = 0; j < columnsInCurrentProcess; j++) {
	// 			nextData[i][j] = calculatedDataPortionBuffer[i * dataSize + j];
	// 		}
	// 	}
	// 	// odbierz obliczone komorki z pozostalych procesow
		// double *calculatedDataFromOtherProcess = new double[dataSize * columnsLastProcess];;
	// 	MPI_Status status;
	// 	int sizeToReceive;
	// 	for (int j = columnsInCurrentProcess; j < dataSize; j++) {
	// 		// jesli potrzeba zestawu kolumn z innego procesu (dziala tez przy sprawdzaniu, czy potrzeba kolumny z ostatniego procesu)
	// 		if (j != columnsInCurrentProcess && j % columnsInCurrentProcess == 0) {
	// 			int processNumber = j / columnsInCurrentProcess; // powinno tez dzialac
	// 			if (j != dataSize - columnsLastProcess - 1) {
	// 				sizeToReceive = dataSize * columnsNotLastProcess;
	// 			}
	// 			else {
	// 				sizeToReceive = dataSize * columnsLastProcess;
	// 			}
	// 			MPI_Recv(calculatedDataFromOtherProcess, sizeToReceive, MPI_INT, processNumber, 0, MPI_COMM_WORLD, &status);
	// 		}
	// 		// zapisz obliczone komorki z pozostalych procesow
	// 		for (int i = 0; i < dataSize; i++) {
	// 			nextData[i][j] = calculatedDataFromOtherProcess[j * dataSize + i];
	// 		}
	// 	}
	// 	double **tmp = data;
	// 	data = nextData;
	// 	nextData = tmp;
	// }
	delete[] calculatedDataPortionBuffer;
}

void MPIDataProcessor::createDataPortion(int row, int col, double *buffer) {
	int counter = 0;
	for (int i = row; i <= row + margin; i++)
		for (int j = col - margin; j <= col + margin; j++)
			buffer[counter++] = arr[i][j];
}

double **MPIDataProcessor::tablePortionAlloc( int rows, int cols ) {
	double **result;
	result = new double* [ rows ]; // size rows
	for ( int i = 0; i < rows; i++ )
		result[ i ] = new double[ cols ]; // size cols
	return result;
}

// void DataProcessor::execute(int repetitions) {
// 	shareData();
// 	for (int i = 0; i < repetitions; i++)
// 		singleExecution();
// 	collectData();
// }