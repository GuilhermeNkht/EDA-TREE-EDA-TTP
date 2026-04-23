#include "misc.hpp"

#include <iterator>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <numeric>
#include <cmath>
#include <algorithm>
#include <algorithm>
#include <random>
#include <filesystem>


void convert( int match, int number_teams, int &home_team, int &away_team )
{
	home_team = ( match / ( number_teams - 1 ) ) + 1;
	int shift = match / number_teams < home_team - 1 ? 1 : 2;
	away_team = ( match % ( number_teams - 1 ) ) + shift;
}

void extract_data_from_file( const std::string& filename,
                             int& number_variables,
                             std::vector< std::vector<double> >& matrix_distances )
{
	if( filename != "" )
	{		
		std::ifstream instance_file( filename );
		std::string line;
		std::string number;
		if( instance_file.is_open() )
		{

			std::getline( instance_file, line );
			std::stringstream ss_num( line );
			ss_num >> number;
			number_variables = std::stoi( number );


			for( int i = 0 ; i < number_variables ; ++i )
			{
				std::getline( instance_file, line );
				std::stringstream ss( line );
				matrix_distances.emplace_back( std::vector<double>( number_variables ) );
				for( int j = 0 ; j < number_variables ; ++j )
				{
					ss >> number;
					matrix_distances[i][j] = std::stod( number );
				}
			}
		}
	}
}

int check_error_solution( const std::vector<int> &solution, int number_teams )
{
	int number_rounds = 2 * ( number_teams - 1 );
	int number_matches = number_teams * ( number_teams - 1 );
	std::set<int> teams;
	std::vector<int> streaks( number_teams, 0 );
	std::vector< std::vector<int> > rounds( number_rounds );
	int home = -1;
	int away = -1;

	int number_violated_constraints = 0;
	
	for( int match = 0 ; match < number_matches ; ++match )
		rounds[solution[match] - 1].push_back( match );

	for( int round = 0 ; round < number_rounds ; ++round )
	{
		teams.clear();
		for( auto& match: rounds[round] )
		{
			convert( match, number_teams, home, away );
			teams.insert( home );
			teams.insert( away );

			if( streaks[ home-1 ] <= 0 )
				streaks[ home-1 ] = 1;
			else
			{
				++streaks[ home-1 ];
				if( streaks[ home-1 ] >= 4 )
				{
					++number_violated_constraints;
				}
			}
			
			if( streaks[ away-1 ] >= 0 )
				streaks[ away-1 ] = -1;
			else
			{
				--streaks[ away-1 ];
				if( streaks[ away-1 ] <= -4 )
				{
					++number_violated_constraints;
				}
			}
		}

		if( teams.size() != number_teams )
		{
			++number_violated_constraints;
		}
	}

	for( int match_a = 0 ; match_a < number_matches ; ++match_a )
	{
		convert( match_a, number_teams, home, away );
		if( home < away )
		{
			int match_b = match_a + ( number_teams - 2 ) * ( away - home ) + 1;
			if( std::abs( solution[ match_a ] - solution[ match_b ] ) <= 1 )
			{
				++number_violated_constraints;
			}
		}
	}

	return number_violated_constraints;
}

double check_cost_solution( const std::vector<int> &solution, int number_teams, const std::vector< std::vector<double> >& matrix_distances)
{
	double total_distance = 0.;
	int home = -1;
	int away = -1;
	
	int number_matches = number_teams * ( number_teams - 1 );
	int number_rounds = 2 * ( number_teams - 1 );
	std::vector< std::vector<int> > rounds( number_rounds );

	for( int match = 0 ; match < number_matches ; ++match )
		rounds[ solution[match] - 1 ].push_back( match );

	std::vector<int> previously_at( number_teams);
	std::iota( previously_at.begin(), previously_at.end(), 0 );

	for( int round = 0 ; round < number_rounds ; ++round )
	{
		for( auto& match: rounds[round] )
		{		
			convert( match, number_teams, home, away );
			--home;
			--away;

			total_distance += matrix_distances[ previously_at[ away ] ][ home ];
			if( previously_at[ home ] != home )
			{
				total_distance += matrix_distances[ previously_at[ home ] ][ home ];
				previously_at[ home ] = home;
			}
			if( round == number_rounds - 1 )
				total_distance += matrix_distances[ home ][ away ];
			else
				previously_at[ away ] = home;
		}
	}
	
	return total_distance;
}

