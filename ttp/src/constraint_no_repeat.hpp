#pragma once

#include <vector>

#include <ghost/constraint.hpp>
#include <ghost/variable.hpp>

using namespace ghost;

class NoRepeat : public Constraint
{
	double required_error( const std::vector<Variable*>& variables ) const override;

public:
	NoRepeat( const std::vector<int>& variables_index );	
};
