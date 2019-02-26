//Class to store a CRV hit

#ifndef _HIT_H
#define HIT_H

#include <CRVEvent>

class Hit{
 public:
  Hit();
  Hit(CRVEvent,double pathL);
  ~Hit();
  double get_PEyield();
  double get_PathLength(); //In mm
  double get_PE_per_cm();
  
 private:
  int trigNum; //Trigger number corresponding to the hit
  double PEyield; //Number of Photo electrons detected
  double pathLength; //Path length of muon track through a counter in mm
  double calcPEyield(CRVEvent); //Calculate PE yield of hit
};

#endif
