#pragma once
#include <vector>
#include "Constants.h"
#include "Position.h"
#include "Track.h"
#include "TGraph.h"


class EventData
{
private:
	std::vector<Position> position_list; //list of positions
	Track* eventTrack; //track for these positions
	bool initializeTrack();
	bool goodTrack_;
	friend class ReadTree;

public:
	EventData();
	EventData(const EventData&);
	void channelHit(int, int, int); //flag a channel as hit on a given board
	void copyData(const EventData&);
	int* getChannelsForBoard(int); //get the array for a given board
	std::vector<Position> getPositionList() const; //get the position list for the event
	int getLayer(int) const; //get the layer for a given board
	int getCSC(int) const; //get the chamber for a given board
	int getSide(int) const; //get the side for a given board
	double get_mapped_pos(int) const;
	void buildPositionList();
	int triggerCount() const;
	bool goodTrack() const;
	Track* getTrack() const;
	//	TGraph* getTimingGraph() const;

	int triggerCount_;
	int channels[NUM_BOARDS][NUM_CHANS];

	//Struct for processed-data tree
	struct ProcessedEvent
	{
	  int triggerCount;
	  int channelHits[NUM_BOARDS][NUM_CHANS];
	};

};
