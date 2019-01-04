#pragma once

#include <map>
#include <string>
#include <fstream>
#include "Constants.h"
#include "EventData.h"

#include "TROOT.h" //<TROOT.h>
#include "TFile.h" //<TFile.h>
#include "TApplication.h" //<TApplication.h>
#include "TTree.h" //<TTree.h>
#include "TVectorF.h"

#include <vector> //probably won't need this

using std::vector;
using std::string;

int main(int, char*[]);

struct timePt //Struct for storing timing data 
{
  int t;
  int pdo;
};

class ProcessBinary
{
public:
	std::map<int, EventData> triggerMap; //map which will contain the associated channel hits

	std::map<int, vector<timePt>> timingMap;

	TTree * vmm_tree;   //!pointer to the analyzed TTree or TChain
	Int_t           fCurrent; //!current Tree number in a TChain
	std::string fileName; //Filename of root file being processed
	std::ifstream file;

	ProcessBinary();
	ProcessBinary(TApplication*, string);
	~ProcessBinary();
	bool parseFile();
	uint32_t decodeGray(uint32_t);

};


