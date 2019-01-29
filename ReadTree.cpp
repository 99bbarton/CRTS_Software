//compile independently with: g++ -std=c++11 -o ReadTree ReadTree.cpp EventData.cpp `root-config --cflags --glibs`

#include <iostream>
#include <string>
#include <cmath> 
#include <cstdio>

#include "ReadTree.h"
#include "Track.h"
#include "Position.h"

#include "TROOT.h"			//<TROOT.h>
#include "TFile.h"			//<TFile.h>
#include "TTree.h"			//<TTree.h>
#include "TApplication.h"	//<TApplication.h>
#include "TSystem.h"		//<TSystem.h>
#include "TCanvas.h"		//<TCanvas.h>
#include "TGraph2D.h"		//<TGraph2D.h>
#include "TPolyLine3D.h"	//<TPolyLine3D.h>
#include "TGraph.h"
#include "TString.h"
#include "TH2F.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
  if (argc < 2 || argc > 4)
	{
		cout << "Usage: ReadTree [inputFilename] <options>" << endl;
		cout << "Allowed options: " << endl;
		cout << "-t : Create CSC timing histograms - written to \"timings.png\"" << endl;
		cout << "-r : Create residuals plots - written to \"residuals.png\"" << endl;
		cout << "-o : Creat occupancy plots for each board - written to \"occupancies.png\"" << endl;
		cout << "-e <voltage> : Calculate per-board efficiencies - written to \"efficiencies.txt\"" << endl;
		exit(-1);
	}

	string inputFileName = argv[1];
	cout << "File: " << inputFileName << endl;

	//Options variables - default to false (0)
	bool effs = false, timing = false, resids = false, occs = false;
	bool options[4];
	double voltage;
	int op;
	while((op = getopt(argc, argv, "tTrRoOe:E:")) != -1)
	  {
	    switch (op)
	      {
	      case 't':
		timing = true;
		break;
	      case 'T':
		timing = true;
		break;
	      case 'r':
		resids = true;
		break;
	      case 'R':
		resids = true;
		break;
	      case 'o':
		occs = true;
		break;
	      case 'O':
		occs = true;
		break;
	      case 'e':
		effs = true;
		voltage = atof(optarg);
		break;
	      case 'E':
		effs = true;
		voltage = atof(optarg);
		break;
	      case '?':
		if (optopt == 'e' || optopt == 'E')
		  {
		    cout << "Additional voltage argument is required. e.g: $ReadTree dataFile.root -e 3000.0" << endl;
		    return 1;
		  }
	      default:
		cout << "Unrecognized option. Run ReadTree with no arguments for list of accepted options" << endl;
		return 1;
	      }
	  }
	options[0] = timing;
	options[1] = resids;
	options[2] = occs;
	options[3] = effs;
	

	TApplication *thisApp = new TApplication("ReadTree", &argc, argv);


		  
	ReadTree processor(thisApp, inputFileName, options, voltage);

	return 1;
}

ReadTree::ReadTree(TApplication* app, TString fileName, bool options[], double voltage = 0)
{
	theApp = app;
	TFile *root_file = new TFile(fileName.Data(), "READ");
	if (root_file != NULL) //If we opened the file
	{
		vmm_tree = (TTree*) root_file->Get("CSC_Data"); //Try and get the tree from it
		timeHist = (TH1I*) root_file->Get("timingHist");
		csc1tHist = (TH1I*) root_file->Get("csc1timingHist");
		csc0tHist = (TH1I*) root_file->Get("csc0timingHist");
		if (getData(vmm_tree))
		{
		  if(timeHist != NULL && options[0]) //If timing option is specified and histogram exists
		    plotTimingHists(timeHist, csc1tHist, csc0tHist);
	       
		  if (options[1]) // If residuals options is specified
		    plotResiduals();

		  if (options[2]) //If occupancies option is specified
		    makeOccupancyPlots();
		    
		  if (options[3] && voltage > 0) //If efficiencies option is selected and valid voltage supplied
		    calcBoardEfficiencies(voltage);
		  
		  drawTracks();
		  root_file->Close();
		
		}
		else
		{
			std::cout << "Cannot find CSC_Data tree!" << std::endl;
			root_file->Close();
		}
	}

	gSystem->Exit(-1);

}

