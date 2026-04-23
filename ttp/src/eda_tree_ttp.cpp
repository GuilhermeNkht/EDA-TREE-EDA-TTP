#include "eda_tree_ttp.hpp"
#include "misc.hpp"
#include "builder_ttp.hpp"
#include "print_ttp.hpp"
#include <iostream>
#include <random>
#include <fstream>
#include <ghost/solver.hpp>
#include <chrono>
#include <queue>

using namespace std::chrono_literals;

eda_tree_ttp::eda_tree_ttp(int id_thread, int seed, const std::string& filename_csv, int penalization_value, int n_teams, const std::vector<std::vector<int>> initial_population, double elite_rate, double survivor_rate, int max_generation, const std::vector<std::vector<double>>& distance_matrix, int n_cores, double ls_probability) : _filename_csv(filename_csv + ".csv"), _penalization_value(penalization_value), _n_teams(n_teams), _initial_population(initial_population), _elite_rate(elite_rate), _survivor_rate(survivor_rate), _max_generation(max_generation), _distance_matrix(distance_matrix), _n_cores(n_cores), _ls_probability(ls_probability)
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

void eda_tree_ttp::run_boa(bool local_search, bool penalization, std::chrono::microseconds timeout_ls)
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

    BuilderTTP builder(_n_teams, _distance_matrix, {});
    ghost::Solver solver(builder);

    while (generation < _max_generation) {

        select_elite();

        auto new_population = select_survivors();

        auto mi_matrix = compute_mi_matrix();

        auto [parent, total_mi] = build_tree_from_mi(mi_matrix);

        build_probabilities(parent);

        double sum_after_ls = 0;
        double sum_before_ls = 0;
        double sum_improvement = 0;
        int count_ls = 0;

        while(new_population.size() < _n_population_size){

            Individual new_ind;
            auto child = sample_individual(parent);

            bool generation_ls = (generation % 50 == 0);

            bool apply_ls = local_search && generation_ls && (std::uniform_real_distribution<double>(0.0,1.0)(rng) < _ls_probability);

            if(apply_ls){
                std::vector<int> solution;
                double cost = 0.0;
                count_ls++;
                
                double before_ls = check_cost_solution(child, _n_teams, _distance_matrix) + _penalization_value * check_error_solution(child, _n_teams);
                sum_before_ls += before_ls;

                builder.reinitialize_solution(child);
                solver.fast_search(cost, solution, timeout_ls, options);

                new_ind.sol = solution;
                new_ind.cost = cost;
                new_ind.constraint = check_error_solution(solution, _n_teams);
                new_ind.fitness = new_ind.cost + _penalization_value * new_ind.constraint;

                sum_after_ls += new_ind.fitness;

                sum_improvement += before_ls - sum_before_ls;
            }
            else{
                new_ind.sol = child;
                new_ind.cost = check_cost_solution(child, _n_teams, _distance_matrix);
                new_ind.constraint = check_error_solution(child, _n_teams);
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

        vec_cost.push_back(mean_value(cost.first, _n_population_size));
        vec_costraint.push_back(mean_value(constraint.first, _n_population_size));
        vec_best_cost.push_back(best_individual.cost);
        vec_best_constraint.push_back(constraint.second);
        vec_best_fitness.push_back(fitness.second);
        vec_feasible_solutions.push_back(count_feasible_solution(_population));
        vec_total_mi.push_back(total_mi);
        vec_entropy.push_back(mean_entropy);
        vec_duplicated_solutions.push_back(check_duplicate_solutions(_population));
        vec_fitness_before_ls.push_back(mean_value(sum_before_ls, count_ls));
        vec_fitness_after_ls.push_back(mean_value(sum_after_ls, count_ls));
        vec_mean_improvement_ls.push_back(mean_value(sum_improvement, count_ls));

        generation++;
    }

    save_statistics_csv(_filename_csv, vec_cost, vec_costraint, vec_best_cost, vec_best_fitness, vec_best_constraint, vec_feasible_solutions, vec_total_mi, vec_entropy, vec_duplicated_solutions);

}

void eda_tree_ttp::select_elite()
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

std::vector<std::vector<double>> eda_tree_ttp::compute_marginals()
{
    std::vector<std::vector<double>> marginal(_n_matches, std::vector<double>(_n_slots, 0.0));

    int elite_size = elite_population.size();

    for (const auto& ind : elite_population) {
        for (int m = 0; m < _n_matches; ++m) {
            int slot = ind.sol[m] - 1;
            marginal[m][slot] += 1.0;
        }
    }

    for (int m = 0; m < _n_matches; ++m) {
        for (int s = 0; s < _n_slots; ++s) {
            marginal[m][s] /= elite_size;
        }
    }

    return marginal;
}

double eda_tree_ttp::compute_mutual_information(int i, int j, const std::vector<std::vector<double>>& marginal)
{
    std::vector<std::vector<double>> joint(_n_slots,std::vector<double>(_n_slots, 0.0));

    int elite_size = elite_population.size();

    for (const auto& ind : elite_population) {
        int si = ind.sol[i] - 1;
        int sj = ind.sol[j] - 1;
        joint[si][sj] += 1.0;
    }

    for (int si = 0; si < _n_slots; ++si){
        for (int sj = 0; sj < _n_slots; ++sj){
            joint[si][sj] /= elite_size;
        }
    }

    double mi = 0.0;

    for (int si = 0; si < _n_slots; ++si) {
        for (int sj = 0; sj < _n_slots; ++sj) {

            double p_ij = joint[si][sj];

            if (p_ij > 0) {
                double p_i = marginal[i][si];
                double p_j = marginal[j][sj];

                mi += p_ij * std::log2(p_ij / (p_i * p_j));
            }
        }
    }

    return mi;
}

std::vector<std::vector<double>> eda_tree_ttp::compute_mi_matrix()
{
    auto marginal = compute_marginals();

    std::vector<std::vector<double>> mi_matrix(
        _n_matches,
        std::vector<double>(_n_matches, 0.0)
    );

    for (int i = 0; i < _n_matches; ++i) {
        for (int j = i + 1; j < _n_matches; ++j) {

            double mi = compute_mutual_information(i, j, marginal);

            mi_matrix[i][j] = mi;
            mi_matrix[j][i] = mi;
        }
    }

    return mi_matrix;
}

eda_tree_ttp::UnionFind::UnionFind(int n)
{
    parent.resize(n);
    rank.assign(n, 0);
    for (int i = 0; i < n; ++i)
        parent[i] = i;
}

int eda_tree_ttp::UnionFind::find(int x)
{
    if (parent[x] != x)
        parent[x] = find(parent[x]);
    return parent[x];
}

bool eda_tree_ttp::UnionFind::unite(int a, int b)
{
    int ra = find(a);
    int rb = find(b);

    if (ra == rb)
        return false;

    if (rank[ra] < rank[rb]){
        parent[ra] = rb;
    }
    else if (rank[ra] > rank[rb]){
        parent[rb] = ra;
    } 
    else {
        parent[rb] = ra;
        rank[ra]++;
    }

    return true;
}

std::pair<std::vector<int>, double> eda_tree_ttp::build_tree_from_mi(const std::vector<std::vector<double>>& mi_matrix)
{
    std::vector<Edge> edges;

    for (int i = 0; i < _n_matches; ++i) {
        for (int j = i + 1; j < _n_matches; ++j) {
            edges.push_back({i, j, mi_matrix[i][j]});
        }
    }

    std::sort(edges.begin(), edges.end(),
        [](const Edge& a, const Edge& b) {
            return a.w > b.w;
        });

    UnionFind uf(_n_matches);

    std::vector<std::vector<int>> adj(_n_matches);

    int added = 0;
    double total_mi = 0.0;

    for (const auto& e : edges) {
        if (uf.unite(e.u, e.v)) {
            adj[e.u].push_back(e.v);
            adj[e.v].push_back(e.u);
            added++;
            total_mi += e.w;

            if (added == _n_matches - 1)
                break;
        }
    }

    std::vector<int> parent(_n_matches, -1);
    std::vector<bool> visited(_n_matches, false);

    std::uniform_int_distribution<int> dist(0, _n_matches - 1);
    int root = dist(rng);
    std::queue<int> q;

    q.push(root);
    visited[root] = true;
    parent[root] = -1;

    while (!q.empty()) {
        int cur = q.front();
        q.pop();

        for (int nxt : adj[cur]) {
            if (!visited[nxt]) {
                visited[nxt] = true;
                parent[nxt] = cur;
                q.push(nxt);
            }
        }
    }

    return {parent, total_mi};
}

void eda_tree_ttp::build_probabilities(const std::vector<int>& parent)
{
    int elite_size = elite_population.size();

    prob_root.assign(_n_matches, std::vector<double>(_n_slots, 0.0));

    prob_cond.assign(_n_matches, std::vector<std::vector<double>>(_n_slots, std::vector<double>(_n_slots, 0.0)));

    std::vector<std::vector<double>> parent_count(_n_matches, std::vector<double>(_n_slots, 0.0));

    for (const auto& ind : elite_population){
        for (int i = 0; i < _n_matches; ++i){
            int slot_i = ind.sol[i] - 1;

            if (parent[i] == -1){
                prob_root[i][slot_i] += 1.0;
            }
            else{
                int p = parent[i];
                int slot_p = ind.sol[p] - 1;

                prob_cond[i][slot_p][slot_i] += 1.0;
                parent_count[i][slot_p] += 1.0;
            }
        }
    }

    for (int i = 0; i < _n_matches; ++i){
        if (parent[i] == -1){
            double total = elite_size + _n_slots;

            for (int s = 0; s < _n_slots; ++s){
                prob_root[i][s] = (prob_root[i][s] + 1.0) / total;
            }
        }
        else{
            for (int sp = 0; sp < _n_slots; ++sp){
                double total = parent_count[i][sp] + _n_slots;

                for (int s = 0; s < _n_slots; ++s){
                    prob_cond[i][sp][s] = (prob_cond[i][sp][s] + 1.0) / total;
                }
            }
        }
    }
}

std::vector<int> eda_tree_ttp::sample_individual(const std::vector<int>& parent)
{
    std::vector<int> individual(_n_matches, -1);

    std::vector<std::vector<int>> children(_n_matches);
    int root = -1;

    for (int i = 0; i < _n_matches; ++i){
        if (parent[i] == -1){
            root = i;
        }
        else{
            children[parent[i]].push_back(i);
        }
    }

    std::queue<int> q;

    std::discrete_distribution<int> dist_root(prob_root[root].begin(), prob_root[root].end());

    int sampled_slot = dist_root(rng);

    individual[root] = sampled_slot;
    q.push(root);

    while (!q.empty())
    {
        int cur = q.front();
        q.pop();

        for (int child : children[cur]){
            int parent_slot = individual[cur];

            std::discrete_distribution<int> dist_child(prob_cond[child][parent_slot].begin(), prob_cond[child][parent_slot].end());

            int sampled_child_slot = dist_child(rng);

            individual[child] = sampled_child_slot;
            q.push(child);
        }
    }

    // 1-Based
    for (int i = 0; i < _n_matches; ++i){
        individual[i] += 1;
    }

    return individual;
}

std::vector<Individual> eda_tree_ttp::select_survivors()
{

    std::vector<Individual> survivors;
    for (int i = 0; i < _n_survive; ++i) {
        survivors.push_back(elite_population[i]);
    }

    return survivors;
}

double eda_tree_ttp::compute_mean_entropy()
{
    auto marginal = compute_marginals();

    double total_entropy = 0.0;

    for (int i = 0; i < _n_matches; ++i)
    {
        double Hi = 0.0;

        for (int s = 0; s < _n_slots; ++s)
        {
            double p = marginal[i][s];

            if (p > 0.0)
                Hi -= p * std::log2(p);
        }

        total_entropy += Hi;
    }

    return total_entropy / _n_matches;
}