#include <cmath>
#include "objective.hpp"

TTPObjective::TTPObjective( const std::string& name,
														const std::vector<ghost::Variable>& variables,
														int nb_teams,
														int nb_rounds,
														const std::vector< std::vector<double> >& distances )
	: Minimize( variables, name ),
	  number_teams( nb_teams ),
		number_rounds( nb_rounds ),
	  matrix_distances( distances )
{ }
