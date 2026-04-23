#pragma once

#include <vector>

#include <ghost/constraint.hpp>
#include <ghost/variable.hpp>

using namespace ghost;

class MaxStreak : public Constraint
{
	double required_error( const std::vector<Variable*>& variables ) const override;

public:
	MaxStreak( const std::vector<int>& variables_index );	
};