ReadTree::~ReadTree()
{
	eventList.clear();
	gSystem->Exit(-1);
}

bool ReadTree::getData(TTree* dataTree)
{
	if (dataTree != NULL) //doesn't hurt to check that the pointer provided is actually set
	{
		//dataTree->SetBranchAddress("hits", &pEvent, &b_hits);
	  dataTree->SetBranchAddress("triggerCount", &dEvent.triggerCount_, &b_triggerCount);
 		char buff[14];
		for (int b = 0; b < NUM_BOARDS; b++) //Set the branch addresses
		{
			sprintf(buff, "chans_board%d", b);
			dataTree->SetBranchAddress(buff, &dEvent.channels[b], &b_chans_board[b]);
		}

		int numEntries = dataTree->GetEntriesFast();
		for (int iEvent = 0; iEvent < numEntries; iEvent++)
		{
			dataTree->GetEntry(iEvent);
			eventList.push_back(dEvent); //Push_back will create a copy of pEvent and put it into the vector
			//std::cout << "Wrote : " << iEvent << " " << eventList[iEvent].triggerCount << std::endl;
		}

		for(int i = 0; i < eventList.size(); i++)
		{
			eventList[i].buildPositionList();
		}
		return true;
	}
	else
		return false;
}


void ReadTree::drawTracks()
{
	TCanvas *c1 = new TCanvas("TrackCanvas", "Tracks", 800, 800);

	TGraph2D *graph; // = new TGraph2D(); //Create new graph to fill with track points

	TPolyLine3D *line; //= new TPolyLine3D(2); //2 points

	char input;
	int trackNum = 0;
	std::cout << "Press RETURN to draw next track, any other key to exit." << std::endl;
	//bool tracks[3] = {false, false, false};
	do 
	{
		//do
		//{
			graph = new TGraph2D();
			graph->SetPoint(0, -1, -1, 1);
			graph->SetPoint(1, 64, 64, SPACING + SPACING2);
			line = new TPolyLine3D(2);
			line->SetLineWidth(3);
			line->SetLineStyle(9);
			line->SetLineColor(kBlue);


			int pointCntr = 2;
			//if (eventList[trackNum].goodTrack())
			//{
				Track* eventTrack = eventList[trackNum].getTrack();
				std::vector<Position> positions = eventList[trackNum].getPositionList();
	
				if (eventTrack->is_valid())
				{
					line->SetLineColor(kBlue);
					line->SetPoint(0, eventTrack->x(0), eventTrack->y(0), 0);
					line->SetPoint(1, eventTrack->x(SPACING + SPACING2), eventTrack->y(SPACING + SPACING2), SPACING + SPACING2);
				}
				else
				{
					line->SetLineColor(kRed);
					line->SetPoint(0, 0, 0, 0);
					line->SetPoint(1, 0, 0, SPACING + SPACING2);
				}

				for (unsigned int pos = 0; pos < positions.size(); pos++) //put the positions into the graph
					graph->SetPoint(pointCntr, positions[pos].x(), positions[pos].y(), positions[pos].z());
			//}
			//tracks[0] = false; tracks[1] = false; tracks[2] = false;
			////if(graph != NULL) graph->Delete(); //Delete the old graph
			//graph = new TGraph2D(); //Create new graph to fill with track points
			//graph->SetPoint(0, -1,-1,1);
			//graph->SetPoint(1, 64,64,SPACING+SPACING2);
			//int pointCntr = 2;
			//for (int ch_0 = 0; ch_0 < NUM_CHANS; ch_0++) //Top chamber
			//{
			//	for (int ch_1 = 0; ch_1 < NUM_CHANS; ch_1++)
			//	{
			//		if (eventList[trackNum].channels[0][ch_0] > 0 && eventList[trackNum].channels[1][ch_1] > 0) //if the PDOs > 0, must be hit channels
			//		{
			//			tracks[0] = true;
			//			//graph->SetPoint(pointCntr++, dEvent.get_mapped_pos(ch_0), dEvent.get_mapped_pos(ch_1), SPACING);
			//			graph->SetPoint(pointCntr++, ch_0, ch_1, SPACING+SPACING2);
			//		}
			//	}
			//}
			//if(tracks[0])
			//{
			//	for (int ch_2 = 0; ch_2 < NUM_CHANS; ch_2++) //Middle chamber
			//	{
			//		for (int ch_3 = 0; ch_3 < NUM_CHANS; ch_3++)
			//		{
			//			if (eventList[trackNum].channels[2][ch_2] > 0 && eventList[trackNum].channels[3][ch_3] > 0) //if the PDOs > 0, must be hit channels
			//			{
			//				tracks[1] = true;
			//				//graph->SetPoint(pointCntr++, dEvent.get_mapped_pos(ch_2), dEvent.get_mapped_pos(ch_3), 1);
			//				graph->SetPoint(pointCntr++, ch_2, ch_3, SPACING2);
			//			}
			//		}
			//	}
			//	if(tracks[1])
			//	{
			//		for(int ch_4 = 0; ch_4 < NUM_CHANS; ch_4++) //Bottom Chamber
			//		{
			//			for(int ch_5 = 0; ch_5 < NUM_CHANS; ch_5++)
			//			{
			//				if(eventList[trackNum].channels[4][ch_4] > 0 && eventList[trackNum].channels[5][ch_5] > 0) // if the PDO of two crossing channels x,y is > 0
			//				{
			//					tracks[2] = true;
			//					graph->SetPoint(pointCntr++, ch_4, ch_5, 1);
			//				}
			//			}
			//		}
			//	}
			//}

			//trackNum++;

			//}while(!((tracks[0] && tracks[1]) && tracks[2]) && trackNum < eventList.size());
		//} while (/*!eventList[trackNum].goodTrack() &&*/ trackNum < eventList.size()); //while it is not a good track and we haven't reach the end of the tracks, keep looking for good tracks
		c1->cd();
		graph->SetMarkerStyle(20);
		graph->Draw("PCOL");
		line->Draw("SAME");
		c1->Update();
		gSystem->ProcessEvents();
		std::cout << "Track: " << trackNum << std::endl;
		trackNum++;
		std::cin.get(input);

	} while (input == '\n' && trackNum < eventList.size());
}


