/*
 * SimlationInfo.cpp
 *
 *  Created on: 25 lis 2021
 *      Author: oramus
 */

#include"Alloc.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <fstream>
#include "SimulationData.h"

using namespace std;

SimulationData *getShortSimulationData( void ) {
	SimulationData *simInfo = createEmptySimulationInfo();
	simInfo->dataSize = 15;
	simInfo->margin = 1;
	simInfo->repetitions = 1;
	simInfo->fileName = "ShortSimulation.txt";
	return simInfo;
}

SimulationData *getLongSimulationData( void ) {
	SimulationData *simInfo = createEmptySimulationInfo();
	simInfo->dataSize = 1567;
	simInfo->margin = 2;
	simInfo->repetitions = 128;
	simInfo->fileName = "LongSimulation.txt";
	return simInfo;
}

SimulationData *createEmptySimulationInfo( void ) {
	return new SimulationData();
}

SimulationData* loadDataFromFile(string fname) {
	SimulationData *simInfo = new SimulationData();
	ifstream report;
	report.open(fname);
	if (!report.good()) {
		simInfo->dataSize = -1;
	} else {
		report >> simInfo->dataSize;
		report >> simInfo->margin;
		report >> simInfo->repetitions;
		report >> simInfo->wallTime;
		report >> simInfo->magicFunctionCalls;
		simInfo->result = tableAlloc(simInfo->dataSize);
		for (int r = 0; r < simInfo->dataSize; r++)
			for (int c = 0; c < simInfo->dataSize; c++)
				report >> simInfo->result[r][c];
	}
	report.close();
	return simInfo;
}

void saveSimulationDataInFile(SimulationData *simInfo ) {
	saveSimulationDataInFile(simInfo, simInfo->fileName);
}

void saveSimulationDataInFile(SimulationData *simInfo, string fname) {
	ofstream report(fname);
	report << simInfo->dataSize << endl;
	report << simInfo->margin << endl;
	report << simInfo->repetitions << endl;
	report << simInfo->wallTime << endl;
	report << simInfo->magicFunctionCalls << endl;
	for (int r = 0; r < simInfo->dataSize; r++)
		for (int c = 0; c < simInfo->dataSize; c++)
			report << simInfo->result[r][c] << endl;
	report.close();
}


