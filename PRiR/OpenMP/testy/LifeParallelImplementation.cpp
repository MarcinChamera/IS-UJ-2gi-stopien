#include <stdlib.h>
#include "LifeParallelImplementation.h"
#include <math.h>
#include <iomanip>
#include <iostream>
#include <omp.h>
 
struct drand48_data randBuffer;
#pragma omp threadprivate(randBuffer)
 
LifeParallelImplementation::LifeParallelImplementation() {
    #pragma omp parallel 
    {
        long long seed = (time(NULL) + (omp_get_thread_num() * 31 * 31 * 999));
        //long long seed = 1;
        srand48_r(seed, &randBuffer);
    }
}
 
// do zrownoleglenia
//uwazac na age, najlepiej jakby zalozyc sekcje krytyczna na odpowiednie miejsca lub odnosic sie do age sprzed wejscia do petli, chyba private/firstprivate czy cos to zalatwia
void LifeParallelImplementation::oneStep() {
    // int** tabelkaTestowa=new int*[20];
    //  for (int i = 0; i < 20; i++)
    //      tabelkaTestowa[i] = new int[20];
 
    #pragma omp parallel
    {
    int neighbours;
    double randomek;
    #pragma omp for ordered collapse(2)
    //#pragma omp for collapse(2)
    for (int row = 0; row < size; row++)
        for (int col = 0; col < size; col++) {
            
            neighbours = liveNeighbours(row, col);
            drand48_r(&randBuffer, &randomek);
            #pragma omp ordered
            {
            if (cells[row][col]) {
                // komorka zyje
                
                
                if (rules->cellDies(neighbours, age[row][col], randomek)) {
                    // smierc komorki
                    nextGeneration[row][col] = 0;
                    age[row][col] = 0;
                    //tabelkaTestowa[row][col]=20*row+col;
                    //std::cout << row << " "<<col<<std::endl;
                } else {
                    // komorka zyje nadal, jej wiek rosnie
                    nextGeneration[row][col] = 1;
                    age[row][col]++;
                    //tabelkaTestowa[row][col]=20*row+col;
                    //std::cout << row << " "<<col<<std::endl;
                }
            } else {
                // komorka nie zyje
    
 
                if (rules->cellBecomesLive(neighbours,
                        neighboursAgeSum(row, col), randomek)) {
                    // narodziny
                    nextGeneration[row][col] = 1;
                    age[row][col] = 0;
                    //tabelkaTestowa[row][col]=20*row+col;
                    //std::cout << row << " "<<col<<std::endl;
                } else {
                    nextGeneration[row][col] = 0;
                    //tabelkaTestowa[row][col]=20*row+col;
                    //std::cout << row << " "<<col<<std::endl;
                }
            }
            }
        }
    }
    int **tmp = cells;
    cells = nextGeneration;
    nextGeneration = tmp;
    // for (int row = 0; row < size; row++)
    //  for (int col = 0; col < size; col++) {
    //      std::cout<<" "<<tabelkaTestowa[row][col]<<" "<<std::endl;
    //  }
}
 
// do zrownoleglenia
double LifeParallelImplementation::avgNumerOfLiveNeighboursOfLiveCell() {
    int sumOfNeighbours = 0;
    int counter = 0;
    #pragma omp parallel for reduction(+ : sumOfNeighbours) reduction(+ : counter) collapse(2)
    //{
        //#pragma omp for collapse(2)
        for (int row = 1; row < size - 1; row++)
            for (int col = 1; col < size - 1; col++) {
                if (cells[row][col]) {
                    sumOfNeighbours += liveNeighbours(row, col);
                    counter++;
                }
            }
    //}
    if (counter == 0)
        return 0.0;
    return (double) sumOfNeighbours / (double) counter;
}
 
// do zrownoleglenia
// tu ten if niebezpieczny, bo zapis do niego i odczyt musi byc bezpieczny, case taki ze internet powie jak to zrobic w 5 minut (oby)
 
 
int LifeParallelImplementation::maxSumOfNeighboursAge() {
    int sumOfNeighboursAge;
    int max = 0;
    #pragma omp parallel private(sumOfNeighboursAge)
    {
        int local_max=0;
        #pragma omp for collapse(2)
        for (int row = 1; row < size - 1; row++)
            for (int col = 1; col < size - 1; col++) {
                sumOfNeighboursAge = neighboursAgeSum(row, col);
 
                if (local_max < sumOfNeighboursAge) {
                    local_max = sumOfNeighboursAge;
                }
                
            }
        #pragma omp critical
        {
            if ( max < local_max ) {
                max = local_max;
            }
        }
    }
    return max;
}
 
// do zrownoleglenia
// pewnie pulapka w tym 1 forze, zrownoleglanie czegos co ma 9 iteracji to zazwyczaj glupota, chyba nie powinno sie tego robic
 
// tu powiem o co w ogóle chodzi w metodzie, bo mi to zajelo chwile, lecisz po kazdej komorce (oprocz brzegow) i sprawdzasz, ile ma zywych sasiadow
// jesli 5, to inkrementujesz 5 index w tbl, jest 3 to inkrementujesz 3 w tbl itp.
// na koniec dostajesz lacznie tabele z iloscią żywych sąsiadów na każdym indexie
 
//pewnie chodzi o to, zeby przerobic w jakis sposb to na reduce, ale nie wiem jak reduce ma sie do wartosci w tablicy w porwnaniu do inkremetnowania pojedynczej zmiennej, w sumie to 9 pojedynczych zmiennych umieszczonych obok siebie w pamieci, wiec w sumie co moze pojsc zle?
int* LifeParallelImplementation::numberOfNeighboursStatistics() {
    int *tbl = new int[9]; // od 0 do 8 sąsiadów włącznie
    for (int i = 0; i < 9; i++)
        tbl[i] = 0;
    #pragma omp parallel for reduction(+:tbl[:9]) collapse(2)   
    //{
        //#pragma omp for collapse(2)
        for (int row = 1; row < size - 1; row++)
            for (int col = 1; col < size - 1; col++) {
                tbl[liveNeighbours(row, col)]++;
            }
    //}
    return tbl;
}