//Plot timing difference of CSC hits to triggers
void ReadTree::plotTimingHists(TH1I *timeHist, TH1I *csc1tHist, TH1I *csc0tHist)
{
  TCanvas *timeCanv = new TCanvas("timeCanv", "Timing", 1000,1000);
  timeCanv->Divide(2,2);

  timeHist->SetTitle("Timing Difference of Hits to Triggers");
  timeHist->SetXTitle("Hit BCID - Trigger BCID [nano sec]");
  timeHist->SetYTitle("Count");
  if(csc1tHist != NULL)
    {
      csc1tHist->SetTitle("CSC #1 Timing");
      csc1tHist->SetXTitle("Hit BCID - Trigger BCID [nano sec]");
      csc1tHist->SetYTitle("Count");
      csc1tHist->SetLineColor(kRed);
      timeCanv->cd(3);
      csc1tHist->Draw();
      timeCanv->cd(2);
      TH1I *copy = new TH1I(*csc1tHist);
      copy->SetTitle("Timing Difference of Hits to Triggers");
      copy->Draw();
    }
  
  if (csc0tHist != NULL)
    {
      csc0tHist->SetTitle("CSC #0 Timing");
      csc0tHist->SetXTitle("Hit BCID - Trigger BCID [nano sec]");
      csc0tHist->SetYTitle("Count");
      csc0tHist->SetLineColor(kBlue);
      timeCanv->cd(4);
      csc0tHist->Draw();
      timeCanv->cd(2);
      csc0tHist->Draw("same");
    }
  
  timeCanv->cd(1);
  timeHist->Draw();
  timeCanv->cd(2);
  // timeCanv->BuildLegend();
  timeCanv->Update();
  timeCanv->Print("timing.png");
}


