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
#include <fstream>
#include <mutex>

#include "builder_ttp.hpp"
#include "print_ttp.hpp"
#include "misc.hpp"
#include "ttp_initial_solution.hpp"
#include "eda_ttp.hpp"
#include "eda_tree_ttp.hpp"
#include "experiment_configurations.hpp"

using namespace std::literals::chrono_literals;
std::mutex file_mutex;

int main(int argc, char **argv)
{
	bool success;
	int number_teams;

	std::vector<ExperimentConfig> experiments;
	ExperimentConfig experiment_base;
	std::vector<std::vector<int>> initial_solutions;

	std::vector<std::vector<double>> distances;

	if (argc >= 8)
	{
		experiment_base.file = argv[1];
		experiment_base.elite_rate = std::stod(argv[2]);
		experiment_base.retained_rate = std::stod(argv[3]);
		experiment_base.max_evaluations = std::stoi(argv[4]);
		experiment_base.n_population = std::stoi(argv[5]);
		experiment_base.num_runs = std::stoi(argv[6]);
		experiment_base.seed = std::stoi(argv[7]);
		experiment_base.name = argv[8];
	}

	extract_data_from_file(experiment_base.file, number_teams, distances);
	ttp_initial_solution sol(number_teams, distances);

	experiment_base.name = "Random";
	initial_solutions = sol.generateRandomSolution(experiment_base.n_population);


	eda_tree_ttp eda_tree(0, experiment_base.seed, "edatree/without_local_search/thread_" + std::to_string(0) + "_without_local_search", number_teams, initial_solutions, experiment_base.elite_rate, experiment_base.retained_rate, experiment_base.max_evaluations, distances, 1);
	eda_tree.run_eda_tree();

}