#include "eda_ttp.hpp"
#include "misc.hpp"
#include "builder_ttp.hpp"
#include "print_ttp.hpp"
#include <iostream>
#include <random>
#include <fstream>
#include <ghost/solver.hpp>
#include <chrono>

using namespace std::literals::chrono_literals;

eda_ttp::eda_ttp(int id_thread, int seed, const std::string& filename_csv, int penalization_value, int n_teams, const std::vector<std::vector<int>> initial_population, double elite_rate, double survivor_rate, int max_generation, const std::vector<std::vector<double>>& distance_matrix, int n_cores, double ls_probability) : _filename_csv(filename_csv + ".csv"), _penalization_value(penalization_value), _n_teams(n_teams), _initial_population(initial_population), _elite_rate(elite_rate), _survivor_rate(survivor_rate), _max_generation(max_generation), _distance_matrix(distance_matrix), _n_cores(n_cores), _ls_probability(ls_probability)
{
    _n_matches = n_teams * (n_teams - 1);
    _n_slots = 2 * (n_teams - 1);
    rng.seed(seed + id_thread);

    for (const auto& sol : initial_population) {
        Individual ind;
        ind.sol = sol;
        ind.cost = check_cost_solution(sol, _n_teams, _distance_matrix);
        ind.constraint = check_error_solution(sol, _n_teams);
        ind.fitness = ind.cost + _penalization_value * ind.constraint;
        _population.push_back(ind);
    }

    _n_elite = std::max(1,int(_elite_rate * _population.size()));
    _n_survive = std::max(1,int(_survivor_rate * _n_elite));
    _n_population_size = _population.size();

    _upper_bound = 0;

    for(int i = 0; i < _distance_matrix.size(); i++){
        for(int j = 0; j < _distance_matrix[i].size(); j++){
            _upper_bound += _distance_matrix[i][j];
        }
    }

    _penalization_value = _upper_bound;
}

void eda_ttp::run_eda(bool local_search, bool penalization, bool eda, std::chrono::microseconds timeout_ls)
{
    int generation = 0;

    std::vector<double> vec_cost;
    std::vector<int> vec_costraint;
    std::vector<double> vec_best_cost;
    std::vector<double> vec_best_fitness;
    std::vector<int> vec_best_constraint;
    std::vector<int> vec_feasible_solutions;
    std::vector<double> vec_total_mi;
    std::vector<double> vec_entropy;
    std::vector<int> vec_duplicated_solutions;
    std::vector<double> vec_fitness_before_ls;
    std::vector<double> vec_fitness_after_ls;
    std::vector<double> vec_mean_improvement_ls;

    Individual best_individual = _population[0];

    std::shared_ptr<ghost::Print> printer = std::make_shared<PrintTTP>();
  	ghost::Options options;
	options.print = printer;
	options.number_start_samplings = _n_teams * _n_teams;
	options.enable_optimization_guidance = false;
	options.max_moves_in_opt_space = 2;
    options.custom_starting_point = true;

    if(_n_cores > 1){
	    options.parallel_runs = true;
        options.number_threads = _n_cores;
    }

    int size_population = _population.size();

    evaluate_population();

    BuilderTTP builder(_n_teams, _distance_matrix, {});
    ghost::Solver solver(builder);

    if(eda){
        while (generation < _max_generation) {

            select_elite();

            compute_probability();

            auto new_population = select_survivors();

            double sum_after_ls = 0;
            double sum_before_ls = 0;
            double sum_improvement = 0;
            int count_ls = 0;

            while(new_population.size() < size_population){
                
                auto child = sample_solution();
                
                Individual new_ind;

                bool generation_ls = (generation % 50 == 0);

                bool apply_ls = local_search && generation_ls && (std::uniform_real_distribution<double>(0.0,1.0)(rng) < _ls_probability);

                if(apply_ls){
                    double cost = 0.0;
                    std::vector<int> solution;
                    count_ls++;

                    double before_ls = check_cost_solution(child.sol, _n_teams, _distance_matrix) + _penalization_value * check_error_solution(child.sol, _n_teams);
                    sum_before_ls += before_ls;

                    builder.reinitialize_solution(child.sol);
                    solver.fast_search(cost, solution, timeout_ls, options);

                    new_ind.sol = solution;
                    new_ind.cost = check_cost_solution(solution, _n_teams, _distance_matrix);
                    new_ind.constraint = check_error_solution(solution, _n_teams);
                    new_ind.fitness = new_ind.cost + _penalization_value * new_ind.constraint;

                    sum_after_ls += new_ind.fitness;
                    sum_improvement += before_ls - sum_before_ls;
                }
                else{
                    new_ind.sol = child.sol;
                    new_ind.cost = check_cost_solution(child.sol, _n_teams, _distance_matrix);
                    new_ind.constraint = check_error_solution(child.sol, _n_teams);
                    new_ind.fitness = new_ind.cost + _penalization_value * new_ind.constraint;
                }

                new_population.push_back(new_ind);
            }

            _population = new_population;

            double mean_entropy = compute_mean_entropy();
            auto cost = sum_cost(_population);
            auto fitness = sum_fitness(_population);
            auto constraint = sum_constraint(_population);

            check_best_solution(best_individual, _population);

            vec_cost.push_back(mean_value(cost.first,_n_population_size));
            vec_costraint.push_back(mean_value(constraint.first,_n_population_size));
            vec_best_cost.push_back(best_individual.cost);
            vec_best_constraint.push_back(constraint.second);
            vec_best_fitness.push_back(fitness.second);
            vec_feasible_solutions.push_back(count_feasible_solution(_population));
            vec_entropy.push_back(mean_entropy);
            vec_total_mi.push_back(0);
            vec_duplicated_solutions.push_back(check_duplicate_solutions(_population));        
            vec_fitness_before_ls.push_back(mean_value(sum_before_ls, count_ls));
            vec_fitness_after_ls.push_back(mean_value(sum_after_ls, count_ls));
            vec_mean_improvement_ls.push_back(mean_value(sum_improvement, count_ls));

            generation++;

        }
    }
    else{
        std::shared_ptr<ghost::Print> printer = std::make_shared<PrintTTP>();
        ghost::Options options;
        options.print = printer;
        options.number_start_samplings = _n_teams * _n_teams;
        options.enable_optimization_guidance = false;
        options.max_moves_in_opt_space = 2;
        options.custom_starting_point = true;
        if(_n_cores > 1){
            options.parallel_runs = true;
            options.number_threads = _n_cores;
        }
        
        std::vector<int> solution;
        double cost = 0.0;

        BuilderTTP builder(_n_teams, _distance_matrix, _initial_population[0]);
        ghost::Solver solver(builder);

        solver.fast_search(cost, solution, 10min, options);
    }

    save_statistics_csv(_filename_csv, vec_cost, vec_costraint, vec_best_cost, vec_best_fitness, vec_best_constraint, vec_feasible_solutions, vec_total_mi, vec_entropy, vec_duplicated_solutions);
}

