#include "goal_state.h"

#include "helper_function.h"                  /// check_magic

GoalState::GoalState()
{
    goals = NULL;
    g_size = 0;
}

void GoalState::input(istream &in)
{
    check_magic(in, "begin_goal");
    in >> g_size;
    goals=new struct Pair[g_size];
    for (int i = 0; i < g_size; i++)
       in >> goals[i].first >> goals[i].second;
    check_magic(in, "end_goal");
}

void GoalState::dump(){
    cout << "goal state of " << g_size << ":" << endl;
    for (int i = 0; i < g_size; ++i)
        cout << "varID = " << goals[i].first << "  value = " << goals[i].second<<endl;
    cout<<endl;
}

int GoalState::get_variable(int i)
{
    return goals[i].first;
}

int GoalState::get_value(int i)
{
    return goals[i].second;
}

int GoalState::get_size()
{
    return g_size;
}

GoalState :: ~GoalState()
{
    if (goals!= NULL)
       delete[] goals;
}
