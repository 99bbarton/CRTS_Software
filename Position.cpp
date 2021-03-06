#include "Position.h"

#include "Constants.h"

#include <cassert>
#include <iostream>
#include <cmath>



Position::Position()
{
}


//TODO: Update position information to also contain PDO

Position::Position(unsigned int csc_in, unsigned int layer_in, unsigned int x_hit_in, unsigned int y_hit_in) :
	csc_(csc_in), layer_(layer_in), x_hit_(x_hit_in), y_hit_(y_hit_in)
{
}

Position::Position(const Position& positionToCopy)
{
	csc_ = positionToCopy.csc_;
	layer_ = positionToCopy.layer_;
	x_hit_ = positionToCopy.x_hit_;
	y_hit_ = positionToCopy.y_hit_;
}

unsigned int Position::csc()
{
	return csc_;
}

unsigned int Position::layer()
{
	return layer_;
}

unsigned int Position::x_hit()
{
	return x_hit_;
}

unsigned int Position::y_hit()
{
	return y_hit_;
}


double Position::x_csc()
{
	//TODO: Map channel to CSC coordinates
	double position = x_hit_;
	return position;
}



double Position::y_csc()
{
	//TODO: Map channel to CSC coordinates
	double position = y_hit_;
	return position;
}



double Position::x()
{
	//TODO: Map channel to test stand coordinates
	double position = x_hit_;
	return position;
}



double Position::y()
{
	//TODO: Map channel to test stand coordinates
	double position = y_hit_;
	return position;
}


double Position::z()
{
	double position = (csc_ * SPACING) + (layer_ * (PANEL_THICKNESS + GAS_GAP_THICKNESS)); //bottom csc = 0, top csc = 1, layer 0 is at bottom, layer 3 is at top (z goes up from the bottom layer of the bottom csc)
	return position;
}


double Position::get_mapped_pos(int channel) const
{
	//TODO: fix this nonsense, seems to be off by 1 strip...
	double position = ((channel - CHAN_MID_LOW) * (CATHODE_WIDTH + CATHODE_GAP)) + (copysign(1.0, channel - CHAN_MID_LOW) * (CATHODE_MID_GAP + CATHODE_CENTER));
	if (position < -500 || position > 500)
		return -99;
	else
		return position;
}