std::vector<std::vector<int>> select_best_individuals(const std::vector<std::vector<int>>& population, int number_elites, int number_teams,const std::vector<std::vector<double>>& distance_matrix)
{
    const int population_size = population.size();

    std::vector<double> costs(population_size);

    for (int i = 0; i < population_size; ++i)
    {
        costs[i] = check_cost_solution(
            population[i],
            number_teams,
            distance_matrix
        );
    }

    std::vector<int> indices(population_size);
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(),
        [&](int a, int b)
        {
            return costs[a] < costs[b];
        });

    number_elites = std::min(number_elites, population_size);

    std::vector<std::vector<int>> elites;
    elites.reserve(number_elites);

    for (int i = 0; i < number_elites; ++i)
    {
        elites.push_back(population[indices[i]]);
    }

    return elites;
}

std::vector<std::vector<int>> select_random_individuals(const std::vector<std::vector<int>>& population, int number_random, int number_teams)
{
    int population_size = population.size();

    number_random = std::min(number_random, population_size);

    std::vector<int> indices(population_size);
    std::iota(indices.begin(), indices.end(), 0);

    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::shuffle(indices.begin(), indices.end(), gen);

    std::vector<std::vector<int>> random_population;
    random_population.reserve(number_random);

    for (int i = 0; i < number_random; ++i)
        random_population.push_back(population[indices[i]]);

    return random_population;
}

double compute_average_entropy(const std::vector<std::vector<int>>& population, int number_teams)
{
	int number_matches = number_teams * ( number_teams - 1 );
	int number_rounds = 2 * ( number_teams - 1 );
    int N = population.size();
    double total_entropy = 0.0;

    for (int var = 0; var < number_matches; ++var) {

        std::vector<int> count(number_rounds, 0);

        for (int i = 0; i < N; ++i) {
            int value = population[i][var] - 1;
            count[value]++;
        }

        double entropy = 0.0;

        for (int v = 0; v < number_rounds; ++v) {
            if (count[v] == 0)
                continue;

            double p = (double)count[v] / N;
            entropy -= p * std::log2(p);
        }

        total_entropy += entropy;
    }

    double Hmax = std::log2(number_rounds);
	return (total_entropy / number_matches) / Hmax;
}

double compute_single_entropy(const std::vector<std::vector<int>>& population, int var, int number_rounds)
{
    int N = population.size();

    std::vector<int> count(number_rounds, 0);

    for (int i = 0; i < N; ++i) {
        int value = population[i][var] - 1;
        count[value]++;
    }

    double entropy = 0.0;

    for (int v = 0; v < number_rounds; ++v) {
        if (count[v] == 0)
            continue;

        double p = (double)count[v] / N;
        entropy -= p * std::log2(p);
    }

    return entropy;
}

double compute_joint_entropy(const std::vector<std::vector<int>>& population, int var_i, int var_j, int number_rounds)
{
    int N = population.size();

    std::vector<std::vector<int>> count(number_rounds,std::vector<int>(number_rounds, 0));

    for (int i = 0; i < N; ++i) {
        int xi = population[i][var_i] - 1;
        int xj = population[i][var_j] - 1;

        count[xi][xj]++;
    }

    double entropy = 0.0;

    for (int a = 0; a < number_rounds; ++a) {
        for (int b = 0; b < number_rounds; ++b) {

            if (count[a][b] == 0)
                continue;

            double p = (double)count[a][b] / N;
            entropy -= p * std::log2(p);
        }
    }

    return entropy;
}

double compute_conditional_entropy(const std::vector<std::vector<int>>& population, int var_i, int var_j, int number_teams)
{
    int number_rounds = 2 * (number_teams - 1);

    double H_joint = compute_joint_entropy(population,var_i,var_j,number_rounds);

    double H_j = compute_single_entropy(population,var_j,number_rounds);

    return H_joint - H_j;
}

