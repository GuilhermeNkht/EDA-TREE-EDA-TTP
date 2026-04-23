#pragma once

#include <ghost/objective.hpp>

class TTPObjective : public ghost::Minimize
{
protected:
	int number_teams;
	int number_rounds;
	std::vector< std::vector<double> > matrix_distances;
	
public:
	TTPObjective( const std::string& name,
								const std::vector<ghost::Variable>& variables,
								int number_teams,
								int number_rounds,
								const std::vector< std::vector<double> >& matrix_distances );
};
