/*
 * LifeParallelImplementation.cpp
 *
 *  Created on: 5 lis 2021
 *      Author: oramus
 */

#include "LifeParallelImplementation.h"
#include <stdlib.h>

struct drand48_data drandBuffer;
#pragma omp threadprivate(drandBuffer)

LifeParallelImplementation::LifeParallelImplementation() {

	// drandBuffer = new struct drand48_data();
	// #pragma omp parallel
	// {
	srand48_r(omp_get_thread_num(), &drandBuffer);
	// }
}

// wersja 1
// do zrownoleglenia
// void LifeParallelImplementation::oneStep() {
// 	// #pragma omp parallel shared(cells, age, nextGeneration)
// 	#pragma omp parallel
// 	{
// 		int neighbours;
// 		double rnd;
// 		struct drand48_data *drandBuffer = new struct drand48_data();
// 		srand48_r(omp_get_thread_num(), drandBuffer);
// 		#pragma omp for collapse(2)
// 		for (int row = 0; row < size; row++) {
// 			for (int col = 0; col < size; col++) {
// 				neighbours = liveNeighbours(row, col);
// 				if (cells[row][col]) {
// 					// komorka zyje
// 					drand48_r(drandBuffer, &rnd);
// 					if (rules->cellDies(neighbours, age[row][col], rnd)) {
// 						// smierc komorki
// 						nextGeneration[row][col] = 0;
// 						age[row][col] = 0;
// 					} else {
// 						// komorka zyje nadal, jej wiek rosnie
// 						nextGeneration[row][col] = 1;
// 						age[row][col]++;
// 					}
// 				} else {
// 					// komorka nie zyje
// 					drand48_r(drandBuffer, &rnd);
// 					if (rules->cellBecomesLive(neighbours,
// 							neighboursAgeSum(row, col), rnd)) {
// 						// narodziny
// 						nextGeneration[row][col] = 1;
// 						age[row][col] = 0;
// 					} else {
// 						nextGeneration[row][col] = 0;
// 					}
// 				}
// 			}
// 		}
// 	} 
// 	int **tmp = cells;
// 	cells = nextGeneration;
// 	nextGeneration = tmp;
// }

//wersja 2
// void LifeParallelImplementation::oneStep() {
// 	// #pragma omp parallel shared(cells, age, nextGeneration)
// 	#pragma omp parallel
// 	{
// 		int neighbours;
// 		double rnd;
// 		struct drand48_data *drandBuffer = new struct drand48_data();
// 		srand48_r(omp_get_thread_num(), drandBuffer);
// 		#pragma omp for
// 		for (int row = 0; row < size; row++) {
// 			for (int col = 0; col < size; col++) {
// 				neighbours = liveNeighbours(row, col);
// 				if (cells[row][col]) {
// 					// komorka zyje
// 					drand48_r(drandBuffer, &rnd);
// 					if (rules->cellDies(neighbours, age[row][col], rnd)) {
// 						// smierc komorki
// 						nextGeneration[row][col] = 0;
// 						age[row][col] = 0;
// 					} else {
// 						// komorka zyje nadal, jej wiek rosnie
// 						nextGeneration[row][col] = 1;
// 						age[row][col]++;
// 					}
// 				} else {
// 					// komorka nie zyje
// 					drand48_r(drandBuffer, &rnd);
// 					if (rules->cellBecomesLive(neighbours,
// 							neighboursAgeSum(row, col), rnd)) {
// 						// narodziny
// 						nextGeneration[row][col] = 1;
// 						age[row][col] = 0;
// 					} else {
// 						nextGeneration[row][col] = 0;
// 					}
// 				}
// 			}
// 		}
// 	} 
// 	int **tmp = cells;
// 	cells = nextGeneration;
// 	nextGeneration = tmp;
// }

// wersja 3
// void LifeParallelImplementation::oneStep() {
// 	// #pragma omp parallel shared(cells, age, nextGeneration)
// 	#pragma omp parallel
// 	{
// 		int neighbours;
// 		double rnd;
// 		struct drand48_data *drandBuffer = new struct drand48_data();
// 		srand48_r(omp_get_thread_num(), drandBuffer);
// 		#pragma omp for schedule(static, 1)
// 		for (int row = 0; row < size; row++) {
// 			for (int col = 0; col < size; col++) {
// 				neighbours = liveNeighbours(row, col);
// 				if (cells[row][col]) {
// 					// komorka zyje
// 					drand48_r(drandBuffer, &rnd);
// 					if (rules->cellDies(neighbours, age[row][col], rnd)) {
// 						// smierc komorki
// 						nextGeneration[row][col] = 0;
// 						age[row][col] = 0;
// 					} else {
// 						// komorka zyje nadal, jej wiek rosnie
// 						nextGeneration[row][col] = 1;
// 						age[row][col]++;
// 					}
// 				} else {
// 					// komorka nie zyje
// 					drand48_r(drandBuffer, &rnd);
// 					if (rules->cellBecomesLive(neighbours,
// 							neighboursAgeSum(row, col), rnd)) {
// 						// narodziny
// 						nextGeneration[row][col] = 1;
// 						age[row][col] = 0;
// 					} else {
// 						nextGeneration[row][col] = 0;
// 					}
// 				}
// 			}
// 		}
// 	} 
// 	int **tmp = cells;
// 	cells = nextGeneration;
// 	nextGeneration = tmp;
// }