double compute_mutual_information(const std::vector<std::vector<int>>& population, int var_i, int var_j, int number_teams)
{
    int number_rounds = 2 * (number_teams - 1);

    double Hi = compute_single_entropy(population, var_i, number_rounds);
    double Hj = compute_single_entropy(population, var_j, number_rounds);
    double Hjoint = compute_joint_entropy(population, var_i, var_j, number_rounds);

    return Hi + Hj - Hjoint;
}

void check_mutual_information(const std::vector<std::vector<int>>& population,int number_teams)
{
    int number_matches = number_teams * (number_teams - 1);

    for (int i = 0; i < number_matches; ++i) {

        for (int j = 0; j < number_matches; ++j) {

            if (i == j)
                continue;

            double m_i = compute_mutual_information(population,i,j,number_teams);

            std::cout << "Mutual Information: " << m_i << std::endl;
        }
    }
}

int check_duplicate_solutions(const std::vector<Individual>& population) {
    std::set<std::vector<int>> solutions;

    int duplicates = 0;

    for (const auto& individual : population) {
        auto result = solutions.insert(individual.sol);

        if (!result.second) {
            duplicates++;
        }
    }

    return duplicates;
}

double mean_value(double sum, int n_size)
{
    if(n_size == 0)
        return 0;

    return sum / n_size;
}

std::pair<double, double> sum_cost(std::vector<Individual> population)
{
    int sum = 0;
    int best_cost = population[0].cost;

    for(const auto& ind : population){
        sum += ind.cost;

        if(ind.cost < best_cost){
            best_cost = ind.cost;
        }
    }

    return {sum, best_cost};
}

std::pair<double, double> sum_fitness(std::vector<Individual> population)
{
    int sum = 0;
    int best_fitness = population[0].fitness;

    for(const auto& ind : population){
        sum += ind.fitness;

        if(ind.fitness < best_fitness){
            best_fitness = ind.fitness;
        }
    }

    return {sum, best_fitness};
}


std::pair<int, int> sum_constraint(std::vector<Individual> population)
{
    int sum = 0;
    int best_constraint = population[0].constraint;

    for(const auto& ind : population){
        sum += ind.constraint;

        if(ind.constraint < best_constraint){
            best_constraint = ind.constraint;
        }
    }

    return {sum, best_constraint};
}

int count_feasible_solution(std::vector<Individual> population)
{
    int feasible_solution = 0;
    
    for(const auto& ind : population){
        if(ind.constraint == 0)
            feasible_solution++;
    }

    return feasible_solution;
}

void check_best_solution(Individual& best_individual, std::vector<Individual> population){
    
    for(auto individual : population){
        if(individual.cost < best_individual.cost and individual.constraint == 0)
            best_individual = individual;
    }

}

void save_statistics_csv(const std::string& filename, std::vector<double> vec_cost, std::vector<int>& vec_costraint, std::vector<double>& vec_best_cost, std::vector<double>& vec_best_fitness, std::vector<int>& vec_best_constraint, std::vector<int>& vec_feasible_solutions, std::vector<double>& vec_total_mi, std::vector<double>& vec_entropy, std::vector<int>& vec_duplicated_solutions)
{

    std::filesystem::path filepath(filename);

    std::filesystem::create_directories(filepath.parent_path());

    std::ofstream file(filename);

    file << "generation,cost,constraint,best_cost,best_fitness,less_constraint,feasible_solutions, mi_total, entropy, duplicated_solutions\n";

    int n = vec_cost.size();

    for (int i = 0; i < n; ++i) {
        file << i << "," << vec_cost[i] << "," << vec_costraint[i] << "," << vec_best_cost[i] << "," << vec_best_fitness[i] << "," << vec_best_constraint[i] << "," << vec_feasible_solutions[i] << "," << vec_total_mi[i] << "," << vec_entropy[i] << "," << vec_duplicated_solutions[i] << "\n";
    }

}