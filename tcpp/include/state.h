#ifndef STATE_H
#define STATE_H

#include <iostream>

using namespace std;

class State {
    int *values;
    int size;
public:
    State();
    void input(istream &in,int var_size);
    int operator[](int var);
    void dump();
    ~State();
};

#endif
