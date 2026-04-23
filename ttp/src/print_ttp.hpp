#pragma once

#include <sstream>
#include <vector>

#include "ghost/print.hpp"
#include "ghost/variable.hpp"

class PrintTTP : public ghost::Print
{
public:
	std::stringstream print_candidate( const std::vector<ghost::Variable>& variables ) const override;
};
