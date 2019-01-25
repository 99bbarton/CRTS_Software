#pragma once

#include "Constants.h"

class Position
{
public:
	Position();
	Position(unsigned int csc_in, unsigned int layer_in, unsigned int x_hit_in, unsigned int y_hit_in);
	Position(const Position& positionToCopy);

	//Chamber #, layer #, x,y channels
	unsigned int csc();
	unsigned int layer();
	unsigned int x_hit();
	unsigned int y_hit();

	//coordinates within the wire chamber
	double x_csc();
	double y_csc();

	//coordinates relative to the test stand origin
	double x();
	double y();
	double z();

private:
	unsigned int csc_, layer_, x_hit_, y_hit_; //chamber #, layer #, channel x,y
	double x_local, x_global, y_local, y_global, z_global; //x,y,z for local detector coordinates and global test stand coordinates
	double get_mapped_pos(int channel) const;
};
