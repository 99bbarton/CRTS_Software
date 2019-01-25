//compile independently with: g++ -std=c++11 -o ParseBinary ParseBinary.cpp EventData.cpp `root-config --cflags --glibs`

#include "ProcessBinary.h"

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <arpa/inet.h>
#include <vector>
//#include <WinSock2.h>

#include "TROOT.h"			//<TROOT.h>
#include "TFile.h"			//<TFile.h>
#include "TTree.h"			//<TTree.h>
#include "TApplication.h"	//<TApplication.h>
#include "TSystem.h"		//<TSystem.h>
#include "TCanvas.h"
#include "TH1I.h"


using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "Usage: parseBinaryFile [inputFilename]" << endl;
		return 0;
	}

	string inputFileName = argv[1];
	cout << "File: " << inputFileName << endl;
	TApplication *thisApp = new TApplication("ProcessBinary", &argc, argv);

	ProcessBinary processor(thisApp, inputFileName);

	return 1;
}


ProcessBinary::ProcessBinary()
{
	//do nothing I guess
}


ProcessBinary::ProcessBinary(TApplication* app, string inputFileName)
{
	file.open(inputFileName, std::ifstream::in | std::ifstream::binary);
	if (file.fail())
	{
		cout << "Cannot open file." << endl;
		gSystem->Exit(-1);
	}
	else
		parseFile();
}


ProcessBinary::~ProcessBinary()
{
	gSystem->Exit(1); //Allow root to kill everything
}


