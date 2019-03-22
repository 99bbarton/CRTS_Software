//Implementation of Counter class representing a CRV counter

#include "Counter.h"

using namespace std;


//Default constructor - do nothing
Counter::Counter(){}

//Primary constructor - initialize fields
Counter::Counter(double xMin, double zMin, int layer, int posInLayer, int id, int diSN)
{
  this.xMin = xMin;
  this.zMin = zMin;
  this.layer = layer;
  this.posInLayer = posInLayer;
  this.id = id;
  dicounterSN = diSN;
  hits = new map<int,Hit>();
}

//Destructor 
Counter::~Counter(){
  delete hits;
}

int Counter::get_xMin() const{
  return xMin;
}

int Counter::get_xMax() const{
  return xMin + COUNTER_WIDTH;
}

int Counter::get_zMin() const{
  return zMin;
}

int Counter::get_zMax() const{
  return zMin + COUNTER_HEIGHT;
}

int Counter::get_layer() const{
  return layer
}

int Counter::get_posInLayer() const{
  return posInLayer;
}

int Counter::getDiCounterSN() const{
  return dicounterSN;
}

string Counter::getID() const{
  return id;
}

map<int,Hit> Counter::getHits() const{
  return hits;
}

//@TODO Implement with actual CRV Event class
//Add a hit to the counter's record
bool Counter::addHit(CRVEvent crvEv, Track track)
{
  Hit hit = new Hit(crvEV, calcPathLength(track));
  pair<int,Hit> trig_hit (crvEV.trigNum, hit);

  pair<map<int,Hit>::iterator,bool> insertResult;
  insertResult = hits.insert(trig_hit);

  return insertResult.second;
}


//Calculate the path length of a muon through the counter using a track
double Counter::calcPathLength(Track track) const
{
  if(!track.is_valid()) //If not a valid track, return a negative number
    return -1;
  
  int incidCase = trackIncidenceCase(track); //Get how the track passed through the counter
  if (incidCase == -1) //If track doesn't pass through counter
    return 0;


  //Calculate the bounds of the track in the counter
  double xLow = 0;
  double xHigh = 0;
  double yLow = 0;
  double yHigh = 0;
  double zLow = 0;
  double zHigh = 0;
  double xMax = get_xMax();
  double zMax = get_zMax();
  double x0 = track.x0();
  double dxdz = track.dxdz();

  switch (incidCase)
    {
    case 0: //Top and bottom
      zLow = zMin;
      zHigh = zMax;
      xLow = track.x(zMin); //This will be backwards in the case of negative slopes but distance will be same so ignore
      xHigh = track.y(zMax);
      yLow = track.y(zLow);
      yHigh = track.y(zHigh);
      break;
    case 1: //Upper left corner
      zLow = zMin + (1.0 / dxdz * xMin);
      zHigh = zMax;
      xLow = xMin;
      xHigh = track.x(zMax);   
      yLow = track.y(zLow);
      yHigh = track.y(zHigh);
      break;
    case 2: //Upper right corner
      zLow = zMax + (1.0 / dxdz * xMax); //Neg slope will subtract from zMax
      zHigh = zMax;
      xLow = track.x(zMax);
      xHigh = xMax;
      yLow = track.y(zLow);
      yHigh = track.y(zHigh);
      break;
    case 3: //Lower left corner
      zLow = zMin;
      zHigh = zMax + (1.0 / dxdz * xMin); //Neg slope will subtract from zMax
      xLow = xMin;
      xHigh = track.x(zMin);
      yLow = track.y(zLow);
      yHigh = track.y(zHigh);
      break;
    case 4: //Lower right corner
      zLow = zMin;
      zHigh = zMin + (1.0 / dxdz * xMax);
      xLow = track.x(zMin);
      xHigh = xMax;
      yLow = track.y(zLow);
      yHigh = track.y(zHigh);
      break;
    default:
      return -1;    
    }

  return sqrt(pow(xHigh - xLow, 2) + pow(yHigh - yLow, 2) + pow(zHigh - zLow, 2));
}

/*
  Returns an int corresponding to the way a track passes through a counter
  @return
  -2 if track is invalid
  -1 if track does not pass through counter
  0 if the track passes through the upper and lower z planes of the counter
  1 if the track passes through the upper left corner (left-> lower x) (i.e. left side and top)
  2 if the track passes through the upper right corner (i.e. right side and top)
  3 if the track passes through the lower left corner (i.e. left side and bottom)
  4 if the track passes through the upper right corner (i.e. right side and bottom)
 */
int Counter::trackIncidenceCase(Track t)
{
  if(!t.is_valid()) //If not a valid track
    return -2;

  double zMax = get_zMax();
  double xMax = get_xMax();

  bool bottomHit = (t.x(zMin) >= xMin) && (t.x(zMin) <= xMax); //True if track passes through zMin (bottom) plane of counter
  bool topHit = (t.x(zMax) >= xMin) && (t.x(zMax) <= xMax); //True if track passes through zMax (top) plane of counter
  bool rightHit = (t.x(zMin) > xMax); //True if track passes through right (xMax) side
  bool leftHit = (t.x(zMin) < xMin); //True if track passes through left (xMin) side

  if (topHit && bottomHit)
    return 0;
  else if (topHit && leftHit)
    return 1;
  else if (topHit && rightHit)
    return 2;
  else if (bottomHit && leftHit)
    return 3;
  else if (bottomHit && rightHit)
    return 4;
  else //Track did not intersect couter
    return -1; 
}


/*
  Calculate the average PE yield per cm for all of the hits
 */
double Counter::calcAvg_PE_per_cm() const
{
  double avg = 0;
  Hit thisHit;
  double pathLen_cm;
  double pe;

  for (std::map<int,Hit>::iterator it = hits.begin(); it != hits.end(); ++it)
    {
      thisHit = it->second;
      if (thisHit.get_PathLength() > 0)
	avg += thisHit.get_PEyield() / thisHit.get_PathLength() * 10; //Convert to cm from mm
    }

  return avg;
}


/*
  Return a pointer to a histogram filled with PE yield from all of the hits
 */
TH1F* Counter::buildPE_Hist() const
{
  TH1F *hist = new TH1F("peHist:Counter=" + id, "PE Yield : Counter = " + id, 100, 0, 100);
  hist->SetXTitle("PE Yield");
  Hit thisHit;

  for (std::map<int,Hit>::iterator it = hits.begin(); it != hits.end(); ++it)
    {
      thisHit = it->second;
      hist->Fill(thisHit.get_PEyield());
    }

  return hist;
}


/*
  Return pointer to histogram filled with PE yield per cm of all hits
 */
TH1F* Counter::buildPE_per_cm_Hist() const
{
  TH1F *hist = new TH1F("PE_per_cm_Hist:Counter=" + id, "PE Yield per cm : Counter = " + id, 100, 0, 100);
  hist->SetXTitle("PE Yield per cm");
  Hit thisHit;

  for (std::map<int,Hit>::iterator it = hits.begin(); it != hits.end(); ++it)
    {
      thisHit = it->second;
      if (thisHit.get_PathLength() > 0)
	hist->Fill(thisHit.get_PEyield() / thisHit.get_PathLength() * 10); //Convert to cm from mm
    }

  return hist;
}
