#pragma once
#include <vector>
#include <string>

class ttp_initial_solution {
public:
    ttp_initial_solution(int n_teams, const std::vector<std::vector<double>>& distance_matrix);

    std::vector<std::vector<int>> generateCircleMethod(int n_schedules);
    std::vector<std::vector<int>> generateAlternatingMethod(int n_schedules);
    std::vector<std::vector<int>> generateRandomSolution(int n_schedules);

private:
    int _n_teams;
    int _n_matches;
    const std::vector<std::vector<double>> _distance_matrix;
    double min_max_streak_distance(const std::vector<int>& solution, int number_teams, int number_weeks, const std::vector<std::vector<double>>& distances);
    int get_match_id(int, int, int) const;
    void save_probability_csv(const std::string& filename, const std::string& method, int n_teams, int n_schedules, const std::vector<std::vector<int>>& counts);
    void save_costs_csv(const std::string& filename, const std::string& method, const std::vector<std::pair<double, std::vector<int>>>& population, int n_best);
};