//wersja 4
// void LifeParallelImplementation::oneStep() {
// 	omp_set_nested(true);
// 	#pragma omp parallel
// 	{
// 		int neighbours;
// 		double rnd;
// 		drandBuffer = new struct drand48_data();
// 		srand48_r(omp_get_thread_num(), drandBuffer);
// 		#pragma omp for
// 		for (int row = 0; row < size; row++) {
// 			for (int col = 0; col < size; col++) {
// 				neighbours = liveNeighbours(row, col);
// 				drand48_r(drandBuffer, &rnd);
// 				if (cells[row][col]) {
// 					// komorka zyje
// 					if (rules->cellDies(neighbours, age[row][col], rnd)) {
// 						// smierc komorki
// 						nextGeneration[row][col] = 0;
// 						age[row][col] = 0;
// 					} else {
// 						// komorka zyje nadal, jej wiek rosnie
// 						nextGeneration[row][col] = 1;
// 						age[row][col]++;
// 					}
// 				} else {
// 					// komorka nie zyje
// 					if (rules->cellBecomesLive(neighbours,
// 							neighboursAgeSum(row, col), rnd)) {
// 						// narodziny
// 						nextGeneration[row][col] = 1;
// 						age[row][col] = 0;
// 					} else {
// 						nextGeneration[row][col] = 0;
// 					}
// 				}
// 			}
// 		}
// 	} 
// 	int **tmp = cells;
// 	cells = nextGeneration;
// 	nextGeneration = tmp;
// }

//wersja 5
void LifeParallelImplementation::oneStep() {
	#pragma omp parallel
	{
		int neighbours;
		double rnd;
		#pragma omp for
		for (int row = 0; row < size; row++) {
			for (int col = 0; col < size; col++) {
				neighbours = liveNeighbours(row, col);
				drand48_r(&drandBuffer, &rnd);
				if (cells[row][col]) {
					// komorka zyje
					if (rules->cellDies(neighbours, age[row][col], rnd)) {
						// smierc komorki
						nextGeneration[row][col] = 0;
						age[row][col] = 0;
					} else {
						// komorka zyje nadal, jej wiek rosnie
						nextGeneration[row][col] = 1;
						age[row][col]++;
					}
				} else {
					// komorka nie zyje
					if (rules->cellBecomesLive(neighbours,
							neighboursAgeSum(row, col), rnd)) {
						// narodziny
						nextGeneration[row][col] = 1;
						age[row][col] = 0;
					} else {
						nextGeneration[row][col] = 0;
					}
				}
			}
		}
	} 
	int **tmp = cells;
	cells = nextGeneration;
	nextGeneration = tmp;
}

// do zrownoleglenia - wynik jest dobry
double LifeParallelImplementation::avgNumerOfLiveNeighboursOfLiveCell() {
	int sumOfNeighbours = 0;
	int counter = 0;
	// sumOfNeighbours i counter są domyślnie uznawane za shared
	#pragma omp parallel for reduction(+ : sumOfNeighbours, counter)
		for (int row = 1; row < size - 1; row++)
			for (int col = 1; col < size - 1; col++) {
				if (cells[row][col]) {
					sumOfNeighbours += liveNeighbours(row, col);
					counter++;
				}
			}
	if (counter == 0)
		return 0.0;
	return (double) sumOfNeighbours / (double) counter;
}

//do zrownoleglenia - poprawnie, dobry wynik
// int LifeParallelImplementation::maxSumOfNeighboursAge() {
// 	int sumOfNeighboursAge;
// 	int max_value = 0;
// 	// sumOfNeighboursAge jest domyslnie uznawana za shared
// 	#pragma omp parallel
// 	{
// 		int max_thread_value = 0;
// 		#pragma omp for collapse(2)
// 		for (int row = 1; row < size - 1; row++) {
// 			for (int col = 1; col < size - 1; col++) {
// 				sumOfNeighboursAge = neighboursAgeSum(row, col);
// 				if (max_thread_value < sumOfNeighboursAge)
// 					max_thread_value = sumOfNeighboursAge;
// 			}
// 		}

