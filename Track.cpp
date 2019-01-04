#include <iostream>
#include <vector>
#include <map>
#include <string>

#include "Track.h"
#include "Position.h"

#include "TMath.h"
#include "TGraphErrors.h"
#include "TF1.h"



Track::Track() : is_valid_(false)
{
	x0_ = 0;
	y0_ = 0;
	dxdz_ = 0;
	dydz_ = 0;
}



Track::Track(std::vector<Position> const& positions) :
	is_valid_(false),
	positions_(positions)
{
	x0_ = 0;
	y0_ = 0;
	dxdz_ = 0;
	dydz_ = 0;

	if (positions_.size() > 1) //if there are at least 2 points, can fit a line
		fit(1);
}



bool Track::is_valid() const
{
	return is_valid_;
}



double Track::x(double const& z) const //return the x position for a given z
{
	return x0_ + dxdz_ * z;
}




double Track::y(double const& z) const //return the y position for a given z
{
	return y0_ + dydz_ * z;
}



double Track::x_at_csc(unsigned const& station) const
{
	//return x(Constants::STATION_SURVEY.at(station).z);
	return 0;
}



double Track::y_at_csc(unsigned const& station) const
{
	//return y(Constants::STATION_SURVEY.at(station).z);
	return 0;
}



double Track::x_at_crv(int layer) const //return the x position at the crv for a given layer
{
	//return x(Constants::CRV_SURVEY.z) - Constants::CRV_SURVEY.x;
	return 0;
}



double Track::y_at_crv(int layer) const //return the y position at the crv for a given layer
{
	//return y(Constants::CRV_SURVEY.z) - Constants::CRV_SURVEY.y;
	return 0;
}


void Track::fit()
{
	TF1 *xfit = new TF1("xfit", "pol1", 0, 1500);
	TF1 *yfit = new TF1("yfit", "pol1", 0, 1500);
	TGraphErrors *xz = new TGraphErrors(positions_.size());
	TGraphErrors *yz = new TGraphErrors(positions_.size());

	for (unsigned int i = 0; i < positions_.size(); i++)
	{
		xz->SetPoint(i, positions_[i].z(), positions_[i].x());
		xz->SetPointError(i, Z_UNCERTAINTY, CATHODE_WIDTH);
		yz->SetPoint(i, positions_[i].z(), positions_[i].y());
		yz->SetPointError(i, Z_UNCERTAINTY, CATHODE_WIDTH);
	}

	xz->Fit(xfit, "QN"); //fit quietly, don't store any graphics
	x0_ = xfit->GetParameter(0);
	dxdz_ = xfit->GetParameter(1);
	float xchi2 = xfit->GetChisquare();
	float xndf = xfit->GetNDF();
	float xchi2_ndf = xchi2 / xndf;

	yz->Fit(yfit, "QN"); //fit quietly, don't store any graphics
	y0_ = yfit->GetParameter(0);
	dydz_ = yfit->GetParameter(1);
	float ychi2 = yfit->GetChisquare();
	float yndf = yfit->GetNDF();
	float ychi2_ndf = ychi2 / xndf;


	if (xchi2_ndf < CHI2_NDF_CUT && ychi2_ndf < CHI2_NDF_CUT) //if any of the fits are poor, flag the track as bad
		is_valid_ = true;
	else
		is_valid_ = false;

}

void Track::fit(int a)
{
	double N = positions_.size(); //Get the total number of positions
	std::map<std::string, double> mean = { {"x", 0.0}, {"y", 0.0}, {"z", 0.0} }; //define a set of means, one for x, y, and z
	for (Position& p : positions_) //for each position in the list of positions
	{
		mean.at("x") += p.x() / N; //update the mean position for x
		mean.at("y") += p.y() / N; //	for y
		mean.at("z") += p.z() / N; //	for z
	}

	std::map<std::string, double> variance = { {"z", 0.0}, {"xz", 0.0}, {"yz", 0.0} }; //define the variance for z, xz, and yz
	for (Position& p : positions_) //for each position in the list of positions
	{
		variance.at("z") += get_variance(p.z(), mean.at("z")) / (N - 1); //get the variance for z
		variance.at("xz") += get_covariance(p.x(), mean.at("x"), p.z(), mean.at("z")) / (N - 1); //get the variance in xz
		variance.at("yz") += get_covariance(p.y(), mean.at("y"), p.z(), mean.at("z")) / (N - 1); //get the variance in yz
		//    variance.at("xx")  += get_variance(p.x(), mean.at("x"))    / (N - 1);
		//    variance.at("yy")  += get_variance(p.y(), mean.at("y"))    / (N - 1);
	}

	if (variance.at("z") == 0) //if there is no variance in z, all positions must be at the same plane (can't make tracks)
	{
		is_valid_ = false;
		return;
	}

	dxdz_ = variance.at("xz") / variance.at("z"); //dxdz = var(xz) - var(z)  expected slope
	x0_ = mean.at("x") - (dxdz_ * mean.at("z")); //x0 = (x-mean) - (dxdz * z-mean)  expected intercept

	dydz_ = variance.at("yz") / variance.at("z"); //dydz = var(yz) - var(z)  expected slope
	y0_ = mean.at("y") - (dydz_ * mean.at("z")); //y0 = (y-mean) - (dydz * z-mean)  expected intercept

	//Here starts the calculation for chi2
	std::map<std::string, double> chi2 = { {"chi2x",0.0},{"chi2y",0.0} }; //define chi2 for x and y directions
	for (Position& p : positions_) //for each position in the list of positions
	{
		double x_theo = x0_ + dxdz_ * p.z(); //x_theo = x0 + dxdz * z-pos
		double y_theo = y0_ + dydz_ * p.z(); //y_theo = y0 + dydz * z-pos
		chi2.at("chi2x") += (p.x() - x_theo)*(p.x() - x_theo)*TMath::Sqrt(12.); //the chi2 for x is updated with the (x-pos - x_theo)^2 * sqrt(12)
		chi2.at("chi2y") += (p.y() - y_theo)*(p.y() - y_theo)*TMath::Sqrt(12.); //the chi2 for y is updated with the (y-pos - y_theo)^2 * sqrt(12)

	}

	if (((chi2.at("chi2x")) < 20) && ((chi2.at("chi2y")) < 20)) //if the chi2 for x and the chi2 for y are less than 2, must be a valid track
	{
		is_valid_ = true;
		//	std::cout<<dxdz_<<" "<<chi2.at("chi2x")<<" "<<dydz_<<" "<<chi2.at("chi2y")<<std::endl;
	}
	else
		is_valid_ = false;
}



double Track::get_variance(double const& a, double const& mean_a) const
{
	return (a - mean_a) * (a - mean_a);
}



double Track::get_covariance(double const& a, double const& mean_a,
	double const& b, double const& mean_b) const
{
	return (a - mean_a) * (b - mean_b);
}

double Track::x0() const
{
	return x0_;
}

double Track::dxdz() const
{
	return dxdz_;
}

double Track::y0() const
{
	return y0_;
}

double Track::dydz() const
{
	return dydz_;
}



//std::ostream& operator<<(std::ostream & o, Track const& t)
//{
//	o << "(x0, dxdz) = (" << t.x0_ << ", " << t.dxdz_ << ")\n"
//		<< "(y0, dydz) = (" << t.y0_ << ", " << t.dydz_ << ")";
//
//	return o;
//}
