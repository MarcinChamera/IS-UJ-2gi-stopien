#include "LifeParallelImplementation.h"
#include <stdlib.h>

struct drand48_data drandBuffer;
#pragma omp threadprivate(drandBuffer)

LifeParallelImplementation::LifeParallelImplementation() {
	#pragma omp parallel
	{
	srand48_r(omp_get_thread_num(), &drandBuffer);
	}
}

void LifeParallelImplementation::oneStep() {
	#pragma omp parallel
	{
		int neighbours;
		double rnd;
		#pragma omp for schedule(dynamic) nowait
		for (int row = 0; row < size; row++) {
			for (int col = 0; col < size; col++) {
				neighbours = liveNeighbours(row, col);
				drand48_r(&drandBuffer, &rnd);
				if (cells[row][col]) {
					if (rules->cellDies(neighbours, age[row][col], rnd)) {
						nextGeneration[row][col] = 0;
						age[row][col] = 0;
					} else {
						nextGeneration[row][col] = 1;
						age[row][col]++;
					}
				} else {
					if (rules->cellBecomesLive(neighbours,
							neighboursAgeSum(row, col), rnd)) {
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

double LifeParallelImplementation::avgNumerOfLiveNeighboursOfLiveCell() {
	int sumOfNeighbours = 0;
	int counter = 0;
	#pragma omp parallel for reduction(+ : sumOfNeighbours, counter) schedule(dynamic)
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

int LifeParallelImplementation::maxSumOfNeighboursAge() {
	int max_value = 0;
	#pragma omp parallel
	{
		int sumOfNeighboursAge;
		#pragma omp for schedule(dynamic) reduction(max : max_value) nowait
		for (int row = 1; row < size - 1; row++) {
			for (int col = 1; col < size - 1; col++) {
				sumOfNeighboursAge = neighboursAgeSum(row, col);
				if (max_value < sumOfNeighboursAge)
					max_value = sumOfNeighboursAge;
			}
		}
	}
	return max_value;
}

int* LifeParallelImplementation::numberOfNeighboursStatistics() {
	int *tbl = new int[9];
	for (int i = 0; i < 9; i++)
		tbl[i] = 0;
	#pragma omp parallel for schedule(dynamic, 9) reduction(+ : tbl[:9])
	for (int row = 1; row < size - 1; row++)
		for (int col = 1; col < size - 1; col++) {
			tbl[liveNeighbours(row, col)]++;
		}

	return tbl;
}