#pragma once
#include <chrono>
#include <string>

struct ExperimentConfig {
    std::string file = "";
    std::string name = "base";
    int penalization_value = 1000;
    int cores = -1;
    double elite_rate = 0.5;
    double survivor_rate = 0.5;
    int max_generation = 5000;
    int n_population = 1000;
    std::chrono::microseconds timeout_ls = std::chrono::microseconds(1000000);
    bool local_search = true;
    bool penalization = true;
    int num_runs = 30;
    int seed = 2026;
    double ls_probability = 0.2;
};