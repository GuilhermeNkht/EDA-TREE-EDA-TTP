#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>

#include "print_ttp.hpp"
#include "misc.hpp"

std::stringstream PrintTTP::print_candidate( const std::vector<ghost::Variable>& variables ) const
{
	std::stringstream stream;

	int nb_matches = static_cast<int>( variables.size() );
	int nb_teams = static_cast<int>( std::ceil( std::sqrt( nb_matches ) ) );

	int indent = std::ceil( std::log10( nb_teams ) ) + 1;

	int home = -1;
	int away = -1;

	int count = 1;
	
	for( int match = 0 ; match < nb_matches ; ++match )
	{
		convert( match, nb_teams, home, away );
		if( home != away )
			stream << std::setw( indent ) << home << "/" << away << std::setw( indent+1 ) << variables[match].get_value() <<", ";

		if( count % ( nb_teams - 1 ) == 0 )
			stream << "\n";
		++count;			
	}

	return stream;
}