//Calculate the efficiency of each board 
void ReadTree::calcBoardEfficiencies(double voltage)
{
  std::cout << "Calculating efficiencies..." << endl;

  int hitCounts[NUM_BOARDS] = {0};
  double efficiencies[NUM_BOARDS];
  EventData event;
  int numEvents = eventList.size();

  FILE *effsFile = fopen("efficiencies.txt", "a+");
  
  for (int e = 0; e < numEvents; e++) //For each event
    {
      event = eventList.at(e);

      for (int b = 0; b < NUM_BOARDS; b++) //Look for pdo > 0 on any channel for each board
	{
	  int *chans = event.getChannelsForBoard(b);

	  for (int c = 0; c < NUM_CHANS; c++)
	    {
	      
	      if (chans[c] > 0)
		{
		  hitCounts[b]++;
		  break; //If found a hit, skip to the next board for efficiencies sake
		}
	    }
	}
    }

  // fprintf(effsFile, "#Voltage\tboard 1 efficiency\t...\tboard 16 efficiency\taverage efficiency\n");
  fprintf(effsFile, "%4.1lf\t", voltage);
  double avgEff = 0;
  int hitBoardCount = 0;
  //Efficiency is simply the number of hits on the board / total number of events
  for (int b = 0; b < NUM_BOARDS; b++)
    {
      efficiencies[b] = double(hitCounts[b]) / numEvents;
      fprintf(effsFile, "%1.4lf\t", efficiencies[b]);
      avgEff += efficiencies[b];
      if (efficiencies[b] > 0)
	hitBoardCount++;
    }
  avgEff /= hitBoardCount;
  fprintf(effsFile, "%1.4lf\n", avgEff);

  fclose(effsFile);
}

//Make Occupancy plots for each board
void ReadTree::makeOccupancyPlots()
{
  TCanvas *occCanv = new TCanvas("occCanv","Occupancies",2000,2000);
  occCanv->Divide(4,4);

  TH1I *occs[NUM_BOARDS];
  TString name = "occ_board_";
  TString title = "Occupancy: Board #";
  TString boards[NUM_BOARDS] = {"100", "101", "102", "103", "104", "105", "106", "107", "108", "109", "110", "111", "112", "113", "114", "115"};

  //Make 16 histograms, one for each board
  for (int b = 0; b < NUM_BOARDS; b++)
    {
      TString boardID = TString(100 + b);
      occs[b] = new TH1I("occ_board_" + boards[b], "Occupancy: Board #" + boards[b], NUM_CHANS, 0.5, NUM_CHANS - 0.5);
    }

  EventData event;

  //Look for each channel that was hit on a given event, fill histograms with hit channels
  for (int evi = 0; evi < eventList.size(); evi++) 
    {
      event = eventList.at(evi);
      
      for (int b = 0; b < NUM_BOARDS; b++) //Look for pdo > 0 on each channel for each board
	{
	  int *chans = event.getChannelsForBoard(b);

	  for (int c = 0; c < NUM_CHANS; c++)
	    { 
	      if (chans[c] > 0)
		{
		  occs[b]->Fill(c);
		}
	    }
	}
    } 

  for (int h = 0; h < NUM_BOARDS; h++)
    {
      occs[h]->SetXTitle("Channel #");
      occs[h]->SetYTitle("Number of Hits");
      // occs[h]->SetNdivisions(NUM_CHANS/4);
      occCanv->cd(h+1);
      occs[h]->Draw("col");
    }


  occCanv->Print("occupancies.png");
}



