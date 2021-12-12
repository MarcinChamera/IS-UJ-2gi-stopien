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
	double *calculatedDataPortionBuffer = new double[(dataSize - 2 * margin) * columnsInCurrentProcess];
	double *processTablePortion = new double[columnsInCurrentProcess * dataSize];
	double **arr;
	// jako proces zerowy udostępnij każdemu z innych procesów część tablicy
	if (rank == 0) {
		int sizeToSend;
		// udostepnij czesc tablicy procesowi i
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
			// std::cout << "Proces 0 wysyla do procesu " << i << " " << columnsInReceivingProcess * dataSize << " komorek:" << std::endl;
			// for (int i = 0; i < columnsInReceivingProcess * dataSize; i++)
			// 	std::cout << tablePortionSend[i]<< " ";
			// std::cout << "\n\n";
			MPI_Send(tablePortionSend, columnsInReceivingProcess * dataSize, MPI_DOUBLE, i, i, MPI_COMM_WORLD);
			delete[] tablePortionSend;
		}
		// a dla siebie weź pierwszą część tablicy
		int counter = 0;
		for (int col = 0; col < columnsInCurrentProcess; col++) {
			for (int row = 0; row < dataSize; row++) {
				processTablePortion[counter++] = data[row][col];
			}
		} 
		// std::cout << "Proces 0 wzial komorek:" << std::endl;
		// for (int i = 0; i < counter; i++)
		// 	std::cout << processTablePortion[i]<< " ";
		// std::cout << "\n\n";
	}
	// jako proces różny od zerowego, odbierz swoją część tablicy
	else {
		bool isLastProcess = rank == numOfProcesses - 1;
		int sizeToRecv = (!isLastProcess ? columnsNotLastProcess * dataSize : columnsLastProcess * dataSize);
		MPI_Recv(processTablePortion, sizeToRecv, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD, &status);
		// if (rank == 1) {
		// 	std::cout << "Proces " << rank << " odebral nastepujace komorki:" << std::endl;
		// 	for (int i = 0; i < sizeToRecv; i++) {
		// 		printf("%.2f ", processTablePortion[i]);
		// 	}
		// 	std::cout << "\n\n";
		// }	
	}
	// // niech każdy proces każdorazowo przy repetition odbierze od i wyśle sąsiadom po x kolumn, gdzie x = margin
	// // (oprócz procesu pierwszego, który nie ma wcześniejszego sąsiada, i procesu ostatniego, który nie ma następnego sąsiada)
	if (rank == 0 && numOfProcesses > 1) {
		int counter = 0;
		for (int i = columnsInCurrentProcess - margin; i < columnsInCurrentProcess; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForNextProcess[counter++] = processTablePortion[i * dataSize + j];
			}
		}
		MPI_Send(columnsForNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank + 1, MPI_COMM_WORLD);
		// std::cout << "Proces " << rank << " wyslal czesc swojej tablicy watkowi " << rank + 1 << ":" << std::endl;
		// for (int i = 0; i < margin * dataSize; i++) {
		// 	printf("%.2f ", columnsForNextProcess[i]);
		// }
		// std::cout << "\n\n";


		MPI_Recv(columnsFromNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, &status);
		// std::cout << "Proces " << rank << " otrzymal czesc tablicy od watka " << rank + 1 << ":" << std::endl;
		// for (int i = 0; i < margin * dataSize; i++) {
		// 	printf("%.2f ", columnsFromNextProcess[i]);
		// }
		// std::cout << "\n\n";
	}
	else if (rank != 0 && rank != numOfProcesses - 1) {
		int counter = 0;
		for (int i = columnsInCurrentProcess - margin; i < columnsInCurrentProcess; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForNextProcess[counter++] = processTablePortion[i * dataSize + j];
			}
		}
		
		MPI_Send(columnsForNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank + 1, MPI_COMM_WORLD);
		// std::cout << "Proces " << rank << " wyslal czesc swojej tablicy watkowi " << rank + 1 << ":" << std::endl;
		// for (int i = 0; i < margin * dataSize; i++) {
		// 	printf("%.2f ", columnsForNextProcess[i]);
		// }
		// std::cout << "\n\n";


		MPI_Recv(columnsFromNextProcess, margin * dataSize, MPI_DOUBLE, rank + 1, rank, MPI_COMM_WORLD, &status);
		// if (rank == 2) {
		// 	std::cout << "Proces " << rank << " otrzymal czesc tablicy od watka " << rank + 1 << ":" << std::endl;
		// 	for (int i = 0; i < margin * dataSize; i++) {
		// 		printf("%.2f ", columnsFromNextProcess[i]);
		// 	}
		// 	std::cout << "\n\n";
		// }

		counter = 0;
		for (int i = 0; i < margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForPreviousProcess[counter++] = processTablePortion[i * dataSize + j];
			}
		}
		MPI_Send(columnsForPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD);
		// std::cout << "Proces " << rank << " wyslal czesc swojej tablicy watkowi " << rank - 1 << ":" << std::endl;
		// for (int i = 0; i < margin * dataSize; i++) {
		// 	printf("%.2f ", columnsForPreviousProcess[i]);
		// }
		// std::cout << "\n\n";


		MPI_Recv(columnsFromPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank, MPI_COMM_WORLD, &status);
		// if (rank == 2) {
		// 	std::cout << "Proces " << rank << " otrzymal czesc tablicy od watka " << rank - 1 << ":" << std::endl;
		// 	for (int i = 0; i < margin * dataSize; i++) {
		// 		printf("%.2f ", columnsFromPreviousProcess[i]);
		// 	}
		// 	std::cout << "\n\n";
		// }
	}
	else {
		int counter = 0;
		for (int i = 0; i < margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				columnsForPreviousProcess[counter++] = processTablePortion[i * dataSize + j];
			}
		}
		MPI_Send(columnsForPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank - 1, MPI_COMM_WORLD);
		// std::cout << "Proces " << rank << " wyslal czesc swojej tablicy watkowi " << rank - 1 << ":" << std::endl;
		// for (int i = 0; i < margin * dataSize; i++) {
		// 	printf("%.2f ", columnsForPreviousProcess[i]);
		// }
		// std::cout << "\n\n";


		MPI_Recv(columnsFromPreviousProcess, margin * dataSize, MPI_DOUBLE, rank - 1, rank, MPI_COMM_WORLD, &status);
		// std::cout << "Proces " << rank << " otrzymal czesc tablicy od watka " << rank - 1 << ":" << std::endl;
		// for (int i = 0; i < margin * dataSize; i++) {
		// 	printf("%.2f ", columnsFromPreviousProcess[i]);
		// }
		// std::cout << "\n\n";
	}
	delete[] columnsForNextProcess;
	delete[] columnsForPreviousProcess;

	// // stworz tablice skladajace sie z:
	// // dla procesu 0: swojej tablicy i tablicy nastepnego sasiada
	// // dla procesow od 1 dno n -1: tablicy poprzedniego sasiada, swojej tablicy i tablicy nastepnego sasiada
	// // dla procesu n - 1: tablicy poprzedniego sasiada i swojej tablicy
	arr = (rank == 0 || rank == numOfProcesses - 1) ? tablePortionAlloc(dataSize, columnsInCurrentProcess + margin) : tablePortionAlloc(dataSize, columnsInCurrentProcess + 2 * margin);
	if (rank != 0 && rank < numOfProcesses - 1) {
		// arr = tablePortionAlloc(dataSize, columnsInCurrentProcess + 2 * margin);
		for (int i = 0; i < columnsInCurrentProcess + 2 * margin; i++) {
			for (int j = 0; j < dataSize; j++) {
				if (i < margin) {
					arr[j][i] = columnsFromPreviousProcess[i * dataSize + j];
					// if (rank == 1)
					// 	std::cout << "Rzad " << j << ", kolumna " << i << ": " << columnsFromPreviousProcess[i * dataSize + j] << "\n";
				}
				else if (i < margin + columnsInCurrentProcess) {
					arr[j][i] = processTablePortion[(i - margin) * dataSize + j];
					// if (rank == 1)
					// 	std::cout << "Rzad " << j << ", kolumna " << i << ": " << processTablePortion[(i - margin) * dataSize + j] << "\n";
				}
				else {
					arr[j][i] = columnsFromNextProcess[(i - margin - columnsInCurrentProcess) * dataSize + j];
					// if (rank == 1)
						// std::cout << "Rzad " << j << ", kolumna " << i << ": " << columnsFromNextProcess[(i - margin - columnsInCurrentProcess) * dataSize + j] << "\n";
						// std::cout << "Rzad " << j << ", kolumna " << i << ": " << arr[j][i] << "\n";
				}
			}
		}
		// std::cout << "\n\n";
	}
	else {
		// arr = tablePortionAlloc(dataSize, columnsInCurrentProcess + margin);
		if (rank == 0) {
			for (int i = 0; i < columnsInCurrentProcess + margin; i++) {
				for (int j = 0; j < dataSize; j++) {
					if (i < columnsInCurrentProcess) {
						arr[j][i] = processTablePortion[i * dataSize + j];
						// std::cout << "Rzad " << j << ", kolumna " << i << ": " << processTablePortion[i * dataSize + j] << "\n";
					}
					else {
						arr[j][i] = columnsFromNextProcess[(i - columnsInCurrentProcess) * dataSize + j];
						// std::cout << "Rzad " << j << ", kolumna " << i << ": " << columnsFromNextProcess[(i - columnsInCurrentProcess) * dataSize + j] << "\n";
					}
				}
			}
		}
		else {
			for (int i = 0; i < columnsInCurrentProcess + margin; i++) {
				for (int j = 0; j < dataSize; j++) {
					if (i < margin) {
						arr[j][i] = columnsFromPreviousProcess[i * dataSize + j];
						// std::cout << "Rzad " << j << ", kolumna " << i << ": " << columnsFromPreviousProcess[i * dataSize + j] << "\n";
					}
					else {
						arr[j][i] = processTablePortion[(i - margin) * dataSize + j];
						// std::cout << "Rzad " << j << ", kolumna " << i << ": " << processTablePortion[(i - margin) * dataSize + j] << "\n";
					}
				}
			}
		}
	}
	// if (rank == 3) {
	// 	std::cout << "Proces " << rank << " do dyspozycji ma teraz tablice:" << std::endl;
	// 	for (int j = 0; j < dataSize; j++) {
	// 		for (int i = 0; i < columnsInCurrentProcess + margin; i++) {
	// 			std::cout << arr[j][i] << " ";
	// 		}
	// 		std::cout << "\n";
	// 	}
	// }
	// std::cout << "\n";
	delete[] columnsFromPreviousProcess;
	delete[] columnsFromNextProcess;
	delete[] processTablePortion;

	// // oblicz komorki
	int counter = 0;
	// tutaj ten obszar tworzenia zawezic do obszarow dla kazdego procesu
	// rank0 = [1, columnsInCurrentProcess) = [1, 2)
	// rank1 = [1, columnsInCurrentProcess + margin) = [1, 3)
	// rank2 = [1, columnsInCurrentProcess) = [1, 1)

	// rank0 = [2, columnsInCurrentProxess)
	// rank1 = [2, columnsInCurrentProcess)
	// rank3 = 
	// int columnStop = columnsInCurrentProcess;
	// int columnStop = rank != numOfProcesses - 1 && rank != 0 ? columnsInCurrentProcess + margin : columnsInCurrentProcess;
	// std::cout << "Dla procesu " << rank << " columnStop " << columnStop << std::endl;
	// for (int row = margin; row < dataSize - margin; row++) {
		// for (int col = margin; col < columnStop; col++) {
	// if (rank == numOfProcesses - 2 && columnsLastProcess < margin) {
	// 	// std::cout << "Proces " << rank << ", columnsLastProcess " << columnsLastProcess << ", margin " << margin << std::endl;
	// 	columnStop = columnsInCurrentProcess - (margin - columnsLastProcess);
	// 	std::cout << "ColumnStop " << columnStop << std::endl;
	// } 

	// tutaj trzeba pamietac, ze array ma tez kolumny z sasiednich procesow, wiec pierwsza "prawdziwa" kolumna tego procesu zaczyna sie od margin
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
		// columnStop = columnsLastProcess - margin;
	}
	// std::cout << "PRoces " << rank << ", column stop = " << columnStop << std::endl;
	// if (rank == 1) {
	// 	for (int i = 0; i < columnsInCurrentProcess + margin; i++) {
	// 		for (int j = 0; j < dataSize; j++) {
	// 			std::cout << arr[i][j] << " ";
	// 		}
	// 		std::cout << "\n";
	// 	}
	// }
	// std::cout << "Dla procesu " << rank << " columnStart wynosi " << columnStart << " a columnStop " << columnStop << std::endl;
	for (int col = margin; col < columnStop; col++) {
		for (int row = margin; row < dataSize - margin; row++) {
			// std::cout << arr[row][col] << " ";
			// std::cout << "row " << row << " - col " << col << " | ";
			createDataPortion(row, col, dataPortionBuffer, arr, rank);
			calculatedDataPortionBuffer[counter++] = function->calc(dataPortionBuffer);
			// if (rank == 1)
			// 	std::cout << "calculatedDataPortionBuffer[" << counter - 1 << "]=" <<  calculatedDataPortionBuffer[counter - 1] << std::endl;
		}
		// std::cout << "\n";
	}
	delete[] dataPortionBuffer;
	// wyslij obliczone komorki do procesu 0 (bez marginesu)
	int sizeToSend = counter;
	// std::cout << "Proces " << rank << ", sizeToSend " << sizeToSend << std::endl;
	// int sizeToSend = (dataSize - 2 * margin) * (columnStop - margin);
	if (rank != 0 && sizeToSend > 0) {
		MPI_Send(calculatedDataPortionBuffer, sizeToSend, MPI_DOUBLE, 0, rank, MPI_COMM_WORLD);
		// std::cout << "Proces " << rank << " wysyla nastepujace dane:" << std::endl;
		// for (int i = 0; i < sizeToSend; i++) {
			// std::cout << calculatedDataPortionBuffer[i] << " ";
		// }
		// std::cout << "\n";
	}
	// jako proces 0: 
	else if (rank == 0) {
		int counter = 0;
		// zapisz obliczone przez siebie komorki
		for (int j = margin; j < columnStop; j++) {
			for (int i = margin; i < dataSize - margin; i++) {
				nextData[i][j] = calculatedDataPortionBuffer[counter++];
				// std::cout << nextData[i][j] << " ";
			}
			// std::cout << "\n";
		}
		// odbierz obliczone komorki z pozostalych procesow
		int dataFromOtherProcessesSize = (dataSize - 2 * margin) * (dataSize - 2 * margin) - ((dataSize - 2 * margin) * (columnsInCurrentProcess - margin));
		// std::cout << "dataFromOtherProcessesSize " << dataFromOtherProcessesSize << std::endl;
		double *calculatedDataFromOtherProcesses = new double[dataFromOtherProcessesSize];
		MPI_Status status;
		int sizeToReceive;
		double *p = calculatedDataFromOtherProcesses;
		counter = 0;
		for (int i = 1; i < numOfProcesses; i++) {
			sizeToReceive = i != numOfProcesses - 1 ? (dataSize - 2 * margin) * columnsNotLastProcess : (dataSize - 2 * margin) * (columnsLastProcess - margin);
			// std::cout << "sizeToReceive " << sizeToReceive << std::endl;
			if (sizeToReceive > 0) {
				// double *receivedFromProcess = new double[sizeToReceive];
				// MPI_Recv(receivedFromProcess, sizeToReceive, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &status);
				// for (int i = 0; i < sizeToReceive; i++) {
				// 	calculatedDataFromOtherProcesses[counter++] = receivedFromProcess[i];
				// }
				// delete[] receivedFromProcess;
				// sizeToReceive obliczany poprawnie
				MPI_Recv(p, sizeToReceive, MPI_DOUBLE, i, i, MPI_COMM_WORLD, &status);
				if (i < numOfProcesses - 1) {
					p += sizeToReceive;
				}
				// ten cout pod spodem zwraca wszystko w porządku, ale zapis powyzej w MPI_Recv myli wyniki
				// std::cout << "Otrzymane od procesu " << i << ":" << std::endl;
				// for (int j = (i - 1) * columnsNotLastProcess; j < (i - 1) * columnsNotLastProcess + sizeToReceive; j++) {
				// 	std::cout << calculatedDataFromOtherProcesses[j] << " ";
				// }
				// std::cout << "\n";
			}
		}
		// for (int x = 0; x < 24; x++) {
		// 	std::cout << calculatedDataFromOtherProcesses[x] << " ";
		// }
		// std::cout << "\n";
		// for (int j = 1; j < dataFromOtherProcessesSize + 1; j++) {
		// 	std::cout << calculatedDataFromOtherProcesses[j - 1] << " ";
		// 	if (j % 4 == 0 && j != 0) 
		// 		std::cout << "\n";
		// }
		// std::cout << "\n";
		// zapisz obliczone komorki z pozostalych procesow
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
	// std::cout << "BLok dla procesu " << rank << ":" << std::endl;
	for (int i = row - margin; i <= row + margin; i++) {
		for (int j = col - margin; j <= col + margin; j++) {
			buffer[counter++] = arr[i][j];
			// if (rank == 1) {
			// 	std::cout << "i=" << i << ", j=" <<  j << ": " << arr[i][j] << std::endl;
			// }
		}
		// std::cout << "\n";
	}	
	// std::cout << "======" << std::endl;
	// if (rank == 1) {
	// 	std::cout << "Dla row " << row << " i col " << col << ":" << std::endl;
	// 	for (int i = 0; i < counter; i++) {
	// 		std::cout << buffer[i] << " ";
	// 	}
	// 	std::cout << "\n";
	// 	std::cout << "======" << std::endl;
	// }
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