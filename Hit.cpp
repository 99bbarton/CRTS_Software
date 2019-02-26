//Implementation of the Hit class to store a hit of a single counter in the CRV

#include "Hit.h"

//Default constructor - do nothing
Hit::Hit(){}

//@TODO
//Extract data from crvEvent and initialize fields
Hit::Hit(CRVEvent crvEvent, double pathL)
{
  trigNum  = -1; //@TODO
  pathLength = pathL;
  PEyield = calcPEyield(crvEvent);
}

//Destructor - no destruction necessary
Hit::~Hit(){}

//Return PE yield of hit
double Hit::get_PEyield()
{
  return PEyield;
}

//Return path length of muon track through counter in mm
double Hit::get_PathLength()
{
  return pathLength;
}

//Return PE yield / path length in cm
double Hit::get_PE_per_cm();
{
  return PEyield / (pathLength / 10);
}

//@TODO
//Calculate the PE yield of the hit
double calcPEyield(CRVEvent)
{
  //@TODO
  return -1;
}