// 		#pragma omp critical
// 		{
// 			if (max_thread_value > max_value) {
// 				max_value = max_thread_value;
// 			}
// 		}
// 	}
// 	return max_value;
// }

// int LifeParallelImplementation::maxSumOfNeighboursAge() {
	int max_value = 0;
	// sumOfNeighboursAge jest domyslnie uznawana za shared
	#pragma omp parallel
	{
		int sumOfNeighboursAge;
		int max_thread_value = 0;
		#pragma omp for nowait
		for (int row = 1; row < size - 1; row++) {
			for (int col = 1; col < size - 1; col++) {
				sumOfNeighboursAge = neighboursAgeSum(row, col);
				if (max_thread_value < sumOfNeighboursAge)
					max_thread_value = sumOfNeighboursAge;
			}
		}

		#pragma omp critical
		{
		if (max_thread_value > max_value) {
			max_value = max_thread_value;
		}
		}
	}
	return max_value;
}

// int LifeParallelImplementation::maxSumOfNeighboursAge() {
// 	int max_value = 0;
// 	// sumOfNeighboursAge jest domyslnie uznawana za shared
// 	#pragma omp parallel
// 	{
// 		int sumOfNeighboursAge;
// 		int max_thread_value = 0;
// 		#pragma omp for nowait
// 		for (int row = 1; row < size - 1; row++) {
// 			for (int col = 1; col < size - 1; col++) {
// 				sumOfNeighboursAge = neighboursAgeSum(row, col);
// 				if (max_thread_value < sumOfNeighboursAge) {
// 					max_thread_value = sumOfNeighboursAge;
					
// 					if (max_thread_value > max_value) {
// 						#pragma omp critical
// 						{
// 							max_value = max_thread_value;
// 						}
// 					}
// 				}
// 			}
// 		}		
// 	}
// 	return max_value;
// }

// int LifeParallelImplementation::maxSumOfNeighboursAge() {
// 	int max_value = 0;
// 	// sumOfNeighboursAge jest domyslnie uznawana za shared
// 	#pragma omp parallel
// 	{
// 		int sumOfNeighboursAge;
// 		// int max_thread_value = 0;
// 		#pragma omp for reduction(max : max_value)
// 		for (int row = 1; row < size - 1; row++) {
// 			for (int col = 1; col < size - 1; col++) {
// 				sumOfNeighboursAge = neighboursAgeSum(row, col);
// 				if (max_value < sumOfNeighboursAge)
// 					max_value = sumOfNeighboursAge;
// 			}
// 		}
// 	}
// 	return max_value;
// }

//wersja 1
//zrownoleglic - rozwiazanie jest dobre, test przechodzi dla wszystkich napisanych tu metod
//jedynie metoda numberOfNeighboursStatistics jest wolniejsza w wersji rownoleglej (218s vs 414s)
//efektywnosc programu 13.19% (minimum to 80)
// int* LifeParallelImplementation::numberOfNeighboursStatistics() {
// 	int *tbl = new int[9]; // od 0 do 8 sąsiadów włącznie
// 	for (int i = 0; i < 9; i++)
// 		tbl[i] = 0;
// 	#pragma omp parallel for collapse(2)
// 	for (int row = 1; row < size - 1; row++)
// 		for (int col = 1; col < size - 1; col++) {
// 			#pragma omp critical
// 			tbl[liveNeighbours(row, col)]++;
// 		}

// 	return tbl;
// }

//wersja 2
//efektywnosc programu po uzyciu reduction zamiast critical: 57.97
//pomysł - nie uzywac critical w metodzie maxSumOfNeighboursAge
// int* LifeParallelImplementation::numberOfNeighboursStatistics() {
// 	int *tbl = new int[9]; // od 0 do 8 sąsiadów włącznie
// 	for (int i = 0; i < 9; i++)
// 		tbl[i] = 0;
// 	#pragma omp parallel for collapse(2) reduction(+ : tbl[:9])
// 	for (int row = 1; row < size - 1; row++)
// 		for (int col = 1; col < size - 1; col++) {
// 			// #pragma omp critical
// 			tbl[liveNeighbours(row, col)]++;
// 		}

// 	return tbl;
// }

//wersja 3
int* LifeParallelImplementation::numberOfNeighboursStatistics() {
	int *tbl = new int[9]; // od 0 do 8 sąsiadów włącznie
	for (int i = 0; i < 9; i++)
		tbl[i] = 0;
	#pragma omp parallel for reduction(+ : tbl[:9])
	for (int row = 1; row < size - 1; row++)
		for (int col = 1; col < size - 1; col++) {
			tbl[liveNeighbours(row, col)]++;
		}

	return tbl;
}