//Class to store a CRV hit

#ifndef _HIT_H
#define HIT_H

#include <CRVEvent>
#include "Track.h"

class Hit{
 public:
  Hit();
  Hit(CRVEvent,double pathL);
  ~Hit();
  double get_PEyield();
  double get_PathLength();
  double get_PE_per_cm();
  
 private:
  double PEyield; //Number of Photo electrons detected
  double pathLength; //Path length of muon track through a counter in m
  double calcPEyield(); //Calculate PE yield of hit
};

#endif
