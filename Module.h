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
  Module(int counters[], int dicounters[], int xMin, int zMin, bool offset);
  ~Module();
  string getID() const;
  double getOffset() const;
  vector<CRVEvents> getCRVEvents() const;
  vector<EventData> getCSCEvents() const;
  void fillCounters(vector<CRVEvent> crvEvents); //Add CRV data to counters
  void applyTracks(vector<EventData> cscEvents); //Add CSC data to counters
  vector<Counter> getHitCounters(Track t); //Return list of counters in a track
  double calcAvgPE_per_cm(); //return mean(pe/cm) of all counters
  TH1F *makePEHist();
  TH1F *makePE_per_cm_Hist();
  void plotAllPEHists(); //Plot PE hists of each counter
  void plotAllPE_per_cm_Hists();
  void displayEvents(); //Calls event display class

 private:
  string ID;
  double offset;
  Counter counters;
  vector<CRVEvent> crvEvents;
  vector<EventData> cscEvents;
};

#endif
