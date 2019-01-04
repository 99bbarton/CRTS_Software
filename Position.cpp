#include "Position.h"

#include "Constants.h"

#include <cassert>
#include <iostream>



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
	//TODO: Map channel to test stand coordinates
	double position = 0.1;
	if (csc_ == 0)
		position = SPACING + SPACING2;
	else if (csc_ == 1)
		position = SPACING2;
	else if (csc_ == 2)
		if (layer_ == 3)
			position = 0.1;
		else
			position = 100;
	return position;
}
