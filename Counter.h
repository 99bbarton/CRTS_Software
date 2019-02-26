//Class for representing a CRV counter

#ifndef _COUNTER_H
#define _COUNTER_H

#include <string>
#include <map>

#include <TH1F.h>

#include "Track.h"

class Counter{
 public:
  Counter();
  Counter(int xMin, int zMin, int layer, int posInLayer);
  ~Counter();
  int get_xMin() const;
  int get_xMax() const;
  int get_zMin() const;
  int get_zMax() const;
  int get_layer() const;
  int get_posInLayer() const;
  string getID() const;
  int getDicounterSN() const;
  map<int,Hit> getHits() const;
  bool addHit(CRVEvent, Track); //Add a hit to the counter's record
  double calcPathLength(Track) const; //Calc path length through counter
  double calcAvg_PE_per_cm(); //Return avg PE/cm over all hits
  TH1F* buildPE_Hist();  //Make a histogram of PE yield
  TH1F* buildPE_per_cm_Hist(); //Make a histogram of PE yield per cm
  
 private:
  int xMin; //x coord of edge of counter closer to stand origin
  int zMin; //z coord of bottom of counter
  int layer; //0-3, bottom layer = 0
  int posInLayer; //0-15
  map<int trigNum, Hit hit> hits; //Stores counter readout for each trigger
  int ID; //Extrusion id e.g. 1111
  int dicounterSN; //Serial number of parent dicounter
  

};

#endif
