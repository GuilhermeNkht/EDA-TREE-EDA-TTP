#include "ttp_initial_solution.hpp"
#include "misc.hpp"
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <fstream>
#include <string>

ttp_initial_solution::ttp_initial_solution(int n_teams, const std::vector<std::vector<double>>& distance_matrix)
    : _n_teams(n_teams), _distance_matrix(distance_matrix)
{
    _n_matches = n_teams * (n_teams - 1);
}

std::vector<std::vector<int>> ttp_initial_solution::generateCircleMethod(int n_schedules)
{
    const int n = _n_teams;
    const int single_slots = n - 1;
    const int slots = 2 * (n - 1);
    const int matches_per_slot = n / 2;
    const int n_matches = n * (n - 1);

    std::mt19937 rng(2026);

    std::vector<std::pair<double, std::vector<int>>> population;

    for (int sample = 0; sample < n_schedules; ++sample)
    {
        std::vector<std::vector<std::pair<int,int>>> schedule(
            slots,
            std::vector<std::pair<int,int>>(matches_per_slot)
        );

        std::vector<int> teams(n);
        for (int i = 0; i < n; ++i)
            teams[i] = i;

        std::shuffle(teams.begin(), teams.end(), rng);

        for (int slot = 0; slot < single_slots; ++slot)
        {
            for (int m = 0; m < matches_per_slot; ++m)
            {
                int home = teams[m];
                int away = teams[n - 1 - m];
                schedule[slot][m] = {home, away};
            }

            int last = teams.back();
            for (int i = n - 1; i > 1; --i)
                teams[i] = teams[i - 1];
            teams[1] = last;
        }

        for (int slot = 0; slot < single_slots; ++slot)
            for (int m = 0; m < matches_per_slot; ++m)
            {
                auto [home, away] = schedule[slot][m];
                schedule[slot + single_slots][m] = {away, home};
            }

        std::vector<int> solution_vector(n_matches, -1);

        for (int slot = 0; slot < slots; ++slot)
            for (int m = 0; m < matches_per_slot; ++m)
            {
                int home = schedule[slot][m].first;
                int away = schedule[slot][m].second;
                int match = get_match_id(home, away, n);
                solution_vector[match] = slot + 1;
            }

        double cost = check_cost_solution(solution_vector, n, _distance_matrix);

        population.push_back({cost, solution_vector});
    }

    std::vector<std::vector<int>> solutions;

    for (const auto& p : population) {
        solutions.push_back(p.second);
    }

    return solutions;
}

std::vector<std::vector<int>> ttp_initial_solution::generateAlternatingMethod(int n_schedules)
{
    const int n = _n_teams;
    const int slots = 2 * (n - 1);
    const int matches_per_slot = n / 2;
    const int n_matches = n * (n - 1);

    std::mt19937 rng(2026);

    std::vector<std::pair<double, std::vector<int>>> population;

    for (int sample = 0; sample < n_schedules; ++sample)
    {
        std::vector<int> teams(n);
        for (int i = 0; i < n; ++i) teams[i] = i;
        std::shuffle(teams.begin(), teams.end(), rng);

        std::vector<std::vector<std::pair<int,int>>> schedule(slots, std::vector<std::pair<int,int>>(matches_per_slot));

        for (int slot = 0; slot < slots; ++slot)
        {
            for (int m = 0; m < matches_per_slot; ++m)
            {
                int t1 = teams[m];
                int t2 = teams[n - 1 - m];
                int home = (slot % 2 == 0) ? t1 : t2;
                int away = (slot % 2 == 0) ? t2 : t1;
                schedule[slot][m] = {home, away};
            }

            int last = teams.back();
            for (int i = n - 1; i > 1; --i) teams[i] = teams[i - 1];
            teams[1] = last;
        }

        std::vector<int> solution_vector(n_matches, -1);
        for (int slot = 0; slot < slots; ++slot)
            for (int m = 0; m < matches_per_slot; ++m)
            {
                auto [home, away] = schedule[slot][m];
                int match = get_match_id(home, away, n);
                solution_vector[match] = slot + 1;
            }

        double cost = check_cost_solution(solution_vector, n, _distance_matrix);
        population.push_back({cost, solution_vector});
    }

    std::vector<std::vector<int>> solutions;
    for (const auto& p : population)
        solutions.push_back(p.second);

    return solutions;
}


std::vector<std::vector<int>> ttp_initial_solution::generateRandomSolution(int n_schedules)
{
    const int n = _n_teams;
    const int slots = 2 * (n - 1);
    const int n_matches = n * (n - 1);

    std::mt19937 rng(2026);
    std::uniform_int_distribution<int> slot_dist(1, slots);

    std::vector<std::vector<int>> solutions;
    solutions.reserve(n_schedules);

    for (int sample = 0; sample < n_schedules; ++sample)
    {
        std::vector<int> solution_vector(n_matches, -1);

        for (int m = 0; m < n_matches; ++m)
        {
            solution_vector[m] = slot_dist(rng);
        }

        solutions.push_back(solution_vector);
    }

    return solutions;
}


void ttp_initial_solution::save_probability_csv(const std::string& filename, const std::string& method, int n_teams, int n_schedules, const std::vector<std::vector<int>>& counts){
    const int n_matches = n_teams * (n_teams - 1);
    const int slots = counts[0].size();

    std::ofstream file(filename);

    file << "method,match_id,home,away,slot,probability\n";

    for (int m = 0; m < n_matches; ++m)
    {
        int h, a;
        convert(m, n_teams, h, a);

        for (int s = 0; s < slots; ++s)
        {
            double p = counts[m][s] / double(n_schedules);

            file << method << ","
                 << m << ","
                 << h - 1 << ","
                 << a - 1 << ","
                 << s << ","
                 << p << "\n";
        }
    }

    file.close();
}

void ttp_initial_solution::save_costs_csv(const std::string& filename, const std::string& method, const std::vector<std::pair<double, std::vector<int>>>& population, int n_best){
    std::ofstream file(filename);
    file << "method,solution_id,cost\n";

    for (int i = 0; i < n_best; ++i)
    {
        file << method << ","
             << i << ","
             << population[i].first << "\n";
    }

    file.close();
}

int ttp_initial_solution::get_match_id(int home, int away, int n) const
{
    if (away < home)
        return home * (n - 1) + away;
    else
        return home * (n - 1) + (away - 1);
}