//Returns an array of the efficiency that a hit was registerd in each plane of the cscs when a track was constructed
void ReadTree::calcLayerTrackEfficiencies(double efficiencies[])
{
  int numPlanes = 16; //2 CSCs * 4 planes/csc
  int nHits;
  int count;
  EventData event;
  Position pos;
  double pos_eps_cm = 1.0; /////////////////////////////// Radius of disk about track to search for a layer hit 


  for (int plane = 0; plane < numPlanes; plane++) //For each layer
    {
      nHits = 0;
      count = 0;
      
      for ( int eventi = 0; eventi < eventList.size(); eventi++) //For each event
	{
	  event = eventList.at(eventi);
	  Track *track = event.getTrack();
	  int numPos = event.position_list.size();
	  //if (numPos < 3 || !event.goodTrack()) //If not at least 3 hits or a good track, cannot use to calc efficiency
	      // continue;
	  
	  //Calculate the expected x and y coordinates at the z coordinate of the hit
	  double zPlane;
	  double xPred = track->x(zPlane);
	  double yPred = track->y(zPlane);
	  
	  int posN = 0;
	  bool notHit = true;
	  while(posN < numPos && notHit)
	    {
	      pos = event.position_list.at(posN);
	      
	      //If position is within radius to be acceptable "hit"
	      if (std::abs(xPred - pos.x()) <= pos_eps_cm && std::abs(yPred - pos.y()) <= pos_eps_cm)
		{
		  nHits++;
		  count++;
		}
	      else
		count++;
	    }
	}

      efficiencies[plane] = double(nHits) / count;  
    }
}


void ReadTree::plotResiduals()
{
  EventData event;
  double x, y, z;
  double xPred, yPred;
  int csc;
  Position pos;
  

  //Histograms to plot the deviation between theoretical (track-based) and actual (hits) positions
  TH2F *csc1Resids = new TH2F("csc1Devs","CSC #1 Abs(track pos - hit pos)",100, -5, 5, 100, -5, 5);
  TH2F *csc0Resids = new TH2F("csc2Devs","CSC #0 Abs(track pos - hit pos)",100, -5, 5, 100, -5, 5);

  for (int eventi = 0; eventi < eventList.size(); eventi++) //For each event
    {
      event = eventList.at(eventi);
      Track *track = event.getTrack();
      int numPos = event.position_list.size();
      if (numPos < 3 || !event.goodTrack()) //If not at least 3 hits or a good track, cannot use
      	continue;
	  
	  int posN = 0;
	  while(posN < numPos)
	    {
	      pos = event.position_list.at(posN);
	      csc = pos.csc();
	      x = pos.x();
	      y = pos.y();
	      z = pos.z();

	      xPred = track->x(z);
	      yPred = track->y(z);

	      if (csc == 0)
		csc0Resids->Fill(x - xPred, y - yPred);
	      else if (csc == 1)
		csc1Resids->Fill(x - xPred, y - yPred);

	      posN++;
	    }
    }

  TCanvas *residCanv = new TCanvas("residCanv","Residuals",1000,800);
  residCanv->Divide(2,1);
  csc1Resids->SetXTitle("x_hit - x_track [cm]");
  csc1Resids->SetYTitle("y_hit - x_track [cm]");
  csc0Resids->SetXTitle("x_hit - x_track [cm]");
  csc0Resids->SetYTitle("y_hit - y_track [cm]");
  residCanv->cd(1);
  csc1Resids->Draw("colz");
  residCanv->cd(2);
  csc0Resids->Draw("colz");
  
  residCanv->Print("residuals.png");
}
