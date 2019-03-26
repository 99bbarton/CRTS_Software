//Implementation of the Module class representing a CRV module

#include "Module.h"
#include <TCanvas.h>

//Default constuctor, do nothing
Module::Module(){}


/*
	Constructor  - initialize all fields
	@param counters : Length 64 array of counter IDs in order from bottom left to upper right as seen from A-side of module
	@param dicounters : Length 32 array of dicounter IDs in order from bottom left to upper right as seen from A-side of module
	@param xMin : x value of left side of counter[0] relative to coordinate origin of test stand
	@param zMin : z value of bottom of counter[0] relative to coordinate origin of test stand
	@param offset : true if the module is offset
	@param id : Module ID (e.g "crvmod-101")
	@param crvData : vector containing CRV readout data from test
	@param cscData : vector containing CSC data from test
*/
Module::Module(int counters[], int dicounters[], int xMin, int zMin, bool offset, string id, vector<CRVEvent> crvData, vector<EventData> cscData)
{
	this->xMin = xMin;
	this->zMin = zMin;
	if (offset)
		this->offset = LAYER_OFFSET;
	else
		this->offset = 0;
	ID = id;

	countersA = new vector<Counter>();
	countersB = new vector<Counter>();
	initializeCounters(counters, dicounters);

	crvEvents = crvData;
	cscEvents = cscData;
	fillCounters();
}


//Destructor
Module::~Module()
{
	delete crvEvents;
	delete cscEvents;
	delete countersA;
	delete countersB;
}


/*
	Initialize countersA and countersB
	@param counters   : Length 64 : Array of counter IDs in order from bottom left to upper right as seen from A-side of module
	@param dicounters : Length 32 : Array of dicounter IDs in order from bottom left to upper right as seen from A-side of module
*/
void Module::initializeCounters(int counters, int dicounters)
{
	int x, z, layer, posInLayer, diID, dicInLayer;
	Counter counter;
	for (int c = 0; c < NUM_COUNTERS; c++) 
	{
		layer = c / 16;
		posInLayer = c % 16;
		dicInLayer = posInLayer / 2;
		diID = dicounters[c / 2]; 
		x = get_xMin() + (posInLayer * COUNTER_WIDTH) + (layer * get_offset());
		x += ((posInLayer + 1) / 2 * COUNTER_GAP) + (dicInLayer * DICOUNTER_GAP);
		z = get_zMin() + (layer * COUNTER_HEIGHT) + (layer * ABSORBER_THICKNESS);

		counter = new Counter(x, z, layer, posInLayer);

		countersA.push(counter);
		countersB.push(counter);
	}
}


//Return true if Module is ready to analyze (aka both CRV and CSC event data have been applied to the Module)
bool Module::analysisReady() const
{
	return (get_crvEvents().size() > 0) && (get_cscEvents().size() > 0);
}


//Return x value relative to test stand origin of left side counter in bottom left as viewed from side A
int Module::get_xMin() const
{
	return xMin;
}


//Return z value relative to test stand origin of bottom layer of dicounters
int Module::get_zMin() const
{
	return zMin;
}


//Return the offset of each layer. Either 0 or LAYER_OFFSET
double Module::get_offset() const
{
	return offset;
}


//Return the ID of the module (e.g. 101 for crvmod-101)
int Module::get_ID() const
{
	return ID;
}


//Return a vector containing all of the CRV events of a test
vector<CRVEvent>* Module::get_crvEvents() const
{
	return crvEvents;
}


//Return a vector containing all of the CSC events of a test
vector<EventData>* Module::get_cscEvents() const
{
	return cscEvents;
}


//Return a vector containing Counter objects corresponding to A-side readouts
Counter[] Module::get_countersA() const
{
	return countersA;
}


//Return a vector containing Counter objects corresponding to B-side readouts
Counter[] Module::get_countersB() const
{
	return countersB;
}


/*
	@TODO NEED CRV EVENT DATA CLASS TO WRITE
	Adds hits described in crvData to the module Counter objects located in countersA and countersB 
*/
void Module::fillCounters()
{
	//@TODO
}


/*
	Return a pointer to a vector containing the Counters which were hit by a muon
	@param track : The reconstructed Track of the muon
*/
vector<Counter>* Module::getHitCounters(Track track) const
{
	vector<Counter> hitCounters = new vector<Counter>();
	for (int c = 0; c < NUM_COUNTERS; c++)
	{
		if (countersA[c].calcPathLength(track) > 0)
			hitCounters.push(counters[A]);
	}

	return hitCounters;
}


/*
	Calculate the mean total (both ends) PE yield per cm of all hits and all counters
*/
double Module::calcAvgPE_per_cm() const
{
	double avg = 0;
	double sideAval;
	double sideBval;
	int numHit = NUM_COUNTERS * 2;

	for (int c = 0; c < NUM_COUNTERS; c++)
	{
		sideAval = countersA[c].calcAvg_PE_per_cm();
		sideBval = countersB[c].calcAvgPE_per_cm();
		if (sideAval + sideBval <= 0)
			numHit--;
		else
			avg += sideAval + sideBval;
	}

	return avg / numHit;
}


