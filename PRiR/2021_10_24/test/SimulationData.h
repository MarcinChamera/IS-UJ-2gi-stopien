#ifndef SIMULATION_INFO_H_
#define SIMULATION_INFO_H_

#include<iostream>

using namespace std;

struct SimulationData {
	int margin;
	int dataSize;
	int repetitions;
	long magicFunctionCalls;
	double wallTime;
	string fileName;
	double **result;
};

SimulationData *getShortSimulationData( void );
SimulationData *getLongSimulationData( void );
SimulationData *createEmptySimulationInfo( void );
SimulationData *loadDataFromFile(string fname) ;
void saveSimulationDataInFile(SimulationData *simInfo, string fname);
void saveSimulationDataInFile(SimulationData *simInfo );

#endif /* SIMULATION_INFO_H_ */
