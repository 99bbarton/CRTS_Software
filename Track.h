#pragma once
#include <vector>
#include "Position.h"
#include "Constants.h"




class Track
{
public:
	Track();
	Track(std::vector<Position> const& positions);

	double x(double const& z) const;
	double y(double const& z) const;

	double x_at_csc(unsigned const& station) const;
	double y_at_csc(unsigned const& station) const;

	double x_at_crv(int layer) const;
	double y_at_crv(int layer) const;

	double x0() const;
	double dxdz() const;
	double y0() const;
	double dydz() const;

	bool is_valid() const;

private:
	void fit();
	void fit(int);
	double get_variance(double const& a, double const& mean_a) const;
	double get_covariance(double const& a, double const& mean_a,
		double const& b, double const& mean_b) const;

	bool is_valid_;
	double x0_, y0_, dxdz_, dydz_;
	std::vector<Position> positions_;
};
