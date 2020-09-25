#ifndef GOAL_STATE_H_INCLUDED
#define GOAL_STATE_H_INCLUDED

#include <iostream>

using namespace std;

class GoalState {
    struct Pair
    {
       int first;
       int second;
    };
    struct Pair *goals;
    int g_size;
public:
    GoalState();
    void input(istream &in);
    void dump();
    int get_variable(int i);
    int get_value(int i);
    int get_size();
    ~GoalState();
};

#endif // GOAL_STATE_H_INCLUDED
