# When Feasible Solutions Become a Trap: Initialization Strategies for EDAs in the Traveling Tournament Problem

This repository contains the code and materials for our paper. The results can be found here: [View Results](https://drive.google.com/file/d/1xsrR3IH-p38Msbbtn-6Msb7kYA94Apjs/view?usp=sharing)

## Abstract

The Traveling Tournament Problem (TTP) is a highly constrained scheduling problem in which obtaining feasible and good-quality schedules remains difficult for optimization methods. In this work, we study how initialization and feasibility influence the behavior of a Univariate EDA and a Tree-based EDA. Experiments were conducted using feasible, partially feasible, and infeasible initial solutions under different penalty intensities, and the behavior of local search was also investigated. Initialization had a stronger influence on the search process than the penalty intensity. Most of the improvements observed in the hybrid approaches originate from the local search component, particularly for the larger benchmark instances. In summary, the results provide evidence that learning effective probabilistic models for highly constrained scheduling problems remains challenging, and help clarify the importance of initialization and local search in this process.

___

## Problem Overview

The Traveling Tournament Problem (TTP) schedules a double round-robin tournament for an even number of teams, where each team plays every other team twice (home and away), while respecting the following constraints:

1. Maximum Streak (maxStreak) – No team plays more than n consecutive home or away games.
2. No Repeats (noRepeat) – Teams cannot face the same opponent in consecutive rounds.
3. Double Round-Robin (doubleRoundRobin) – Each team plays only one game per round.

<div align="center">
<b>Example:</b><br>
<img src="Images/Constraints.png" alt="Traveling Tournament Problem (TTP)" width="400"/><br>
<i>Figure 1:</i> An example of infeasible TTP instance. The rows indicate slots, while the columns indicate the matches of each team. The airplane/house icon indicates an "away" or "home" game, respectively. Here, we consider k = 2 for the maxStreak constraint.
</div>

---

## Features
- C++ (C++20) implementation
- Standard EDA and EDA-Tree
- Integration with GHOST framework
- Multiple initialization strategies:
    * Random
    * Circle method
    * Alternating circle method
- Experimental evaluation on NL benchmark instances

---

## Build & Run

### Prerequisites:
- g++ with C++20 support
- make
- Unix-like environment
- GHOST library

### Running:

    ./run.sh

### Configuration

All experiment parameters can be modified in the `test_all.sh` script.

Example configuration:

```bash
FILE="instances/NL12.txt"
ELITE=0.5
RETAINED=0.75
MAX_EVA=1000000
N_POP=1000
NUM_RUNS=30
SEED=2026
NAME="RANDOM"
```

---

## License

This project is distributed under the [LICENSE](LICENSE). See the LICENSE file for details.
