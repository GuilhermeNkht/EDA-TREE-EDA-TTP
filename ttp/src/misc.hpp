#pragma once

#include <string>
#include <vector>
#include "individual.hpp"

void convert( int match, int number_teams, int &home_team, int &away_team );

void extract_data_from_file( const std::string& filename,
                             int& number_variables,
                             std::vector< std::vector<double> >& matrix_distances);

int check_error_solution( const std::vector<int> &solution, int number_teams);

double check_cost_solution( const std::vector<int> &solution, int number_teams, const std::vector< std::vector<double> >& matrix_distances);

std::vector<std::vector<int>> select_best_individuals(const std::vector<std::vector<int>>& population, int number_elites, int number_teams, const std::vector<std::vector<double>>& matrix_distances);

std::vector<std::vector<int>> select_random_individuals(const std::vector<std::vector<int>>& population, int number_random, int number_teams);

double compute_average_entropy( const std::vector<std::vector<int>>& solution, int number_teams);

void check_mutual_information(const std::vector<std::vector<int>>& population, int number_teams);

int check_duplicate_solutions(const std::vector<Individual>& population);

double mean_value(double sum, int n_size);

std::pair<double, double> sum_cost(std::vector<Individual> population);

std::pair<double, double> sum_fitness(std::vector<Individual> population);

std::pair<int, int> sum_constraint(std::vector<Individual> population);

int count_feasible_solution(std::vector<Individual> population);

void check_best_solution(Individual& best_individual, std::vector<Individual> population);

void save_statistics_csv(const std::string& filename, std::vector<double> vec_cost, std::vector<int>& vec_costraint, std::vector<double>& vec_best_cost, std::vector<double>& vec_best_fitness, std::vector<int>& vec_best_constraint, std::vector<int>& vec_feasible_solutions, std::vector<double>& vec_total_mi, std::vector<double>& vec_entropy, std::vector<int>& vec_duplicated_solutions);