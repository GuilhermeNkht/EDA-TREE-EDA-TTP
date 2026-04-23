#pragma once
#include <vector>
#include <random>
#include <fstream>
#include <chrono>
#include "individual.hpp"

class eda_tree_ttp {
    public:
        eda_tree_ttp(int id_thread, int seed, const std::string& filename_csv, int penalization_value, int n_teams, const std::vector<std::vector<int>> initial_population, double elite_rate, double survivor_rate, int max_evaluations, const std::vector<std::vector<double>>& distance_matrix, int n_cores, double ls_probability);
        void run_boa(bool local_search, bool penalization, std::chrono::microseconds timeout_ls);

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
        std::vector<Individual> elite_population;

        int _n_slots;
        int _n_matches;
        int _n_elite;
        int _n_survive;
        int _n_cores;
        double _ls_probability;
        int _n_population_size;
        std::mt19937 rng;
        double _upper_bound;

        void select_elite();
        std::vector<Individual> select_survivors();
        void print_individual(const Individual& ind);
        std::vector<std::vector<double>> compute_marginals();
        double compute_mutual_information(int i, int j, const std::vector<std::vector<double>>& marginal);
        double compute_mean_entropy();
        std::vector<std::vector<double>> compute_mi_matrix();

        struct Edge {
            int u;
            int v;
            double w;
        };

        struct UnionFind {
            std::vector<int> parent;
            std::vector<int> rank;

            UnionFind(int n);
            int find(int x);
            bool unite(int a, int b);
        };

        std::pair<std::vector<int>, double> build_tree_from_mi(const std::vector<std::vector<double>>& mi_matrix);
        std::vector<std::vector<double>> prob_root;
        std::vector<std::vector<std::vector<double>>> prob_cond;
        void build_probabilities(const std::vector<int>& parent);
        std::vector<int> sample_individual(const std::vector<int>& parent);

};