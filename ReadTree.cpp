//compile independently with: g++ -std=c++11 -o ReadTree ReadTree.cpp EventData.cpp `root-config --cflags --glibs`

#include <iostream>
#include <string>
#include <cmath> ////////////////////////////////////////////////////
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

using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
  if (argc < 2 || argc > 3)
	{
		cout << "Usage: ReadTree [inputFilename] <voltage>" << endl;
		return 0;
	}

	string inputFileName = argv[1];
	cout << "File: " << inputFileName << endl;

	double voltage = 0;
	if (argc > 2)
	  {
	    voltage = atof(argv[2]);
	  }

	TApplication *thisApp = new TApplication("ReadTree", &argc, argv);


		  
	ReadTree processor(thisApp, inputFileName, voltage);

	return 1;
}

ReadTree::ReadTree(TApplication* app, TString fileName, double voltage = 0)
{
	theApp = app;
	TFile *root_file = new TFile(fileName.Data(), "READ");
	if (root_file != NULL) //If we opened the file
	{
		vmm_tree = (TTree*) root_file->Get("CSC_Data"); //Try and get the tree from it
		timeHist = (TH1I*) root_file->Get("timingHist");
		csc1tHist = (TH1I*) root_file->Get("csc1timingHist");
		csc2tHist = (TH1I*) root_file->Get("csc2timingHist");
		if (getData(vmm_tree))
		{
		  if(timeHist != NULL)
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
		      
		      if (csc2tHist != NULL)
			{
			  csc2tHist->SetTitle("CSC #2 Timing");
			  csc2tHist->SetXTitle("Hit BCID - Trigger BCID [nano sec]");
			  csc2tHist->SetYTitle("Count");
			  csc2tHist->SetLineColor(kBlue);
			  timeCanv->cd(4);
			  csc2tHist->Draw();
			  timeCanv->cd(2);
			  csc2tHist->Draw("same");
			}
		      
		      timeCanv->cd(1);
		      timeHist->Draw();
		      timeCanv->cd(2);
		      // timeCanv->BuildLegend();
		      timeCanv->Update();
		      timeCanv->Print("timing.pdf");
		    }
		  
		  if (voltage > 0)
		    calcBoardEfficiencies(voltage);

		  makeOccupancyPlots();

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


  occCanv->Print("occupancies.pdf");
}



//Returns an array of the efficiency that a hit was registerd in each plane of the cscs when a track was constructed
void ReadTree::calcLayerTrackEfficiencies(double efficiencies[])
{
  int numPlanes = 12; //3 CSCs * 4 planes/csc
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