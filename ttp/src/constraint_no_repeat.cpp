#include <algorithm>

#include "constraint_no_repeat.hpp"

using namespace ghost;

NoRepeat::NoRepeat( const std::vector<int>& variables_index )
	: Constraint( variables_index )
{ }

double NoRepeat::required_error( const std::vector<Variable*>& variables ) const
{
	return std::max( 0.0, 2.0 - std::abs( variables[0]->get_value() - variables[1]->get_value() ) );
}