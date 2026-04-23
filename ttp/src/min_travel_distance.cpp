#include <numeric>
#include <algorithm>

#include "min_travel_distance.hpp"
#include "misc.hpp"

MinTravelDistance::MinTravelDistance( const std::vector<ghost::Variable>& variables,
																			int nb_teams,
																			int nb_rounds,
																			const std::vector< std::vector<double> >& distances )
	: TTPObjective( "MinTravelDistance",
									variables,
									nb_teams,
									nb_rounds,
									distances )
{ }

double MinTravelDistance::required_cost( const std::vector<ghost::Variable*>& variables ) const
{
	double total_distance = 0.;
	int home = -1;
	int away = -1;
	
	int number_matches = number_teams * ( number_teams - 1 );
	std::vector< std::vector<int> > rounds( number_rounds );

	for( int match = 0 ; match < number_matches ; ++match )
		rounds[ variables[match]->get_value() - 1 ].push_back( match ); // variables[match]->get_value() - 1 because rounds start at 1

	std::vector<int> previously_at( number_teams);
	std::iota( previously_at.begin(), previously_at.end(), 0 );

	for( int round = 0 ; round < number_rounds ; ++round )
	{
		for( auto& match: rounds[round] )
		{		
			convert( match, number_teams, home, away );
			--home;
			--away;

			total_distance += matrix_distances[ previously_at[ away ] ][ home ];
			if( previously_at[ home ] != home )
			{
				total_distance += matrix_distances[ previously_at[ home ] ][ home ];
				previously_at[ home ] = home;
			}
			if( round == number_rounds - 1 )
				total_distance += matrix_distances[ home ][ away ]; // away returns home
			else
				previously_at[ away ] = home;
		}
	}
	
	return total_distance;
}
