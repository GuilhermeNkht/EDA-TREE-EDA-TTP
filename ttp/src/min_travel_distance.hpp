#pragma once

#include "objective.hpp"

class MinTravelDistance : public TTPObjective
{
public:
	MinTravelDistance( const std::vector<ghost::Variable>& variables,
										 int nb_teams,
										 int nb_rounds,
										 const std::vector< std::vector<double> >& distances );
	
	double required_cost( const std::vector<ghost::Variable*>& variables ) const override;
};
