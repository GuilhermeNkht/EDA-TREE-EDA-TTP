# Compiler flags
MYFLAGS=
CXXFIRSTFLAGS= -O3 -W -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-use-after-free
CXXFIRSTFLAGSDEBUG= -g -O0 -DDEBUG -fsanitize=address -fno-omit-frame-pointer -W -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-use-after-free
CXXFIRSTFLAGSINFO= -g -O2 -W -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -Wno-use-after-free
LDFLAGSDEBUG= -fsanitize=address -fno-omit-frame-pointer -pthread -lghost_staticd
LDFLAGSINFO= -pthread -lghost

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	CXX=g++
	CXXFLAGS= -std=c++20 $(CXXFIRSTFLAGS) $(MYFLAGS)
	CXXFLAGSDEBUG= -std=c++20 $(CXXFIRSTFLAGSDEBUG) $(MYFLAGS)
	CXXFLAGSINFO= -std=c++20 $(CXXFIRSTFLAGSINFO) $(MYFLAGS)
	LDFLAGS=-pthread -lghost_static
endif
ifeq ($(UNAME_S),Darwin)
	CXX=clang++
	CXXFLAGS= -std=c++20  -stdlib=libc++ $(CXXFIRSTFLAGS) $(MYFLAGS)
	CXXFLAGSDEBUG= -std=c++20  -stdlib=libc++ $(CXXFIRSTFLAGSDEBUG) $(MYFLAGS)
	LDFLAGS=-pthread -lghost_static -lc++ -lc++abi
endif

# Directories
SRCDIR=src
HPPDIR=src
OBJDIR=obj
BINDIR=bin

# Files
SOURCES=$(foreach sdir, $(SRCDIR), $(wildcard $(sdir)/*.cpp))
OBJECTS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(SOURCES)))

vpath %.cpp $(SRCDIR)

# Reminder, 'cause it is easy to forget makefile's fucked-up syntax...
# $@ is what triggered the rule, ie the target before :
# $^ is the whole dependencies list, ie everything after :
# $< is the first item in the dependencies list

# Rules

all: $(BINDIR)/ttp_sat $(BINDIR)/ttp_opt $(BINDIR)/solution_checker

debug: CXXFLAGS=$(CXXFLAGSDEBUG)
debug: LDFLAGS=$(LDFLAGSDEBUG)
debug: $(BINDIR)/ttp_sat $(BINDIR)/ttp_opt $(BINDIR)/solution_checker

info: CXXFLAGS=$(CXXFLAGSINFO)
info: LDFLAGS=$(LDFLAGSINFO)
info: $(BINDIR)/ttp_sat $(BINDIR)/ttp_opt $(BINDIR)/solution_checker

$(BINDIR)/ttp_sat: $(OBJDIR)/traveling_tournament_problem.o $(OBJDIR)/builder_ttp.o $(OBJDIR)/constraint_no_repeat.o $(OBJDIR)/constraint_max_streak.o $(OBJDIR)/print_ttp.o $(OBJDIR)/misc.o $(OBJDIR)/ttp_initial_solution.o $(OBJDIR)/eda_ttp.o $(OBJDIR)/eda_tree_ttp.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/ttp_opt: $(OBJDIR)/min_traveling_tournament_problem.o $(OBJDIR)/min_builder_ttp.o $(OBJDIR)/constraint_no_repeat.o $(OBJDIR)/constraint_max_streak.o $(OBJDIR)/objective.o $(OBJDIR)/min_travel_distance.o $(OBJDIR)/min_max_streak_distance.o $(OBJDIR)/print_ttp.o $(OBJDIR)/misc.o $(OBJDIR)/ttp_initial_solution.o $(OBJDIR)/eda_ttp.o $(OBJDIR)/eda_tree_ttp.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BINDIR)/solution_checker: $(OBJDIR)/solution_checker.o $(OBJDIR)/misc.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(HPPDIR) -c $< -o $@

$(OBJDIR)/min_builder_ttp.o: $(OBJDIR)/constraint_no_repeat.o $(OBJDIR)/constraint_max_streak.o $(OBJDIR)/min_travel_distance.o $(OBJDIR)/min_max_streak_distance.o $(OBJDIR)/misc.o builder_ttp.cpp
	$(CXX) $(CXXFLAGS) -I$(HPPDIR) -DTTP_OPT -c $(SRCDIR)/builder_ttp.cpp -o $@

$(OBJDIR)/min_traveling_tournament_problem.o: $(OBJDIR)/min_builder_ttp.o $(OBJDIR)/print_ttp.o $(OBJDIR)/misc.o traveling_tournament_problem.cpp
	$(CXX) $(CXXFLAGS) -I$(HPPDIR) -DTTP_OPT -c $(SRCDIR)/traveling_tournament_problem.cpp -o $@

.PHONY: clean

clean:
	rm -fr core $(BINDIR)/ttp_sat $(BINDIR)/ttp_opt $(BINDIR)/solution_checker $(OBJECTS) 
	rm -f ../nohup.out
	-find results -type f -delete