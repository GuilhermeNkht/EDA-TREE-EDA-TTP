#pragma once

#include <ghost/model_builder.hpp>

class BuilderTTP : public ghost::ModelBuilder
{
	int _number_teams;
	int _number_matches;
	int _number_rounds;
	std::vector< std::vector<double> > _distance_matrix;

	std::vector< std::vector<int> > _homes;
	std::vector< std::vector<int> > _aways;
	std::vector< std::vector<int> > _pairs;
	
public:
	BuilderTTP( int number_teams, const std::vector< std::vector<double> >& distances, const std::vector<int>& start_solution);
	BuilderTTP( int number_teams );
	
	void declare_variables() override;
	void declare_constraints() override;
	void reinitialize_solution(const std::vector<int>& sol);
#if defined TTP_OPT
	void declare_objective() override;
#endif

private:
	std::vector<int> _start_solution;
};