bool ProcessBinary::parseFile()
{
	file.seekg(0, file.end); //Go to the end of the file
	unsigned int size_in_bytes = file.tellg(); //Store were the end of the file (tells us how many bytes the file is)
	file.seekg(0, file.beg); //Reset the 'cursor' to the beginning of the file

	if (size_in_bytes <= 16)
	{
		cout << "Empty file!" << endl;
		return false;
	}

	char* buff = new char[size_in_bytes]; //define a new character buffer which holds the file in memory
	uint32_t *buff_uint32 = (uint32_t*)buff; //16-bit words

	file.read(buff, size_in_bytes); //read everything from the file into the buffer

	if (file)
		cout << "File read successfully." << endl;
	else
	{
		cout << "ERR: only " << file.gcount() << " bytes read!" << endl;
		return false;
	}

	file.close();

	unsigned int sizeInUint32 = size_in_bytes / 4;
	uint32_t packetHeader = 0x00A8C0; //packet header marker
	uint32_t packetFooter = 0xFFFFFFFF; //packet footer marker

	std::vector<unsigned int> headerPos;
	for (unsigned int pos = 0; pos < sizeInUint32; pos++)
	{
		if ((htonl(buff_uint32[pos]) & 0xFFFFFF) == packetHeader)
			headerPos.push_back(pos);
	}

	if (headerPos.size() == 0)
	{
		cout << "Found no packet headers." << endl;
		return false;
	}

	TH1I *timingHist = new TH1I("timingHist", "Timing",  50, -25, 25);  
	TH1I *c1TimingHist = new TH1I("csc1timingHist", "CSC#1 Timing", 50, -25, 25);
	TH1I *c0TimingHist = new TH1I("csc0timingHist", "CSC#0 Timing", 50, -25, 25);
	TH1I *c3TimingHist = new TH1I("csc3timingHist", "CSC#3 Timing", 50, -25, 25);

	//Loop over each packet in the buffer, pulling out the relevant information
	unsigned int lastSubRun = 0;
	for (unsigned int headerNum = 0; headerNum < headerPos.size(); headerNum++)
	{
		EventData* dEvent;

		unsigned int basePos = headerPos.at(headerNum);
		if (basePos >= sizeInUint32) //if the base position from which we will start reading exceeds the buffer length, skip
			continue;

		unsigned int endPos = sizeInUint32 - 1; //define the end-read position
		if (headerNum < headerPos.size() - 1) //if the header number is before the end of the list of header positions
			endPos = headerPos.at(headerNum + 1) - 1; //then the end position must be just before the next header
		if (endPos >= sizeInUint32) //if the end position exceeds the buffer length, skip
			continue;

		uint32_t headerWord0 = (uint32_t) htonl(buff_uint32[basePos + 0]);//Bits 31-24 Board ID, 23-0 Header Token = 0x00A8C0
		uint32_t headerWord1 = (uint32_t) htonl(buff_uint32[basePos + 1]); //No significant bits
		uint32_t headerWord2 = (uint32_t) htonl(buff_uint32[basePos + 2]); //Bits 31-0 Trigger Count
		uint32_t headerWord3 = (uint32_t) htonl(buff_uint32[basePos + 3]); //Bits 23-8 Trigger BCID
		uint32_t boardId = (headerWord0 & 0xFF000000) >> 24; //Shift by 24 bits right
		uint32_t triggerCount = (headerWord2 & 0xFFFFFFFF);
		uint32_t triggerBcid = (headerWord3 & 0xFFFF00) >> 8; //Shift by 8 bits right (BCID associated with trigger)
		//uint32_t triggerDecode = decodeGray(triggerBcid); //(BCID associated with trigger)

		if (triggerMap.count(triggerCount/*triggerDecode*/)) //If the trigger count already exists in the map, we will update the EventData associated with it
		{
			dEvent = new EventData(triggerMap.at(triggerCount/*triggerDecode*/)); //use copy constructor
			triggerMap.erase(triggerCount); //erase the entry since we will be pushing another with updated values
		}
		else //Otherwise we will need to create a new EventData object and associate it with the trigger count, placing it in the map
		{
			dEvent = new EventData();
		}

		
		for (unsigned int line = basePos + 4; line <= endPos; line+=2)
		{

		
			uint32_t hitWord0 = (uint32_t)htonl(buff_uint32[line + 0]); //Bits 29-22 TDO, 21-10 BCID in gray code, 9-0 PDO
			uint32_t hitWord1 = (uint32_t)htonl(buff_uint32[line + 1]); //Bits 7-2 Channel, 1 Threshold, 0 Flag
			if (hitWord0 == packetFooter || hitWord1 == packetFooter) //Footer token is 0xFFFFFFFF
				break; //Get out of the loop because we have reached the end of the event, there will be no more hits to read...

			uint32_t pdo = (hitWord0 & 0x3FF); //Bits 9-0 PDO
			uint32_t bcidGray = (hitWord0 & 0x3FFC00) >> 10; //Bits 21-10 BCID in gray, shift to right by 10 bits
			uint32_t bcidDecoded = decodeGray(bcidGray); //Decode BCID in gray
			uint32_t tdo = (hitWord0 & 0x3FC00000) >> 22; //Bits 29-22 TDO, shift to right by 22 bits
			uint32_t flag = (hitWord1 & 0x1); //Bit 0 Flag
			uint32_t threshold = (hitWord1 & 0x2) >> 1; //Bit 1 Threshold, shift to right by 1 bit
			uint32_t channel = (hitWord1 & 0xFC) >> 2; //Bits 7-2 Channel, shift to right by 2 bits

			if (channel < 0 || channel > 63){ cout << "Found unexpected channel number!" << endl; continue; }

			//Calculate time difference between hit and trigger and store in timing histogram corresbonding to board ID
			int trigTime = int(bcidDecoded) - int(triggerBcid);
			timingHist->Fill(trigTime);
			if (boardId >= BOARD_START_ID && boardId < (BOARD_START_ID + 8)) //First 8 boards are on CSC #1
			  {
			    c1TimingHist->Fill(trigTime);
			  }
			else if (boardId >= (BOARD_START_ID + 8) && boardId < (BOARD_START_ID + 16))
			  {
			    c0TimingHist->Fill(trigTime);
			  }

			//confirm that the hit falls within the time for cosmic hits
			if (trigTime > TIME_CUT_LOW && trigTime < TIME_CUT_HIGH)
			  {
			    dEvent->channelHit(channel, pdo, boardId);  //If it did, then we will flag that channel as hit for that trigger (filled with pdo)
			  }	
		}
		

		std::pair<int, EventData> eventPair(triggerCount, *dEvent);
		triggerMap.insert(eventPair);
		delete dEvent;
	}

	
	

	TFile* output = new TFile("processedBinary.root", "RECREATE"); //create root file
	TTree *proc_tree = new TTree("CSC_Data", "Stage 1 processed CSC event data");
	EventData::ProcessedEvent proc_event;
	proc_tree->Branch("triggerCount", &proc_event.triggerCount, "TriggerCount/I"); //create a branch for the trigger count
	char branch_buff[20];
	for (int b = 0; b < NUM_BOARDS; b++)
	{
		sprintf(branch_buff, "chans_board%d", b);
		string leafList = branch_buff; 
		leafList.append("[64]/I");
		proc_tree->Branch(branch_buff, &proc_event.channelHits[b], leafList.c_str()); //create a branch for the collection of 64 channels of each board
	}
	
	
	for (std::map<int, EventData>::iterator ev = triggerMap.begin(); ev != triggerMap.end(); ++ev)
	{
		bool atLeastOneHit = false;
		proc_event.triggerCount = ev->first; //copy over the trigger count
		for(int b = 0; b < NUM_BOARDS; b++) //copy over the hit channel PDO
			for(int c = 0; c < NUM_CHANS; c++)
			{
				proc_event.channelHits[b][c] = ev->second.channels[b][c];
				if(proc_event.channelHits[b][c] > 0)
					atLeastOneHit = true;
			}
		if(atLeastOneHit)
			proc_tree->Fill(); //fill the tree
	}

	timingHist->Write();
	c1TimingHist->Write();
	c0TimingHist->Write();
	proc_tree->Write(); //save everything to the file
	
      
	output->Write();
	output->Close();

}

uint32_t ProcessBinary::decodeGray(uint32_t gray)
{
	uint32_t mask;
	for (mask = gray >> 1; mask != 0; mask = mask >> 1)
		gray = gray ^ mask;

	return gray;
}
