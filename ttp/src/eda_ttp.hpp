#pragma once
#include <vector>
#include <random>
#include <fstream>
#include <chrono>
#include "individual.hpp"

class eda_ttp {
    public:
        eda_ttp(int id_thread, int seed, const std::string& filename_csv, int penalization_value, int n_teams, const std::vector<std::vector<int>> initial_population, double elite_rate, double survivor_rate, int max_evaluations, const std::vector<std::vector<double>>& distance_matrix, int n_cores, double ls_probability);

        void initializePolygon();
        void run_eda(bool local_search, bool penalization, bool eda, std::chrono::microseconds timeout_ls);

    private:
        std::string _filename_csv;
        int _penalization_value;
        int _n_teams;
        const std::vector<std::vector<int>>& _initial_population;
        std::vector<Individual> _population;
        double _elite_rate;
        double _survivor_rate;
        int _max_generation;
        const std::vector<std::vector<double>> _distance_matrix;

        int _n_slots;
        int _n_matches;
        int _n_elite;
        int _n_survive;
        int _n_cores;
        double _ls_probability;
        int _n_population_size;
        std::mt19937 rng;
        double _upper_bound;

        std::vector<Individual> elite_population;
        std::vector<std::vector<double>> slot_probability;

        void evaluate_population();
        void select_elite();
        void compute_probability();
        double compute_mean_entropy();

        Individual sample_solution();
        std::vector<Individual> select_survivors();
};