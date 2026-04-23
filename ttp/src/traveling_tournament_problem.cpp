#include <iostream>
#include <fstream>
#include <string>

#include <vector>
#include <set>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <thread>
#include <ghost/solver.hpp>
#include <filesystem>

#include "builder_ttp.hpp"
#include "print_ttp.hpp"
#include "misc.hpp"
#include "ttp_initial_solution.hpp"
#include "eda_ttp.hpp"
#include "eda_tree_ttp.hpp"
#include "experiment_configurations.hpp"

using namespace std::literals::chrono_literals;

int main( int argc, char **argv )
{
	bool success;
	int number_teams;

	std::vector<ExperimentConfig> experiments; 
	ExperimentConfig experiment_base;
	std::vector<std::vector<int>> initial_solutions;

	std::vector< std::vector<double>> distances;

	if( argc >= 13 ){
		experiment_base.file = argv[1];
		experiment_base.penalization_value = std::stoi(argv[2]);
		experiment_base.cores = std::stoi(argv[3]);
		experiment_base.elite_rate = std::stod(argv[4]);
		experiment_base.survivor_rate = std::stod(argv[5]);
		experiment_base.max_generation = std::stoi(argv[6]);
		experiment_base.n_population = std::stoi(argv[7]);
		experiment_base.timeout_ls = std::chrono::microseconds(std::stoll(argv[8]));
		experiment_base.local_search = (std::string(argv[9]) == "true");
		experiment_base.penalization = (std::string(argv[10]) == "true");
		experiment_base.num_runs = std::stoi(argv[11]);
		experiment_base.seed = std::stoi(argv[12]);
		experiment_base.name = "base";
	}

	extract_data_from_file(experiment_base.file, number_teams, distances);

	ttp_initial_solution sol(number_teams, distances);

	initial_solutions = sol.generateAlternatingMethod(experiment_base.n_population);

	eda_ttp eda(0, experiment_base.seed, "eda/without_local_search/thread_" + std::to_string(0) + "_without_local_search", experiment_base.penalization_value, number_teams, initial_solutions, experiment_base.elite_rate, experiment_base.survivor_rate, experiment_base.max_generation, distances, experiment_base.cores, experiment_base.ls_probability);
	eda.run_eda(false, experiment_base.penalization, true, experiment_base.timeout_ls);

	eda_tree_ttp eda_tree(0, experiment_base.seed, "edatree/without_local_search/thread_" + std::to_string(0) + "_without_local_search", experiment_base.penalization_value, number_teams, initial_solutions, experiment_base.elite_rate, experiment_base.survivor_rate, experiment_base.max_generation, distances, experiment_base.cores, experiment_base.ls_probability);
	eda_tree.run_boa(false, experiment_base.penalization, experiment_base.timeout_ls);

	if( success )
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;

}