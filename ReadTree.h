//ReadTree.h
#pragma once

#include "Constants.h"
#include "EventData.h" //needed for recreating the data structure
#include <map>
#include <vector>
#include "TTree.h"		//<TTree.h>
#include "TBranch.h"	//<TBranch.h>
#include "TString.h"	//<TString.h>
#include "TApplication.h" //<TApplication.h>
#include "TH1I.h"


int main(int argc, char* argv[]);

class ReadTree
{
private:
	TApplication* theApp;
	std::vector<EventData> eventList;
	TTree* vmm_tree;   //pointer to the analyzed TTree
	TBranch* b_triggerCount; //pointer to the triggercount branch
	TBranch* b_chans_board[NUM_BOARDS]; //array of pointers to each board branch
	TH1I *timeHist; //Pointer to histogram of time diff b/w scinttilator and hits
	TH1I *csc1tHist; //Timing histograms of individual chambers
	TH1I *csc2tHist;
	EventData dEvent;
	int trigCount;
	

public:
	ReadTree(TApplication*, TString, double);
	virtual ~ReadTree();
	bool getData(TTree*);
	void drawTracks();
	void calcBoardEfficiencies(double voltage);
	void makeOccupancyPlots();
	void calcLayerTrackEfficiencies(double efficiencies[]);
	void plotResiduals();

};

