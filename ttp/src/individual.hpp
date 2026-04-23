#pragma once
#include <vector>

struct Individual {
    std::vector<int> sol;
    double cost;
    int constraint;
    double fitness;
};