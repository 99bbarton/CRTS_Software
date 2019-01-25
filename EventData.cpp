#include "EventData.h"
#include "Position.h"
#include <cmath>

//Default Constructor, initializes the channel bits for each board
EventData::EventData()
{
	goodTrack_ = false;
	triggerCount_ = 0;
	for (int board = 0; board < NUM_BOARDS; board++)
		for(int chan = 0; chan < NUM_CHANS; chan++)
			channels[board][chan] = 0; //initialize all the values in the array to 0
}

//Copy Constructor, copies data over from another EventData objects
EventData::EventData(const EventData& eventToCopy)
{
	copyData(eventToCopy);
}

//Marks individual channels on each board as 'hit'
void EventData::channelHit(int channel, int pdo, int boardID)
{
	channels[boardID % BOARD_START_ID][channel] = pdo;
}

//Copy array data from another EventData object
void EventData::copyData(const EventData& eventToCopy)
{
	goodTrack_ = eventToCopy.goodTrack_;
	triggerCount_ = eventToCopy.triggerCount_;
	for (int board = 0; board < NUM_BOARDS; board++)
		for (int chan = 0; chan < NUM_CHANS; chan++)
			channels[board][chan] = eventToCopy.channels[board][chan]; //copy over the values from the other event
	
	for(int i = 0; i < eventToCopy.position_list.size(); i++)
		position_list.push_back(eventToCopy.position_list[i]);
}

int* EventData::getChannelsForBoard(int boardID)
{
	return channels[boardID % BOARD_START_ID];
}

std::vector<Position> EventData::getPositionList() const
{
	return position_list;
}

//Get the layer for a given board ID
int EventData::getLayer(int boardID) const
{
int bid = boardID % BOARD_START_ID;
	int layer = -1;
	switch (bid)
	{
		//CSC Top
	case 0:
	case 1:
		//CSC Bottom
	case 8:
	case 9:
		layer = 3;
		break;

		//CSC Top
	case 2:
	case 3:
		//CSC Bottom
	case 10:
	case 11:
		layer = 2;
		break;

		//CSC Top
	case 4:
	case 5:
		//CSC Bottom
	case 12:
	case 13:
		layer = 1;
		break;

		//CSC Top
	case 6:
	case 7:
		//CSC Bottom
	case 14:
	case 15:
		layer = 0;
		break;
	default:
		layer = -1;
	}

	return layer;
}

int EventData::getCSC(int boardID) const
{
	int bid = boardID % BOARD_START_ID;
	int csc = -1;
	switch (bid)
	{
		//CSC Top
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		csc = 1;
		break;
		//CSC Bottom
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		csc = 0;
		break;
	default:
		csc = -1;
	}

	return csc;
}

int EventData::getSide(int boardID) const
{
	int bid = boardID % BOARD_START_ID;
	int side = -1;
	switch (bid)
	{
		//x-side
	case 0:
	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	case 14:
		side = 0;
		break;
		//y-side
	case 1:
	case 3:
	case 5:
	case 7:
	case 9:
	case 11:
	case 13:
	case 15:
		side = 1;
		break;
	default:
		side = -1;
	}

	return side;
}

//Get the position of a strip, relative to the center of the chamber
double EventData::get_mapped_pos(int channel) const
{
	double position = ((channel - CHAN_MID_LOW) * (CATHODE_WIDTH + CATHODE_GAP)) + (std::copysign(1.0, channel - CHAN_MID_LOW) * (CATHODE_MID_GAP + CATHODE_CENTER));
	if (position < -20 || position > 20)
		return -99;
	else
		return position;
}

void EventData::buildPositionList()
{
	position_list.clear();
	//Need to loop through each board on the same layer of the same chamber and pair up the hit channels
	for (int b = 0; b < NUM_BOARDS - 1; b+=2) //0,1 -> 2,3 -> 4,5 (pairs on each layer)
	{
		for (int c1 = 0; c1 < NUM_CHANS; c1++) //channels for the first board
		{
			for (int c2 = 0; c2 < NUM_CHANS; c2++) //channels for the second board
			{
				if (channels[b][c1] > 0 && channels[b + 1][c2] > 0) //if there is positive PDO on both channels, must be a hit
					position_list.emplace_back(getCSC(b), getLayer(b), c1, c2);//channels[b][c1], channels[b + 1][c2]);
					//TODO: Update position information to also contain PDO
			}
		}
	}
	goodTrack_ = initializeTrack();
}

bool EventData::initializeTrack()
{
	eventTrack = new Track(position_list);
	return eventTrack->is_valid();
}

int EventData::triggerCount() const
{
	return triggerCount_;
}

bool EventData::goodTrack() const
{
	return goodTrack_;
}

Track* EventData::getTrack() const
{
	return eventTrack;
}


