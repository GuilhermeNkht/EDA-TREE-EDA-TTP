#include <cmath>
#include "min_max_streak_distance.hpp"
#include "misc.hpp"

MinMaxStreakDistance::MinMaxStreakDistance( const std::vector<ghost::Variable>& variables,
																						int nb_teams,
																						int nb_rounds,
																						const std::vector< std::vector<double> >& distances )
	: TTPObjective( "MinMaxStreakDistance",
									variables,
									nb_teams,
									nb_rounds,
									distances )
{ }

double MinMaxStreakDistance::required_cost( const std::vector<ghost::Variable*>& variables ) const
{
	double max_streak = 0.;
	std::vector<double> max_streaks( number_teams, 0. );
	std::vector< std::vector<int> > rounds( number_rounds );

	for( size_t match = 0 ; match < variables.size() ; ++match )
		rounds[variables[match]->get_value() - 1].push_back( match ); // variables[match]->get_value() - 1 because rounds start at 1
	
	int home = -1;
	int away = -1;

	for( int round = 0 ; round < number_rounds ; ++round )
		for( auto& match: rounds[round] )
		{		
			convert( match, number_teams, home, away );
			--home;
			--away;
			
			max_streaks[home] = 0.;
			max_streaks[away] += matrix_distances[away][home];
			if( max_streak < max_streaks[away] )
				max_streak = max_streaks[away];
		}

	return max_streak;
}
