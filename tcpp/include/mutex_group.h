#ifndef MUTEX_GROUP_H_INCLUDED
#define MUTEX_GROUP_H_INCLUDED

#include <iostream>

using namespace std;

class MutexGroup
{
    struct Pair
    {
       int first;
       int second;
    };
    struct Pair *facts;
    int mg_size;
public:
    MutexGroup();
    bool input(istream &in);
    void dump();
    int get_variable(int i);
    int get_value(int i);
    int get_size();
    ~MutexGroup();

    friend class PLProblem;
};

#endif // MUTEX_GROUP_H_INCLUDED
