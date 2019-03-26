//Class for representing a CRV counter

#ifndef _COUNTER_H
#define _COUNTER_H

#include <string>
#include <map>

#include <TH1F.h>

#include "Track.h"
#include "Hit.h"

class Counter{
 public:
  Counter();
  Counter(double xMin, double zMin, int layer, int posInLayer, int ID,int diSN);
  ~Counter();
  double get_xMin() const;
  double get_xMax() const;
  double get_zMin() const;
  double get_zMax() const;
  int get_layer() const;
  int get_posInLayer() const;
  string get_ID() const;
  int get_DicounterSN() const;
  map<int,Hit> get_hitMap() const;
  vector<Hit> get_hits() const;
  bool addHit(CRVEvent, Track); //Add a hit to the counter's record
  double calcPathLength(Track) const; //Calc path length through counter
  double calcAvg_PE_per_cm() const; //Return avg PE/cm over all hits
  TH1F* buildPE_Hist() const;  //Make a histogram of PE yield
  TH1F* buildPE_per_cm_Hist() const; //Make a histogram of PE yield per cm
  
 private:
  double xMin; //x coord of edge of counter closer to stand origin
  double zMin; //z coord of bottom of counter
  int layer; //0-3, bottom layer = 0
  int posInLayer; //0-15
  map<int,Hit> hitMap; //Stores counter readout for each trigger
  vector<Hit> hits;
  int id; //Extrusion id e.g. 1111
  int dicounterSN; //Serial number of parent dicounter
  int trackIncidenceCase(Track) const; //Return an int corresponding to the way a track hits a counter
  

};

#endif
