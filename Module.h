//Class to represent the CRV module

#ifndef _MODULE_H
#define _MODULE_H

#include <string>
#include <vector>
#include <TH1F.H>
#include "EventData.h"
#include "Counter.h"

class Module{
 public:
  Module();
  Module(int counters[],int dicounters[], int xMin, int zMin, bool offset, string id, vector<CRVEvent> crvData, vector<EventData> cscData);
  ~Module();
  void initializeCounters(int counters, int dicounters); //Initialize countersA and countersB
  bool analysisReady() const; //True if module has boot CRV and CSC data stored
  int get_xMin() const;
  int get_zMin();
  int get_ID() const;
  double get_offset() const;
  vector<CRVEvents>* get_crvEvents() const;
  vector<EventData>* get_cscEvents() const;
  Counter[] get_countersA() const;
  Counter[] get_countersB() const;
  void fillCounters(); //Apply the CRV and CSC data to the Counters
  vector<Counter> getHitCounters(Track t) const; //Return list of counters in a track
  double calcAvgPE_per_cm(); //return mean(pe/cm) of all counters
  TH1F* makePEHist();
  TH1F* makePE_per_cm_Hist();
  void plotAllPEHists(); //Plot PE hists of each counter
  void plotAllPE_per_cm_Hists();
  void displayEvents(); //Calls event display class

 private:
	 int xMin;
	 int zMin;
	 double offset;
	int ID;
  Counter countersA[]; //Side-A counters
  Counter countersB[]; //Side-B counters
  vector<CRVEvent>* crvEvents;
  vector<EventData>* cscEvents;
};

#endif