/*
	Make a histogram of the PE yields of every hit from every Counter (both ends)
*/
TH1F* Module::makePEHist() const
{
	TH1F* hist = new TH1F*("peHist", "PE Yield: All module Readouts", 100, 0, 100);
	hist->SetXTitle("PE Yield");

	vector<Hit> aSideHits;
	vector<Hit> bSideHits;

	for (int c = 0; c < NUM_COUNTERS; c++)
	{
		aSideHits = countersA[c].get_hits();
		bSideHits = countersB[c].get_hits();
		for (int h = 0; h < aSideHits.size(); h++)
		{
			hist->Fill(aSideHits.pop_back().get_PEyield());
		}

		for (int i = 0; i < bSideHits; i++)
		{
			hist->Fill(bSideHits.pop_back().get_PEyield());
		}
	}

	return hist;
}


/*
	Make a histogram of the PE yield per cm of every hit in both ends of every counter
*/
TH1F* Module::makePE_per_cm_Hist() const
{
	TH1F* hist = new TH1F*("pe_per_cm_Hist", "PE Yield per cm: All module Readouts", 100, 0, 100);
	hist->SetXTitle("PE Yield per cm");

	vector<Hit> aSideHits;
	vector<Hit> bSideHits;

	for (int c = 0; c < NUM_COUNTERS; c++)
	{
		aSideHits = countersA[c].get_hits();
		bSideHits = countersB[c].get_hits();
		for (int h = 0; h < aSideHits.size(); h++)
		{
			hist->Fill(aSideHits.pop_back().get_PE_per_cm());
		}

		for (int i = 0; i < bSideHits; i++)
		{
			hist->Fill(bSideHits.pop_back().get_PE_per_cm());
		}
	}

	return hist;
}


/*
	Plot the PE yield histograms of each counter

	Plots on 4 canvases, 1 for each layer of the module. A side readouts plotted in blue, B side in red
*/
void Module::plotAllPEHists() const
{
	//Set up canvases
	TCanvas canvases[4];
	canvases[0] = new TCanvas("layer0Canv", "Layer 0 PE Yield", 800, 400);
	canvases[1] = new TCanvas("layer1Canv", "Layer 1 PE Yield", 800, 400);
	canvases[2] = new TCanvas("layer2Canv", "Layer 2 PE Yield", 800, 400);
	canvases[3] = new TCanvas("layer3Canv", "Layer 3 PE Yield", 800, 400);
	canvases[0].Divide(4, 2);
	canvases[1].Divide(4, 2);
	canvases[2].Divide(4, 2);
	canvases[3].Divide(4, 2);

	TH1F *aSideHist;
	TH1F *bSideHist;
	int layer, posInLayer;

	for (int c = 0; c < NUM_COUNTERS; c++)
	{
		layer = countersA[c].get_layer();
		posInLayer = countersA[c].get_posInLayer();

		aSideHist = countersA[c].buildPE_Hist();
		bSideHist = countersB[c].buildPE_Hist();
		aSideHist->SetLineColor(kBlue);
		bSideHist->SetLineColor(kRed);

		canvases[layer].cd(posInLayer + 1);
		aSideHist->Draw();
		bSideHist->Draw("same");
	}

	for (int i = 0; i < 4; i++)
	{
		canvases[i]->Update();
	}

}


/*
Plot the PE yield per cm histograms of each counter

Plots on 4 canvases, 1 for each layer of the module. A side readouts plotted in blue, B side in red
*/
void Module::plotAllPE_per_cm_Hists() const
{
	//Set up canvases
	TCanvas canvases[4];
	canvases[0] = new TCanvas("layer0Canv_2", "Layer 0 PE Yield per cm", 800, 400);
	canvases[1] = new TCanvas("layer1Canv_2", "Layer 1 PE Yield per cm", 800, 400);
	canvases[2] = new TCanvas("layer2Canv_2", "Layer 2 PE Yield per cm", 800, 400);
	canvases[3] = new TCanvas("layer3Canv_2", "Layer 3 PE Yield per cm", 800, 400);
	canvases[0].Divide(4, 2);
	canvases[1].Divide(4, 2);
	canvases[2].Divide(4, 2);
	canvases[3].Divide(4, 2);

	TH1F *aSideHist;
	TH1F *bSideHist;
	int layer, posInLayer;

	for (int c = 0; c < NUM_COUNTERS; c++)
	{
		layer = countersA[c].get_layer();
		posInLayer = countersA[c].get_posInLayer();

		aSideHist = countersA[c].buildPE_Hist();
		bSideHist = countersB[c].buildPE_Hist();
		aSideHist->SetLineColor(kBlue);
		bSideHist->SetLineColor(kRed);

		canvases[layer].cd(posInLayer + 1);
		aSideHist->Draw();
		bSideHist->Draw("same");
	}

	for (int i = 0; i < 4; i++)
	{
		canvases[i]->Update();
	}
}


/*
	@TODO Make some sort of event display that displays module geometry, CRV hits, and CSC-produced tracks
*/
void Module::displayEvents()
{

}
