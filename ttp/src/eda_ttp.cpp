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

eda_ttp::eda_ttp(int id_thread, int seed, const std::string &filename_csv, int n_teams, const std::vector<std::vector<int>> initial_population, double elite_rate, double retained_rate, int max_evaluation, const std::vector<std::vector<double>> &distance_matrix, double intensity_penalization) : _filename_csv(filename_csv + ".csv"), _n_teams(n_teams), _initial_population(initial_population), _elite_rate(elite_rate), _retained_rate(retained_rate), _max_evaluation(max_evaluation), _distance_matrix(distance_matrix), _intensity_penalization(intensity_penalization)
{
    _n_matches = n_teams * (n_teams - 1);
    _n_slots = 2 * (n_teams - 1);
    rng.seed(seed + id_thread);

    _upper_bound = 0;

    for (int i = 0; i < _distance_matrix.size(); i++)
    {
        for (int j = 0; j < _distance_matrix[i].size(); j++)
        {
            _upper_bound += _distance_matrix[i][j];
        }
    }

    _penalization_value = _upper_bound;

    std::cout << "Upper bound: " << _upper_bound << std::endl;

    for (const auto &sol : initial_population)
    {
        Individual ind;
        ind.sol = sol;
        ind.cost = check_cost_solution(sol, _n_teams, _distance_matrix);
        ind.constraint = check_error_solution(sol, _n_teams);
        ind.fitness = ind.cost + _intensity_penalization * _penalization_value * ind.constraint;
        _population.push_back(ind);
    }

    _n_elite = std::max(1, int(_elite_rate * _population.size()));
    _n_survive = std::max(1, int(_retained_rate * _n_elite));
    _n_population_size = _population.size();

}

void eda_ttp::run_eda()
{
    std::vector<int> vec_evaluation;
    std::vector<double> vec_cost;
    std::vector<int> vec_costraint;
    std::vector<double> vec_best_cost;
    std::vector<double> vec_best_fitness;
    std::vector<int> vec_best_constraint;
    std::vector<int> vec_feasible_solutions;
    std::vector<double> vec_total_mi;
    std::vector<double> vec_entropy;
    std::vector<int> vec_duplicated_solutions;

    Individual best_individual = _population[0];

    int size_population = _population.size();

    int evaluation = size_population;

    while (evaluation < _max_evaluation)
    {

        select_elite();

        compute_probability();

        auto new_population = select_survivors();

        while (new_population.size() < size_population)
        {

            auto child = sample_solution();

            Individual new_ind;

            new_ind.sol = child.sol;
            new_ind.cost = check_cost_solution(child.sol, _n_teams, _distance_matrix);
            new_ind.constraint = check_error_solution(child.sol, _n_teams);
            new_ind.fitness = new_ind.cost + _intensity_penalization * _penalization_value * new_ind.constraint;

            new_population.push_back(new_ind);

            evaluation++;
        }

        _population = new_population;

        double mean_entropy = compute_mean_entropy();
        auto cost = sum_cost(_population);
        auto fitness = sum_fitness(_population);
        auto constraint = sum_constraint(_population);

        check_best_solution(best_individual, _population);

        vec_evaluation.push_back(evaluation);
        vec_cost.push_back(mean_value(cost.first, _n_population_size));
        vec_costraint.push_back(mean_value(constraint.first, _n_population_size));
        vec_best_cost.push_back(best_individual.cost);
        vec_best_constraint.push_back(constraint.second);
        vec_best_fitness.push_back(fitness.second);
        vec_feasible_solutions.push_back(count_feasible_solution(_population));
        vec_entropy.push_back(mean_entropy);
        vec_total_mi.push_back(0);
        vec_duplicated_solutions.push_back(check_duplicate_solutions(_population));
    }

    save_statistics_csv(_filename_csv, vec_evaluation, vec_cost, vec_costraint, vec_best_cost, vec_best_fitness, vec_best_constraint, vec_feasible_solutions, vec_total_mi, vec_entropy, vec_duplicated_solutions);
}

void eda_ttp::evaluate_population()
{
    for (auto &ind : _population)
    {
        ind.cost = check_cost_solution(ind.sol, _n_teams, _distance_matrix);
        ind.constraint = check_error_solution(ind.sol, _n_teams);
        ind.fitness = ind.cost + _intensity_penalization * _penalization_value * ind.constraint;
    }
}

void eda_ttp::select_elite()
{
    elite_population.clear();

    std::vector<Individual> sorted_pop = _population;

    std::sort(
        sorted_pop.begin(),
        sorted_pop.end(),
        [](const Individual &a, const Individual &b)
        {
            return a.fitness < b.fitness;
        });

    for (int i = 0; i < _n_elite && i < sorted_pop.size(); ++i)
    {
        elite_population.push_back(sorted_pop[i]);
    }
}

void eda_ttp::compute_probability()
{
    slot_probability.assign(
        _n_matches,
        std::vector<double>(_n_slots, 0.0));

    for (const auto &ind : elite_population)
    {
        for (int m = 0; m < _n_matches; ++m)
        {
            int slot = ind.sol[m] - 1;
            slot_probability[m][slot] += 1.0;
        }
    }

    double laplace = 1.0;

    for (int m = 0; m < _n_matches; ++m)
    {
        for (int s = 0; s < _n_slots; ++s)
        {
            slot_probability[m][s] = (slot_probability[m][s] + laplace) / (_n_elite + laplace * _n_slots);
        }
    }
}

Individual eda_ttp::sample_solution()
{
    Individual ind;
    ind.sol.assign(_n_matches, -1);
    ind.cost = 0.0;

    for (int m = 0; m < _n_matches; ++m)
    {
        std::discrete_distribution<int> dist(
            slot_probability[m].begin(),
            slot_probability[m].end());
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
        [](const Individual &a, const Individual &b)
        {
            return a.fitness < b.fitness;
        });

    std::vector<Individual> survivors;
    for (int i = 0; i < _n_survive && i < evaluated.size(); ++i)
    {
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

void eda_ttp::print_parameters()
{
    std::cout << "\n===== CONFIG =====\n";
    std::cout << "Population size: " << _n_population_size << '\n';
    std::cout << "Elite rate: " << _elite_rate << '\n';
    std::cout << "Survivor rate: " << _retained_rate << '\n';
    std::cout << "Max evaluations: " << _max_evaluation << '\n';
    std::cout << "Teams: " << _n_teams << '\n';
    std::cout << "Elite count: " << _n_elite << '\n';
    std::cout << "Survivors: " << _n_survive << '\n';
    std::cout << "Intensity penalization: " << _intensity_penalization << '\n';
    std::cout << "=================\n";
}