void eda_ttp::evaluate_population()
{
    for (auto& ind : _population) {
        ind.cost = check_cost_solution(ind.sol,_n_teams,_distance_matrix);
        ind.constraint = check_error_solution(ind.sol, _n_teams);
        ind.fitness = ind.cost + _penalization_value * ind.constraint;
    }
}

void eda_ttp::select_elite()
{
    elite_population.clear();

    std::vector<Individual> sorted_pop = _population;

    std::sort(
        sorted_pop.begin(),
        sorted_pop.end(),
        [](const Individual& a, const Individual& b) {
            return a.fitness < b.fitness;
        }
    );

    for (int i = 0; i < _n_elite && i < sorted_pop.size(); ++i) {
        elite_population.push_back(sorted_pop[i]);
    }

}

void eda_ttp::compute_probability()
{
    slot_probability.assign(
        _n_matches,
        std::vector<double>(_n_slots, 0.0)
    );

    for (const auto& ind : elite_population) {
        for (int m = 0; m < _n_matches; ++m) {
            int slot = ind.sol[m] - 1;
            slot_probability[m][slot] += 1.0;
        }
    }

    double laplace = 1.0;

    for (int m = 0; m < _n_matches; ++m) {
        for (int s = 0; s < _n_slots; ++s) {
            slot_probability[m][s] = (slot_probability[m][s] + laplace) / (_n_elite + laplace * _n_slots);
        }
    }
}

Individual eda_ttp::sample_solution()
{
    Individual ind;
    ind.sol.assign(_n_matches, -1);
    ind.cost = 0.0;

    for (int m = 0; m < _n_matches; ++m) {
        std::discrete_distribution<int> dist(
            slot_probability[m].begin(),
            slot_probability[m].end()
        );
        int slot = dist(rng);
        ind.sol[m] = slot + 1;
    }

    return ind;
}

std::vector<Individual> eda_ttp::select_survivors()
{
    std::vector<Individual> evaluated = _population;

    std::sort(
        evaluated.begin(),
        evaluated.end(),
        [](const Individual& a, const Individual& b) {
            return a.fitness < b.fitness;
        }
    );

    std::vector<Individual> survivors;
    for (int i = 0; i < _n_survive && i < evaluated.size(); ++i) {
        survivors.push_back(evaluated[i]);
    }

    return survivors;
}

double eda_ttp::compute_mean_entropy()
{
    double total_entropy = 0.0;

    for (int i = 0; i < _n_matches; ++i)
    {
        double Hi = 0.0;

        for (int s = 0; s < _n_slots; ++s)
        {
            double p = slot_probability[i][s];

            if (p > 0.0)
                Hi -= p * std::log2(p);
        }

        total_entropy += Hi;
    }

    return total_entropy / _n_matches;
}